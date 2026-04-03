import '../styling/Chart.css';
import React, { useEffect, useRef, useState } from "react";
import {
    createChart,
    type AreaData,
    AreaSeries,
    type IChartApi,
    type UTCTimestamp,
} from "lightweight-charts";

export default function ChartComponent() {
    const initialData = [
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

    const [mode, setMode] = useState("sideways");

    const handleModeChange = (newMode: string) => {
        setMode(newMode);
        // If the socket is already connected, tell the C++ server to switch immediately
        if (socket.readyState === WebSocket.OPEN) {
            socket.send(newMode);
        }
    };

    const LiveChart: React.FC<LiveChartProps> = ({ws}) => {
        const containerRef = useRef<HTMLDivElement | null>(null);

        useEffect(() => {
            if (!containerRef.current) return;
            const chart: IChartApi = createChart(containerRef.current, {
                width: containerRef.current.clientWidth,
                height: containerRef.current.clientHeight,
            })

            const candleSeries = chart.addSeries(AreaSeries);

            // const wsUri = "ws://localhost:5555/";
            // const websocket = new WebSocket(wsUri);
            var counter = 0;
            ws.addEventListener("open", () => {
              ws.send("sideways"); // HERE
              console.log("CONNECTED");
              console.log(`SENT: ping: ${counter}`);
            });
            var Queue: number[]= [];
            ws.addEventListener("message", (e) => {
              //console.log(`RECEIVED: ${e.data}: ${counter}`);
              counter++;
              Queue.push(e.data);
            });

            candleSeries.setData(initialData);
            chart.timeScale().fitContent();
            const date = new Date(Date.UTC(2018, 12, 31, 12, 0, 0, 0));
            var value = 100;
            function* getNextRealTimeUpdate() {
              while (true) {
                date.setUTCDate(date.getUTCDate() + 1);
                
                var val : number | undefined= -1; 
                do {
                  val = Queue.shift();
                  
                } while (val === undefined);
                const time = date.getTime() / 1000 as UTCTimestamp;
                yield {time: time, value: val};
              }
              return null;
            };

            const streamingDataProvider = getNextRealTimeUpdate();
            const intervalID = setInterval(() => {
            const update = streamingDataProvider.next();
            if (update.done) {
              clearInterval(intervalID);
                return;
              }
              var toParse = update.value.value;
              if (toParse.includes(":")) {
                toParse = toParse.split(":")[1].trim();
              }
              candleSeries.update({
                time: update.value.time, 
                value: Number(toParse)
              });
            }, 1000);

            const resizeObserver = new ResizeObserver(entries => {
                const { width, height } = entries[0].contentRect;
                chart.resize(width, height);
            });

            resizeObserver.observe(containerRef.current);

            return () => {
                resizeObserver.disconnect();
                chart.remove();
            };
        }, []);

        const changeMode = (newMode: string) => {
          if (ws.readyState === WebSocket.OPEN) {
            ws.send(newMode);
          }
        };

        return (
          <div style={{ display: 'flex', flexDirection: 'column', height: '100%' }}>
            <div style={{ paddingBottom: '10px', display: 'flex', gap: '10px', justifyContent: 'center' }}>
            <button onClick={() => changeMode("bull")}>Bull</button>
            <button onClick={() => changeMode("bear")}>Bear</button>
            <button onClick={() => changeMode("sideways")}>Sideways</button>
            </div>
            <div className="candleChart" ref={containerRef} style={{ flexGrow: 1 }} />
          </div>

        );
    };

    // Chart body
    function Chart() {
      const socket = new WebSocket("ws://localhost:5555");

      return (
          <div className="Chart_outer_container">
              <div className="Chart_inner_container">
                  <div className="Chart" >
                      <LiveChart ws={socket} />
                  </div>
              </div>
          </div>
      );
    }

    return (
        <div className="ChartContainer">
            <Chart />
        </div>
    );
}
