import "../styling/SimulationRun.css"
import React, { useEffect, useRef, useState } from 'react';
import {
  createChart,
  AreaSeries,
  type IChartApi,
  type UTCTimestamp,
} from "lightweight-charts"

import { type WidgetInterface } from "./EndSimulation"
import EndSimulation from "./EndSimulation"

//import type LiveChartProps from './LiveChart';

export interface Snapshot {
  id: number;
  timeLabel: string;
  price: number;
  clamped: boolean;
  type: string;
  drift: number;
  volatility: number;
}


interface pointData {
  time: UTCTimestamp;
  value: number;
};
/*
type seriesType = 
  ISeriesApi<
    "Area",
    Time,
    AreaData<Time> | WhitespaceData<Time>,
    AreaSeriesOptions,
    DeepPartial<AreaStyleOptions & SeriesOptionsCommon>
  >
*/
interface OrderData {
  id: number;
  qty: number;
  t_price: number;
  type: String;
}

// TODO: update orderdata stuff
let paused = false;
var data: pointData[][] = [];
var order_data: OrderData[][] = [];
var stock_order_index: number = 0;
var lowerBound: number;
var upperBound: number;

var highs: number[] = [0, 0];
var lows: number[] = [1000, 1000];



// -- Sim Run component

const SimRun: React.FC<{ tickers: String[], socketRef: WebSocket, activeStock: String, dates: Date[], onNewSnapshot: (snap: Snapshot) => void }> = ({ tickers, socketRef, activeStock, dates, onNewSnapshot }) => {
  const containerRef = useRef<HTMLDivElement | null>(null);
  const chartRef = useRef<IChartApi | null>(null);
  const seriesRef = useRef<any>(null);
  const [totalTicks, setTotalTicks] = useState(0);
  const [clampedTicks, setClampedTicks] = useState(0);
  const [hoverData, setHoverData] = useState<any>(null);

  // Use ref to avoid stale closures in chart subscriptions
  const activeStockRef = useRef(activeStock);
  useEffect(() => { activeStockRef.current = activeStock; }, [activeStock]);

  // Global mapping to store details for US 13 hover
  const detailsMap = useRef<Record<number, Record<number, any>>>({});


  // TODO: modify so this works with one socket
  function addMsg(curSocket: WebSocket) {
    const onMsgTemp = (e: MessageEvent) => {
      try {
        console.log(e.data);
        const payload = JSON.parse(e.data);
        const msg_t = String(payload.msg_type);
        switch (msg_t) {
          case "stock":
            const price = Number(payload.price);
            const idx = Number(payload.id);
            setTotalTicks(prev => prev + 1);
            if (payload.clamped) {
              setClampedTicks(prev => prev + 1);
            }

            //dates[idx].setUTCDate(dates[idx].getUTCDate() + 1);
            var time: UTCTimestamp;
            if (data[idx].length === 0) {
              const nd = new Date(2018, 12, 31, 12, 0, 0)
              time = nd.getTime() / 1000 as UTCTimestamp;
            }
            else {
              time = (data[idx][data[idx].length - 1].time + 1) as UTCTimestamp;
            }
            //const time: UTCTimestamp = (data[idx][-1].time + 1000) / 1000 as UTCTimestamp;
            data[idx].push({ time: time, value: price });

            const d = new Date(time * 1000);
            const timeLabel = `${d.getUTCFullYear()}-${String(d.getUTCMonth() + 1).padStart(2, '0')}-${String(d.getUTCDate()).padStart(2, '0')}`;

            const marketMode = String(payload.type || 'normal');

            onNewSnapshot({
              id: idx,
              timeLabel: timeLabel,
              price: price,
              clamped: payload.clamped,
              type: marketMode,
              drift: payload.drift,
              volatility: payload.volatility
            });

            if (!detailsMap.current[idx]) detailsMap.current[idx] = {};
            detailsMap.current[idx][time as number] = {
              price: price,
              type: marketMode,
              drift: payload.drift,
              volatility: payload.volatility
            };

            if (Number(activeStockRef.current) === idx) {
              seriesRef.current.update({ time: time, value: price });
            }

            if (data[idx].at(-1)!.value > highs[idx]) {
              highs[idx] = data[0].at(-1)!.value;
            }
            if (data[idx].at(-1)!.value < lows[idx]) {
              lows[idx] = data[idx].at(-1)!.value;
            }

            if (data[idx].at(-1)!.value < lowerBound || data[idx].at(-1)!.value > upperBound) {
              lowerBound = Number.MIN_VALUE;
              upperBound = Number.MAX_VALUE;
              socketRef.send("pause");
            }
            break;
          case "fill":
            // find which ticker we have filled
            //Ticker Quantity, Fill_Price, Side
            const tick = String(payload.Ticker);
            var tick_idx = -1;
            for (var i = 0; i < tickers.length; i++) {
              if (tickers[i] === tick) {
                tick_idx = i;
                break;
              }
            }
            if (tick_idx == -1 || tick_idx >= order_data.length) {
              break;
            }
            const quantity = Number(payload.Quantity);
            const f_price = Number(payload.Fill_Price);
            const f_side = String(payload.Side);
            stock_order_index = stock_order_index + 1
            order_data[tick_idx].push({id: stock_order_index, qty: quantity, t_price: f_price, type: f_side})
          //TODO: order update code
        }

      } catch (err) {
        console.error("Error parsing WS message:", err);
      }
    }
    curSocket.addEventListener("message", onMsgTemp);
    return onMsgTemp;
  }

  useEffect(() => {
    if (!containerRef.current) return;
    const chart: IChartApi = createChart(containerRef.current, {
      width: containerRef.current.clientWidth,
      height: containerRef.current.clientHeight,
    });
    chartRef.current = chart;

    const areaSeries = chart.addSeries(AreaSeries);
    seriesRef.current = areaSeries;

    var msg: ((e: MessageEvent) => void) = addMsg(socketRef);


    chart.timeScale().fitContent();

    const resizeObserver = new ResizeObserver(entries => {
      const { width, height } = entries[0].contentRect;
      chart.resize(width, height);
    });

    chart.subscribeCrosshairMove((param) => {
      if (param.time) {
        const timeKey = param.time as number;
        const currentActive = Number(activeStockRef.current);
        if (detailsMap.current[currentActive] && detailsMap.current[currentActive][timeKey]) {
          setHoverData(detailsMap.current[currentActive][timeKey]);
        }
      } else {
        setHoverData(null);
      }
    });

    resizeObserver.observe(containerRef.current);
    seriesRef.current.setData(data[Number(activeStock)]);
    return () => {
      socketRef.removeEventListener("message", msg)
      resizeObserver.disconnect();
      chart.remove();

    };
  }, []);

  useEffect(() => {
    // Check if both the chart and series are ready
    if (seriesRef.current && chartRef.current) {
      try {
        const idx = Number(activeStock);
        const newData = data[idx] || [];

        seriesRef.current.setData(newData);

        chartRef.current.timeScale().fitContent();

      } catch (err) {
        console.error("Error swapping stock data:", err);
      }
    }
  }, [activeStock]);

  const percentReal = totalTicks === 0
    ? 100
    : (((totalTicks - clampedTicks) / totalTicks) * 100).toFixed(2);

  const color = Number(percentReal) >= 95 ? "green" : "red";

  return (
    <div style={{ display: "flex", flexDirection: "column", height: "100%", width: "100%" }}>
      <div style={{ 
        padding: '6px 12px', 
        backgroundColor: '#1e1e1e', 
        borderBottom: '1px solid #333', 
        display: 'flex', 
        justifyContent: 'space-between', 
        alignItems: 'center',
        fontSize: '0.85rem'
      }}>
        <div><strong>Generator Realism:</strong> <span style={{ color: color, marginLeft: '8px' }}>{percentReal}% Adherence</span></div>

        {hoverData && (
          <div style={{ backgroundColor: '#2d2d2d', padding: '2px 8px', borderRadius: '4px', border: '1px solid #444' }}>
            <strong>Hover:</strong> {hoverData.type} | <strong>Drift:</strong> {hoverData.drift?.toFixed(2)}% | <strong>Vol:</strong> {hoverData.volatility?.toFixed(2)}%
          </div>
        )}

        <div style={{ color: '#94a3b8' }}>{clampedTicks} outliers clamped</div>
      </div>
      {/* Removed minHeight to allow flex-grow to control size */}
      <div className="areaChart" style={{ flexGrow: 1 }} ref={containerRef} />
    </div>
  );
};


