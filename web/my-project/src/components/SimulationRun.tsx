import "../styling/SimulationRun.css"
import React, { useEffect, useRef, useState, useMemo } from 'react';
import {
  createChart,
  type AreaData,
  AreaSeries,
  type IChartApi,
  type UTCTimestamp,
  type ISeriesApi,
  type WhitespaceData,
  type LineSeriesOptions,
  type DeepPartial,
  type AreaStyleOptions,
  type SeriesOptionsCommon,
} from "lightweight-charts"
import type { C } from "node_modules/react-grid-layout/dist/position-CetgOHhD.d.mts";
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
    switch(str) {
      case '1':
        setActiveStock('1');
        areaSeries.setData(data1);
      case '2':
         setActiveStock('2');
         areaSeries.setData(data2);
    }
  };

  //const websocket2 = useMemo(() => new WebSocket("ws://localhost:5555/"), []);
  return (
    <div className="chart-wrapper">
       
      <div style={{ marginBottom: '10px' }}>
        <button onClick={() => updateChart('1')}>Stock 1</button>
        <button onClick={() => updateChart('2')}>Stock 2</button>
      </div>
      <Chart />


    </div>
  );
}
