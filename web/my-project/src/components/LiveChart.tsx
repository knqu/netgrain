import '../styling/Chart.css';
import React, { useEffect, useRef } from "react";
import {
    createChart,
    AreaSeries,
    type IChartApi,
    type UTCTimestamp,
} from "lightweight-charts";

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
              Queue.push(Number(e.data));
            });

            candleSeries.setData(initialData);
            chart.timeScale().fitContent();
            const date = new Date(Date.UTC(2018, 12, 31, 12, 0, 0, 0));
            //var value = 100;
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
