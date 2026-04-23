import '../styling/Chart.css';
import React, { useEffect, useRef, useState, useMemo } from "react";
import {
    createChart,
    AreaSeries,
    type IChartApi,
} from "lightweight-charts";

import {
  type DeepPartial,
} from 'lightweight-charts';

import type {
  AreaData,
  AreaSeriesOptions,
  AreaStyleOptions,
  CandlestickData,
  CandlestickSeriesOptions,
  CandlestickStyleOptions,
  ISeriesApi,
  LineData,
  LineSeriesOptions,
  LineStyleOptions,
  SeriesOptionsCommon,
  Time,
  WhitespaceData,
} from 'lightweight-charts';

type chartType =
  ISeriesApi<
    "Candlestick",
    Time,
    WhitespaceData<Time> | CandlestickData<Time>,
    CandlestickSeriesOptions,
    DeepPartial<CandlestickStyleOptions & SeriesOptionsCommon>
  > |
  ISeriesApi<
    "Line",
    Time,
    LineData<Time> | WhitespaceData<Time>,
    LineSeriesOptions,
    DeepPartial<LineStyleOptions & SeriesOptionsCommon>
  > |
  ISeriesApi<
    "Area",
    Time,
    AreaData<Time> | WhitespaceData<Time>,
    AreaSeriesOptions,
    DeepPartial<AreaStyleOptions & SeriesOptionsCommon>
  >;

let chartSeries: chartType;

let paused = false;

interface DataEntry {
  time: string;
  value: number;
};

let data: DataEntry[] = [
];

interface LiveChartProps {
  ws: WebSocket;
}

let Queue: string[];

