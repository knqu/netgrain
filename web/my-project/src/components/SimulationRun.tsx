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

var data1: pointData[] = [];
var data2: pointData[] = [];
var lowerBound: number;
var upperBound: number;

var data1High = 0.0;
var data1Low = 1000.0;

var data2High = 0.0;
var data2Low = 1000.0;

// --- Main Dashboard ---
export default function SimulationRun() {
  const [activeStock, setActiveStock] = useState('1');
  const socketRef1 = useRef<WebSocket>(null);
  if (!socketRef1.current) {
    socketRef1.current = new WebSocket("ws://localhost:5555/");
  }
  const socketRef2 = useRef<WebSocket>(null);
  if (!socketRef2.current) {
    socketRef2.current = new WebSocket("ws://localhost:5556/");
  }


  var areaSeries: any;
  const [currentPage, setPage] = useState<string>("Run");
  const [Stats, setWidget] = useState<any>();

  const SimRun: React.FC = () => {
    const containerRef = useRef<HTMLDivElement | null>(null);

    
    useEffect(() => {
      if (!containerRef.current) return;
      const chart: IChartApi = createChart(containerRef.current, {
        width: containerRef.current.clientWidth,
        height: containerRef.current.clientHeight,
      });

      const areaSeries = chart.addSeries(AreaSeries);


      var counter = 0;
      var counter2 = 0;
      const date1 = new Date(Date.UTC(2018, 12, 31, 12, 0, 0, 0));
      const date2 = new Date(Date.UTC(2018, 12, 31, 12, 0, 0, 0));
      socketRef1.current!.addEventListener("open", () => {
        console.log("CONNECTED ON 1");
        console.log(`SENT: ping: ${counter}`);
      });

      socketRef1.current!.addEventListener("message", (e) => {
        console.log(`RECEIVED: ${e.data}: ${counter}`);
        date1.setUTCDate(date1.getUTCDate() + 1);
        const time: UTCTimestamp = date1.getTime() / 1000 as UTCTimestamp;
        counter++;
        data1.push({ time: time, value: Number(e.data) });

        if (activeStock === '1') {
          areaSeries.update({ time: time, value: Number(e.data) });
        }

        if (data1.at(-1)!.value > data1High) {
          data1High = data1.at(-1)!.value;
        }
        if (data1.at(-1)!.value < data1Low) {
          data1Low = data1.at(-1)!.value;
        }

        if (data1.at(-1)!.value < lowerBound || data1.at(-1)!.value > upperBound) {
          lowerBound = Number.MIN_VALUE;
          upperBound = Number.MAX_VALUE;
          socketRef1.current!.send("pause");
          socketRef2.current!.send("pause");
        }
      });



      socketRef2.current!.addEventListener("open", () => {
        console.log("CONNECTED ON 2");
        console.log(`SENT: ping: ${counter2}`);
      });

      socketRef2.current!.addEventListener("message", (e) => {
        console.log(`RECEIVED: ${e.data}: ${counter2}`);
        date2.setUTCDate(date2.getUTCDate() + 1);
        const time: UTCTimestamp = date2.getTime() / 1000 as UTCTimestamp;
        counter2++;
        data2.push({ time: time, value: Number(e.data) });
        if (activeStock === '2') {
          areaSeries.update({ time: time, value: Number(e.data) });
        }

        if (data2.at(-1)!.value > data2High) {
          data2High = data2.at(-1)!.value;
        }
        if (data2.at(-1)!.value < data2Low) {
          data2Low = data2.at(-1)!.value;
        }

        if (data2.at(-1)!.value < lowerBound || data2.at(-1)!.value > upperBound) {
          lowerBound = Number.MIN_VALUE;
          upperBound = Number.MAX_VALUE;
          socketRef1.current!.send("pause");
          socketRef2.current!.send("pause");
        }
      });


      chart.timeScale().fitContent();

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



    return <div className="areaChart" ref={containerRef} />
  };


  // Chart body
  function Chart() {
    return (
      <div className="Chart_outer_container">
        <div className="Chart_inner_container">
          <div className="Chart" >
            <SimRun />
          </div>
        </div>
      </div>
    );
  }


  function updateChart(str: String) {
    switch (str) {
      case '1':
        setActiveStock('1');
        areaSeries.setData(data1);
        break;
      case '2':
        setActiveStock('2');
        areaSeries.setData(data2);
        break;
    }
  };

  function pause() {
    console.log("send pause signal");
    socketRef1.current!.send("pause");
    socketRef2.current!.send("pause");
  }

  function resume() {
    console.log("send resume signal");
    socketRef1.current!.send("resume");
    socketRef2.current!.send("resume");
  }

  function modify() {
    console.log("modify demo")
    socketRef1.current!.send("mod demo");
    socketRef2.current!.send("mod demo");
  }

  async function sleep(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
  }


  async function sleepAfterTime() {
    console.log("Sleep after elapsed time demo");
    const time = (document.getElementById("sleep timer") as HTMLInputElement).value;
    console.log(`sleep for ${time} seconds`);
    await sleep(Number(time) * 1000);
    socketRef1.current!.send("pause");
    socketRef2.current!.send("pause");
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
    socketRef1.current!.send("stop");
    socketRef2.current!.send("stop");
    var widgetVal: WidgetInterface[] = [];
    widgetVal.length = 0;
    switch (selectedOption) {
      case ('1'):
        widgetVal.push({ lowVal: data1Low, highVal: data1High })
        widgetVal.push({ lowVal: data2Low, highVal: data2High })
        break;
      case ('2'):
        widgetVal.push({ lowVal: data1Low, highVal: data1High })
        widgetVal.push({ lowVal: null, highVal: null })
        break;
      case ('3'):
        widgetVal.push({ lowVal: null, highVal: null })
        widgetVal.push({ lowVal: data2Low, highVal: data2High })
        break;
      case ('4'):
        widgetVal.push({ lowVal: null, highVal: data1High })
        widgetVal.push({ lowVal: null, highVal: data2High })
        break;
      case ('5'):
        widgetVal.push({ lowVal: data1Low, highVal: null })
        widgetVal.push({ lowVal: data2Low, highVal: null });
        break;
    }
    console.log(widgetVal);
    setWidget(widgetVal);
    setPage("End");
    // determine what data is saved from the simulation for the widget
  };

  function RenderPage() {
      console.log(Stats);
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
                <button onClick={() => updateChart('1')}>Stock 1</button>
                <button onClick={() => updateChart('2')}>Stock 2</button>
              </div>
              <Chart />

            </div>
          );
        case "End":
          return (< EndSimulation items={Stats}/>);
      }
    }

  //const websocket2 = useMemo(() => new WebSocket("ws://localhost:5555/"), []);
  return (
    <>
      <RenderPage />
    </>
  );
}
