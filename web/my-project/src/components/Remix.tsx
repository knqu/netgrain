import '../styling/Chart.css';
import React, { useEffect, useRef } from "react";

import {
  createChart,
  ColorType,
  type DeepPartial,
  type TimeChartOptions,
  CandlestickSeries,
} from 'lightweight-charts';

import type {
  IChartApi,
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

let liveChart: IChartApi;
let chartSeries: chartType;

const mediumChartOptions: DeepPartial<TimeChartOptions> = {
  width: 900,
  height: 600,
  layout: {
    attributionLogo: false,
    textColor: 'black',
    background: {
      type: ColorType.Solid,
      color: 'white',
    }
  }
};

const pickerOpts = {
  types: [
    {
      description: "Simulations",
      accept: {
        "application/octet-stream": [ ".sim" ],
      },
    }
  ],
  multiple: false,
};

let fileHandle: FileSystemFileHandle;

async function getFile() {
  [fileHandle] = await (window as any).showOpenFilePicker(pickerOpts);

  const fileData = await fileHandle.getFile();
  return fileData;
}

const LiveChart: React.FC = () => {
  const containerRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    if (!containerRef.current) return;

    liveChart = createChart(
      containerRef.current,
      mediumChartOptions,
    );

    chartSeries = liveChart.addSeries(
      CandlestickSeries,
      {
        upColor: '#26a69a', downColor: '#ef5350',
        borderVisible: false,
        wickUpColor: '#26a69a', wickDownColor: '#ef5350',
      },
    );

    liveChart.timeScale().fitContent();
    liveChart.timeScale().scrollToPosition(5, true);

    return () => {
      liveChart.remove();
    };
  });

  return <div className="candleChart" ref={containerRef} />
};

const Remix: React.FC = () => {
  return(
    <div>
      <LiveChart />
    </div>
  );
};

export default Remix;