const LiveChart: React.FC<LiveChartProps> = ({ws}) => {
  const containerRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    if (!containerRef.current) return;

    const chart: IChartApi = createChart(containerRef.current, {
      width: containerRef.current.clientWidth,
      height: containerRef.current.clientHeight,
    })

    chartSeries = chart.addSeries(AreaSeries);

    var counter = 0;
    Queue = [];

    chart.subscribeDblClick(param => {
      if (param.seriesData && param.seriesData.size > 0) {
        const clickedPoint = param.seriesData.get(chartSeries);

        if (clickedPoint) {
          const clickedIndex = data.findIndex(d => d.time === clickedPoint.time);

          if (clickedIndex !== -1) {
            const newData = data.slice(0, clickedIndex);
            ws.send("update: " + newData[newData.length - 1].value.toString());

            chartSeries.setData(newData);
            data = newData;
            Queue.length = 0;
          }
        }
      }
    });

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

    chartSeries.setData(data);
    chart.timeScale().fitContent();
    let date = new Date(Date.UTC(2018, 12, 31, 12, 0, 0, 0));

    const intervalID = setInterval(() => {
      if (paused) return;

      if (Queue.length === 0) {
        return;
      }

      const val = Queue.shift();

      let year: number | null = null;
      let month: number | null = null;
      let day: number | null = null;
      if (data.length > 0)
      {
        [year, month, day] = data.at(-1)!.time.split("-").map(Number);
      }
      if (year != null && month != null && day != null) {
        console.log(month);
        console.log(day);
        console.log(data.at(-1));
        date = new Date(Date.UTC(year!, month! - 1, day!, 12, 0, 0));
      }

      console.log(date);
      date.setUTCDate(date.getUTCDate() + 1);
      console.log(date);

      var toParse = String(val);
      if (toParse.includes(":")) {
        toParse = toParse.split(":")[1].trim();
      }

      const newDate = date.getFullYear().toString()
        + "-" + (date.getMonth() + 1).toString().padStart(2, "0")
        + "-" + date.getDate().toString().padStart(2, "0");

      console.log(newDate);

      chartSeries.update({
        time: newDate,
        value: Number(toParse)
      });

      data.push({
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

export default function Remix() {
  const [mode, setMode] = useState("sideways");
  const [basePrice, setBasePrice] = useState(0);
  const [percentDrift, setPercentDrift] = useState(0);
  const [percentVolatility, setPercentVolatility] = useState(0);
  const [marketCap, setMarketCap] = useState(0);
  const [targetPrice, setTargetPrice] = useState(0);
  console.log(mode);

  const [uploadFile, setUploadFile] = useState<File | null>(null);

  const socket = useMemo(() => new WebSocket("ws://localhost:18080/websocket"), []);

  const changeMode = (newMode: string) => {
    if (socket.readyState === WebSocket.OPEN) {
      socket.send(newMode);
    }
    setMode(newMode);

    if (newMode === "pause") {
      paused = true;
    } else if (newMode === "resume") {
      paused = false;
    }
  };

  const Form: React.FC = () => {
    const [rewindAmount, setRewindAmount] = useState(0);
    const handleSubmit = (e: React.SubmitEvent<HTMLFormElement>) => {
      e.preventDefault();

      let rewindCount = Math.min(rewindAmount, data.length);
      let remainingSize = data.length - rewindCount;

      let newData = data.slice(0, remainingSize);

      chartSeries.setData(newData);
      data = newData;
      Queue.length = 0;

      socket.send(`rewind:${rewindAmount}`);
      paused = true;
    };

    return (
      <form onSubmit={handleSubmit}>
        <input
          type="number"
          onChange={(e) => setRewindAmount(parseInt(e.target.value))}
        ></input>
        <button type='submit'>Revert</button>
      </form>
    );
  }

  const getFile = (e: React.ChangeEvent<HTMLInputElement>) => {
    if (e.target.files && e.target.files.length > 0) {
      const file = e.target.files[0];
      if (!file.name.endsWith('.sim')) {
        setUploadFile(null);
        e.target.value = "";
        return;
      }
      setUploadFile(file);
    }
  };

  async function handleUpload() {
    if (uploadFile != null) {
      const buffer = await uploadFile.arrayBuffer();

      const response = await fetch(
        "/api/loadSim",
        {
          method: "POST",
          headers: {
            "Content-Type": 'application/octet-stream',
          },
          body: buffer,
        }
      );

      if (response.ok) {
        console.log("Loaded successfully!")
      }

      const json = await response.json();
      console.log(json);
      let dataPoints = json.data;
      console.log(dataPoints);
      let newData = [];
      let date = new Date(Date.UTC(2019, 0, 1, 12, 0, 0, 0));
      for (let i = 0; i < dataPoints.length; i++) {
        date.setUTCDate(date.getUTCDate() + 1);
        const newDate = date.getFullYear().toString()
          + "-" + (date.getMonth() + 1).toString().padStart(2, "0")
          + "-" + date.getDate().toString().padStart(2, "0");
        let entry = { time: newDate, value: dataPoints[i] };
        newData.push(entry);
        console.log(entry);
      }

      console.log(newData);
      chartSeries.setData(newData);
      data = newData;
      Queue.length = 0;

      setBasePrice(json.fields.base_price);
      setPercentDrift(json.fields.percent_drift);
      setPercentVolatility(json.fields.percent_volatility);
      setMarketCap(json.fields.market_cap);
      setTargetPrice(json.fields.target_price);

      paused = true;
    }
  }

  function fieldChanges() {
    socket.send(
      `set_fields:${basePrice}~${percentDrift}~${percentVolatility}~\
        ${marketCap}~${targetPrice}`)

    Queue.length = 0;
  }

  async function downloadSimulation() {
    paused = true;

    const parameters = new URLSearchParams({
      length: `${data.length}`,
    });

    const response = await fetch(
      `/api/serializeSim?${parameters.toString()}`,
      {
        method: "GET",
      }
    );

    const simLoadBuffer: ArrayBuffer = await response.arrayBuffer();

    const blob = new Blob([simLoadBuffer], { type: "application/octet-stream" })
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'simulation.sim';
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);
  }

  return (
    <div className="ChartContainer"
      style={{ display: 'flex', flexDirection: 'column', height: '100vh', width: '100%' }}>

      <div style={{ padding: '15px', display: 'flex', gap: '10px', justifyContent: 'center' }}>
        <Form></Form>
        <button onClick={() => changeMode("pause")}>Pause</button>
        <button onClick={() => changeMode("resume")}>Resume</button>
      </div>

      <button onClick={downloadSimulation}>
        Download Simulation
      </button>

      <div className="sim-form-group" style={{ display: 'flex', gap: '10px' }}>
        <input
          type="file"
          className="sim-input"
          accept=".sim"
          onChange={getFile}
        />
        <button
          className="sim-btn-secondary"
          style={{ marginTop: '0', whiteSpace: 'nowrap' }}
          onClick={handleUpload}
          disabled={!uploadFile}
        >
          Upload
        </button>
      </div>

      <div className="field_editor" style={{ display: 'flex', gap: '10px' }}>
        <input
          type="number"
          value={basePrice}
          className="base_price_input"
          onChange={(e) => { setBasePrice(parseInt(e.target.value)) }}
        />
        <input
          type="number"
          value={percentDrift}
          className="percent_drift_input"
          onChange={(e) => { setPercentDrift(parseFloat(e.target.value)) }}
        />
        <input
          type="number"
          value={percentVolatility}
          className="percent_volatility_input"
          onChange={(e) => { setPercentVolatility(parseFloat(e.target.value)) }}
        />
        <input
          type="number"
          value={marketCap}
          className="market_cap_input"
          onChange={(e) => { setMarketCap(parseInt(e.target.value)) }}
        />
        <input
          type="number"
          value={targetPrice}
          className="target_price_input"
          onChange={(e) => { setTargetPrice(parseInt(e.target.value)) }}
        />
        <button
          className="sim-btn-secondary"
          style={{ marginTop: '0', whiteSpace: 'nowrap' }}
          onClick={fieldChanges}
          // disabled={!uploadFile}
        >
          Set Fields
        </button>
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