interface simulationRunProps {
  num_stocks: number;
  tickers: String[];
}

// --- Main Dashboard ---
export default function SimulationRun({ num_stocks, tickers }: simulationRunProps) {
  for (var i = 0; i < num_stocks; i++) {
    data.push([]);
    order_data.push([]);
  }
  //const [allOrders, setAllOrders] = useState<OrderData[][]>(order_data);
  const [latestPrices, setLatestPrices] = useState<number[]>(new Array(num_stocks).fill(0));
  const [activeStock, setActiveStock] = useState('0');

  // US 12 & 13 State
  const [speed, setSpeed] = useState(10);
  const [snapshotLog, setSnapshotLog] = useState<Snapshot[]>([]);

  const handleNewSnapshot = (snap: Snapshot) => {
    setLatestPrices(prev => {
      const next = [...prev];
      next[snap.id] = snap.price;
      return next;
    });

    setSnapshotLog(prev => {
      const newLog = [snap, ...prev];
      if (newLog.length > 100) newLog.pop();
      return newLog;
    });
  };

  const socketRef = useRef<WebSocket>(null);

  const handleFrequencyChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const val = parseInt(e.target.value);
    setSpeed(val);
    let interval = Math.max(10, Math.round(2000 / val)); // Scale to max out at 10ms
    if (socketRef.current?.readyState === WebSocket.OPEN) {
      socketRef.current.send(`freq:${interval}`);
    }
  };

  if (!socketRef.current) {
    socketRef.current = new WebSocket("ws://localhost:18080/websocket");
  }
  const dates = useRef<Date[] | null>(null);
  if (!dates.current) {
    dates.current = [];
    for (var i = 0; i < num_stocks; i++) {
      dates.current.push(new Date(2018, 12, 31, 12, 0, 0));
    }
  }


  useEffect(() => {
    const handleOpen = () => console.log("Connected");
    socketRef.current!.addEventListener("open", handleOpen);

    return () => {
      socketRef.current!.removeEventListener("open", handleOpen);
    }
  })


  //var areaSeries: any;
  const [currentPage, setPage] = useState<string>("Run");
  const [Stats, setWidget] = useState<any>();


  function updateChart(str: String) {
    setActiveStock(String(str));
  };

  function pause() {
    console.log("send pause signal");
    socketRef.current!.send("pause");
  }

  function resume() {
    console.log("send resume signal");
    socketRef.current!.send("resume");
  }

  function modify() {
    console.log("modify demo")
    socketRef.current!.send("flash_crash");
  }

  function bear() {
    console.log("MARKET EVENT: bear")
    socketRef.current!.send("bear");
  }

  function bull() {
    console.log("MARKET EVENT: bull")
    socketRef.current!.send("bull");
  }

  function sideways() {
    console.log("MARKET EVENT: sideways")
    socketRef.current!.send("sideways");
  }

  async function sleep(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
  }

  async function sleepAfterTime() {
    console.log("Sleep after elapsed time demo");
    const time = (document.getElementById("sleep timer") as HTMLInputElement).value;
    console.log(`sleep for ${time} seconds`);
    await sleep(Number(time) * 1000);
    socketRef.current!.send("pause");
  }

  async function sleepOnCondition() {
    console.log("Sleep after condition demo");

    lowerBound = Number((document.getElementById("lower bound") as HTMLInputElement).value);
    upperBound = Number((document.getElementById("upper bound") as HTMLInputElement).value);

  }


  const [selectedOption, setSelectedOption] = useState("1");

  function WidgetForm() {

    const handleChange = (event: any) => {
      setSelectedOption(event.target.value);
    };


    return (
      <div className="widgetData">
        <form id="widgetData">
          <label> <input type="radio" name="widgetVal" value="1" checked={selectedOption === "1"} onChange={handleChange} /> Default </label>
          <label> <input type="radio" name="widgetVal" value="2" checked={selectedOption === "2"} onChange={handleChange} /> Stock 1 metrics </label>
          <label> <input type="radio" name="widgetVal" value="3" checked={selectedOption === "3"} onChange={handleChange} /> Stock 2 metrics </label>
          <label> <input type="radio" name="widgetVal" value="4" checked={selectedOption === "4"} onChange={handleChange} /> High Vals </label>
          <label> <input type="radio" name="widgetVal" value="5" checked={selectedOption === "5"} onChange={handleChange} /> Low Vals </label>
        </form>
      </div>
    )
  }



  function endSim() {
    data = [];
    order_data = [];
    socketRef.current!.send("stop");
    var widgetVal: WidgetInterface[] = [];
    widgetVal.length = 0;
    switch (selectedOption) {
      case ('1'):
        widgetVal.push({ lowVal: lows[0], highVal: highs[0] })
        widgetVal.push({ lowVal: lows[1], highVal: highs[1] })
        break;
      case ('2'):
        widgetVal.push({ lowVal: lows[0], highVal: highs[0] })
        widgetVal.push({ lowVal: null, highVal: null })
        break;
      case ('3'):
        widgetVal.push({ lowVal: null, highVal: null })
        widgetVal.push({ lowVal: lows[1], highVal: highs[1] })
        break;
      case ('4'):
        widgetVal.push({ lowVal: null, highVal: highs[0] })
        widgetVal.push({ lowVal: null, highVal: highs[1] })
        break;
      case ('5'):
        widgetVal.push({ lowVal: lows[0], highVal: null })
        widgetVal.push({ lowVal: lows[1], highVal: null });
        break;
    }
    //console.log(widgetVal);
    setWidget(widgetVal);
    setPage("End");
    // determine what data is saved from the simulation for the widget
  };


  const renderPage = () => {
    if (currentPage === "End") {
      return <EndSimulation items={Stats} />;
    }

    return (
      <div className="simulation-container">
        {/* Top Control Panel[cite: 2] */}
        <div className="control-grid">
          <div className="control-group">
            <h3>Simulation Controls</h3>
            <div className="button-row">
              <button onClick={pause}>Pause</button>
              <button onClick={resume}>Resume</button>
              <button onClick={() => endSim()} style={{background: '#ef4444'}}>End Simulation</button>
            </div>
            <div className="speed-slider" style={{marginTop: '10px'}}>
              <label style={{fontSize: '0.8rem'}}>Speed: <strong>{speed}x</strong></label>
              <input type="range" min="1" max="20" value={speed} onChange={handleFrequencyChange} style={{width: '100%'}}/>
            </div>
          </div>

          <div className="control-group">
            <h3>Automated Pausing</h3>
            <div className="button-row" style={{marginBottom: '8px'}}>
              <input id="sleep timer" type="number" defaultValue="5" style={{width: '50px'}}/>
              <button onClick={sleepAfterTime}>Wait Pause</button>
            </div>
            <div className="button-row">
              <input id="lower bound" placeholder="Min" style={{width: '50px'}}/>
              <input id="upper bound" placeholder="Max" style={{width: '50px'}}/>
              <button onClick={sleepOnCondition}>Set Bounds</button>
            </div>
          </div>

          <div className="control-group">
            <h3>Market Events</h3>
            <div className="button-row">
              <button onClick={bull}>Bull</button>
              <button onClick={bear}>Bear</button>
              <button onClick={sideways}>Sideways</button>
              <button onClick={modify}>Flash Crash</button>
            </div>
          </div>
        </div>

        <div className="button-row">
          {Array.from({ length: num_stocks }, (_, i) => (
            <button 
              key={i} 
              className={activeStock === i.toString() ? 'active' : ''}
              onClick={() => updateChart(i.toString())}
            >
              ${tickers[i]}: ${latestPrices[i]?.toFixed(2)}
            </button>
          ))}
        </div>

        <div className="main-content-row">
          {/* Component 1: Chart */}
          <div className="Chart_outer_container">
            <SimRun tickers={tickers} socketRef={socketRef.current!} activeStock={activeStock} dates={dates.current!} onNewSnapshot={handleNewSnapshot} />
          </div>

          {/* Component 2: Snapshot Metrics */}
          <div className="table-card">
            <h4>Snapshot Metrics (Stock {Number(activeStock) + 1})</h4>
            <div className="table-container">
              <table>
                <thead>
                  <tr><th>Price</th><th>Mode</th><th>Drift</th><th>Vol</th></tr>
                </thead>
                <tbody>
                  {snapshotLog.filter(snap => snap.id === Number(activeStock)).map((snap, idx) => (
                    <tr key={idx}>
                      <td>${snap.price.toFixed(2)}</td>
                      <td>{snap.type}</td>
                      <td style={{ color: snap.drift >= 0 ? '#10b981' : '#ef4444' }}>{snap.drift?.toFixed(2)}%</td>
                      <td>{snap.volatility?.toFixed(2)}%</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </div>

          {/* Component 3: Active Orders */}
          <div className="table-card">
            <h4>Active Orders</h4>
            <div className="table-container">
              <table>
                <thead>
                  <tr><th>ID</th><th>Type</th><th>Qty</th><th>Price</th></tr>
                </thead>
                <tbody>
                  {order_data[Number(activeStock)]?.map((order) => (
                    <tr key={order.id}>
                      <td>#{order.id}</td>
                      <td style={{ color: order.type === 'BUY' ? '#10b981' : '#ef4444', fontWeight: 'bold' }}>{order.type}</td>
                      <td>{order.qty}</td>
                      <td>${order.t_price.toFixed(2)}</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </div>
        </div>
      </div>
    );
  };

  //const websocket2 = useMemo(() => new WebSocket("ws://localhost:5555/"), []);
  return (
    <>
      {renderPage()}
    </>
  );
}
