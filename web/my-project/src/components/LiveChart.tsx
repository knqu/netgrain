import '../styling/Chart.css';
import React, { useEffect, useRef, useState, useMemo } from "react";
import {
    createChart,
    AreaSeries,
    type IChartApi,
} from "lightweight-charts";

let initialData = [
  { time: '2018-12-22', value: 32.51 },
  { time: '2018-12-23', value: 31.11 },
  { time: '2018-12-24', value: 27.02 },
  { time: '2018-12-25', value: 27.32 },
  { time: '2018-12-26', value: 25.17 },
  { time: '2018-12-27', value: 28.89 },
  { time: '2018-12-28', value: 25.46 },
  { time: '2018-12-29', value: 23.92 },
  { time: '2018-12-30', value: 22.68 },
  { time: '2018-12-31', value: 22.67 },
];

interface LiveChartProps {
  ws: WebSocket;
}

const LiveChart: React.FC<LiveChartProps> = ({ws}) => {
  const containerRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    if (!containerRef.current) return;

    const chart: IChartApi = createChart(containerRef.current, {
      width: containerRef.current.clientWidth,
      height: containerRef.current.clientHeight,
    })

    const candleSeries = chart.addSeries(AreaSeries);

    chart.subscribeDblClick(param => {
      if (param.seriesData && param.seriesData.size > 0) {
        const clickedPoint = param.seriesData.get(candleSeries); 

        if (clickedPoint) {
          const clickedIndex = initialData.findIndex(d => d.time === clickedPoint.time);

          if (clickedIndex !== -1) {
            const newData = initialData.slice(0, clickedIndex + 1);
            ws.send("update: " + newData[newData.length - 1].value.toString());

            candleSeries.setData(newData);
            initialData = newData;
          }
        }
      }
    });

    var counter = 0;
    var Queue: string[]= [];

    const handleOpen = () => {
      ws.send("sideways");
      console.log("CONNECTED");
    };

    const handleMessage = (e: MessageEvent) => {
      const price = JSON.parse(e.data).price;
      counter++;
      Queue.push(String(price));
    };

    ws.addEventListener("open", handleOpen);
    ws.addEventListener("message", handleMessage);

    candleSeries.setData(initialData);
    chart.timeScale().fitContent();
    const date = new Date(Date.UTC(2018, 12, 31, 12, 0, 0, 0));

    const intervalID = setInterval(() => {
      if (Queue.length === 0) {
        return; 
      }

      const val = Queue.shift();

      date.setUTCDate(date.getUTCDate() + 1);

      var toParse = String(val);
      if (toParse.includes(":")) {
        toParse = toParse.split(":")[1].trim();
      }

      const newDate = date.getFullYear().toString() + "-" + date.getMonth().toString().padStart(2, "0") + "-" + date.getDate().toString().padStart(2, "0");

      candleSeries.update({
        time: newDate, 
        value: Number(toParse)
      });

      initialData.push({
        time: newDate, 
        value: Number(toParse)
      });
    }, 1000); // intervalID

    const resizeObserver = new ResizeObserver(entries => {
      const { width, height } = entries[0].contentRect;
      chart.resize(width, height);
    });

    resizeObserver.observe(containerRef.current);

    return () => {
      clearInterval(intervalID);
      resizeObserver.disconnect();
      ws.removeEventListener("open", handleOpen);
      ws.removeEventListener("message", handleMessage);
      chart.remove();
    };


  }, [ws]);

  return <div className="candleChart" ref={containerRef} style={{ width: '100%', height: '100%' }} />;
};

export default function ChartComponent() {
    const [mode, setMode] = useState("sideways");
    console.log(mode);

    const socket = useMemo(() => new WebSocket("ws://localhost:18080/websocket"), []);

    const changeMode = (newMode: string) => {
      if (socket.readyState === WebSocket.OPEN) {
        socket.send(newMode);
      }
      setMode(newMode);
    };

    return (
        <div className="ChartContainer" style={{ display: 'flex', flexDirection: 'column', height: '100vh', width: '100%' }}>
            
            <div style={{ padding: '15px', display: 'flex', gap: '10px', justifyContent: 'center' }}>
              <button onClick={() => changeMode("bull")}>Bull</button>
              <button onClick={() => changeMode("bear")}>Bear</button>
              <button onClick={() => changeMode("sideways")}>Sideways</button>
            </div>
            
            <div className="Chart_outer_container" style={{ flexGrow: 1, width: '100%' }}>
                <div className="Chart_inner_container" style={{ height: '100%', width: '100%'}}>
                    <div className="Chart" style={{ height: '500px', width: '100%' }}>
                        <LiveChart ws={socket} />
                    </div>
                </div>
            </div>
        </div>
    );
}
