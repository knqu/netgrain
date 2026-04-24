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
var data: pointData[][] = [];
var lowerBound: number;
var upperBound: number;

var highs: number[] = [0, 0];
var lows: number[] = [1000, 1000];

// -- Sim Run component

const SimRun: React.FC<{ socketRef: WebSocket, activeStock: String, dates: Date[], onPriceUpdate: (idx: number, price: number) => void }> = ({ socketRef, activeStock, dates, onPriceUpdate }) => {
  const containerRef = useRef<HTMLDivElement | null>(null);
  const chartRef = useRef<IChartApi | null>(null);
  const seriesRef = useRef<any>(null);
  const [totalTicks, setTotalTicks] = useState(0);
  const [clampedTicks, setClampedTicks] = useState(0);

  // TODO: modify so this works with one socket
  function addMsg(curSocket: WebSocket) {
    const onMsgTemp = (e: MessageEvent) => {
      try {
          console.log(e.data);
        
        const payload = JSON.parse(e.data);
        const price = Number(payload.price);
        const idx = Number(payload.id);
        onPriceUpdate(idx, price);
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

        if (Number(activeStock) === idx) {
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
          socketRef.send("multiple: pause");
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

    var msg: ((e: MessageEvent)=> void) = addMsg(socketRef);
    
  
    chart.timeScale().fitContent();

    const resizeObserver = new ResizeObserver(entries => {
      const { width, height } = entries[0].contentRect;
      chart.resize(width, height);
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
    if (seriesRef.current) {
      try {
        seriesRef.current.setData(data[Number(activeStock)]);
      } catch (err) {
        console.error("Chart is probably disposed", err);
      }
    }
  }, [activeStock])

  const percentReal = totalTicks === 0
    ? 100
    : (((totalTicks - clampedTicks) / totalTicks) * 100).toFixed(2);

  const color = Number(percentReal) >= 95 ? "green" : "red";

  return (
    <div style={{ display: "flex", flexDirection: "column", height: "100%", width: "100%" }}>
      <div style={{ padding: '10px', backgroundColor: '#f9f9f9', borderBottom: '2px solid #ccc', display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
        <div style={{ fontSize: '1.2rem' }}><strong>Generator Realism Phase:</strong> <span style={{ color: color, marginLeft: '8px' }}>{percentReal}% Adherence</span></div>
        <div style={{ fontSize: '0.9rem', color: '#666' }}>{clampedTicks} wild outliers clamped</div>
      </div>
      <div className="areaChart" style={{ flexGrow: 1, minHeight: '400px' }} ref={containerRef} />
    </div>
  );
};

interface simulationRunProps {
  num_stocks: number;
}

// --- Main Dashboard ---
export default function SimulationRun({num_stocks}: simulationRunProps) {
  for (var i = 0; i < num_stocks; i++) {
    data.push([]);
  }
  const [latestPrices, setLatestPrices] = useState<number[]>(new Array(num_stocks).fill(0));
  const [activeStock, setActiveStock] = useState('0');

  const handlePriceUpdate = (idx: number, price: number) => {
    setLatestPrices(prev => {
      const next = [...prev];
      next[idx] = price;
      return next;
    });
  };

  const socketRef = useRef<WebSocket>(null);
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
    socketRef.current!.send("multiple: pause");
  }

  function resume() {
    console.log("send resume signal");
    socketRef.current!.send("resume");
  }

  function modify() {
    console.log("multiple: modify demo")
    socketRef.current!.send("multiple: flash_crash");
  }

  async function sleep(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
  }


  async function sleepAfterTime() {
    console.log("Sleep after elapsed time demo");
    const time = (document.getElementById("sleep timer") as HTMLInputElement).value;
    console.log(`sleep for ${time} seconds`);
    await sleep(Number(time) * 1000);
    socketRef.current!.send("multiple: pause");
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
    data = []
    socketRef.current!.send("multiple: stop");
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
    //console.log(Stats);
    switch (currentPage) {
      case "Run":
        return (
          <div className="chart-wrapper">
            <button onClick={() => pause()}>Pause</button>
            <button onClick={() => resume()}>Resume</button>
            <button onClick={() => modify()}>Modify Demo</button>
            <button onClick={() => sleepAfterTime()}>wait pause demo</button>
            <button onClick={() => sleepOnCondition()}>Conditional pause demo</button>
            <input id="sleep timer" inputMode="decimal"></input>
            <div>
              <input id="lower bound" inputMode="decimal"></input>
              <input id="upper bound" inputMode="decimal"></input>
            </div>
            <div>
              <button onClick={() => endSim()}>End simulation</button>
            </div>
            <div>
              <WidgetForm />
            </div>
            <div style={{ marginBottom: '10px' }}>
              {Array.from({ length: num_stocks }, (_, i) => (
                <button 
                key={i} 
                onClick={() => updateChart(i.toString())}
                style={{ 
                marginRight: '5px',
                fontWeight: activeStock === i.toString() ? 'bold' : 'normal' 
                }}
                >
                  Stock {i + 1}
                </button>
              ))}
            </div>
            <div className="Chart_outer_container">
              <div className="Chart_inner_container">
                <div className="Chart" >
                  <SimRun socketRef={socketRef.current!} activeStock={activeStock} dates={dates.current!} onPriceUpdate={handlePriceUpdate} />
                </div>
              </div>
            </div>
            <div className="price-dashboard">
              {latestPrices.map((price, i) => (
                <div key={i}>
                <strong>Stock {i + 1}:</strong> ${price.toFixed(2)}
              </div>
              ))}
            </div>
          </div>
        );
      case "End":
        return (< EndSimulation items={Stats} />);
    }
  }

  //const websocket2 = useMemo(() => new WebSocket("ws://localhost:5555/"), []);
  return (
    <>
      {renderPage()}
    </>
  );
}
