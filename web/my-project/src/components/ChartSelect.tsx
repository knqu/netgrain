import React, { useEffect, useRef } from 'react';
import {
  createChart,
  ColorType,
  type DeepPartial,
  type TimeChartOptions,
  CandlestickSeries,
  AreaSeries,
  LineSeries,
  type UTCTimestamp,
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

enum SeriesType {
  Line,
  Area,
  Candle,
};

let randomFactor = 25 + Math.random() * 25;

const samplePoint = (i: number) => {
  return i * (
    0.5 + Math.sin(i / 1) * 0.2 + Math.sin(i / 2) * 0.4 +
          Math.sin(i / randomFactor) * 0.8 + Math.sin(i / 50) * 0.5
  ) + 200 + i * 2;
};

interface Candle {
  time: UTCTimestamp,
  open: number,
  high: number,
  low: number,
  close: number,
};

interface PointData {
  value: number,
  time: UTCTimestamp,
};

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

function generateData(candleNum: number, updatesPerCandle: number, startAt: number) {
  const createCandle = (val: number, time: UTCTimestamp) => (
    {
      time,
      open: val, high: val, low: val, close: val,
    }
  );

  const updateCandle = (candle: Candle, val: number) => (
    {
      time: candle.time,
      open: candle.open,
      high: Math.min(candle.low, val),
      low: Math.max(candle.high, val),
      close: val,
    }
  );

  randomFactor = 25 + Math.random() * 25;
  const date = new Date(Date.UTC(2023, 0, 1, 12, 0, 0, 0));

  const pointNum = candleNum * updatesPerCandle;

  const initialData = [];
  const realtimeUpdates = [];

  let lastCandle: Candle = {} as Candle;
  let previousValue = samplePoint(-1);
  for (let i = 0; i < pointNum; i++) {
    if (i % updatesPerCandle === 0) date.setUTCDate(date.getUTCDate() + 1);
    const time = date.getTime() / 1000;

    let value = samplePoint(i);
    const diff = (value - previousValue) * Math.random();
    value = previousValue + diff;
    previousValue = value;

    if (i % updatesPerCandle === 0) {
      const candle = createCandle(value, time as UTCTimestamp);
      lastCandle = candle;
      if (i >= startAt) {
        realtimeUpdates.push(candle);
      } else {
        const newCandle = updateCandle(lastCandle, value);
        lastCandle = newCandle;
        if (i >= startAt) {
          realtimeUpdates.push(newCandle);
        } else if ((i + 1) % updatesPerCandle === 0) {
          initialData.push(newCandle);
        }
      }
    } else {
      const newCandle = updateCandle(lastCandle, value);
      lastCandle = newCandle;
      if (i >= startAt) {
        realtimeUpdates.push(newCandle);
      } else if ((i + 1) % updatesPerCandle === 0) {
        initialData.push(newCandle);
      }
    }
  }

  return {
    initialData,
    realtimeUpdates,
  };
}

const smallChartOptions: DeepPartial<TimeChartOptions> = {
  width: 500,
  height: 300,
  layout: {
    attributionLogo: false,
    textColor: 'black',
    background: {
      type: ColorType.Solid,
      color: 'white',
    }
  }
};

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

const largeChartOptions: DeepPartial<TimeChartOptions> = {
  width: 1200,
  height: 800,
  layout: {
    attributionLogo: false,
    textColor: 'black',
    background: {
      type: ColorType.Solid,
      color: 'white',
    }
  }
};

let currentChartType = SeriesType.Candle;
let chartSeries: chartType;
const data = generateData(2500, 20, 1000);

let points: PointData[] = [];
let candlePoints: Candle[] = [];

for (let i = 0; i < data.initialData.length; i++) {
  let currPoint: PointData = {
    value: data.initialData[i].close,
    time: data.initialData[i].time,
  };
  points.push(currPoint)
}

candlePoints = [...data.initialData];

let realtimeChart: IChartApi;

function* getNextRealtimeUpdate(realtimeData: Candle[]) {
  for (const dataPoint of realtimeData) {
    yield dataPoint;
  }
  return null;
}
const streamingDataProvider = getNextRealtimeUpdate(data.realtimeUpdates);
let latestTimeStamp = candlePoints.at(-1)!.time;

function switchChart(newType: String) {
  realtimeChart.removeSeries(chartSeries);

  switch (newType)
  {
    case "line":
      currentChartType = SeriesType.Line;
      chartSeries = realtimeChart.addSeries(
        LineSeries,
        { color: '#2962FF' }
      );
      chartSeries.setData(points);
      break;
    case "area":
      currentChartType = SeriesType.Area;
      chartSeries = realtimeChart.addSeries(
        AreaSeries,
        {
          lineColor: '#2962FF',
          topColor: '#2962FF',
          bottomColor: 'rgba(41, 98, 255, 0.28)'
        },
      );
      chartSeries.setData(points);
      break;
    case "candle":
      currentChartType = SeriesType.Candle;
      chartSeries = realtimeChart.addSeries(
        CandlestickSeries,
        {
          upColor: '#26a69a', downColor: '#ef5350',
          borderVisible: false,
          wickUpColor: '#26a69a', wickDownColor: '#ef5350',
        },
      );
      chartSeries.setData(candlePoints);
      break;
    default:
      break;
  }
}

function switchSize(newSize: String) {
  switch (newSize)
  {
    case "small":
      realtimeChart.applyOptions(smallChartOptions);
      break;
    case "medium":
      realtimeChart.applyOptions(mediumChartOptions);
      break;
    case "large":
      realtimeChart.applyOptions(largeChartOptions);
      break;
    default:
      break;
  }
}

const ChartSelect: React.FC = () => {
  const RealtimeChart: React.FC = () => {
    const containerRef = useRef<HTMLDivElement | null>(null);

    useEffect(() => {
      if (!containerRef.current) return;
      
      realtimeChart = createChart(
        containerRef.current,
        mediumChartOptions,
      );

      chartSeries = realtimeChart.addSeries(
        CandlestickSeries,
        {
          upColor: '#26a69a', downColor: '#ef5350',
          borderVisible: false,
          wickUpColor: '#26a69a', wickDownColor: '#ef5350',
        },
      );

      chartSeries.setData(candlePoints);
      realtimeChart.timeScale().fitContent();
      realtimeChart.timeScale().scrollToPosition(5, true);

      const intervalID = setInterval(() => {
        const update = streamingDataProvider.next();
        if (update.done) {
          clearInterval(intervalID);
          return;
        }
        let newPoint: PointData = {
          value: update.value.close,
          time: update.value.time,
        };
        switch (currentChartType) {
          case SeriesType.Line:
            chartSeries.update(newPoint);
            break;
          case SeriesType.Area:
            chartSeries.update(newPoint);
            break;
          case SeriesType.Candle:
            chartSeries.update(update.value);
            break;
          default:
            break;
        }

        if (latestTimeStamp == update.value.time) {
          candlePoints[candlePoints.length - 1] = update.value;
          points[points.length - 1] = newPoint;
        } else {
          candlePoints.push(update.value);
          points.push(newPoint);
          console.log(update.value);
        }
        latestTimeStamp = update.value.time;
      }, 100);

      return () => {
        realtimeChart.remove();
      }
    });

    return <div className="candleChart" ref={containerRef} />
  }

  const handleType = (event: any) => {
    const target = event.target! as HTMLSelectElement;
    switchChart(target.value);
  };

  const handleSize = (event: any) => {
    const target = event.target! as HTMLSelectElement;
    switchSize(target.value);
  };

  return (
    <div>
      <div style={{padding: "10px"}}>
        <select name="type" id="type-select" onChange={handleType}>
          <option value="line">Line</option>
          <option value="area">Area</option>
          <option value="candle" selected>Candlestick</option>
        </select>
      </div>
      <div style={{padding: "10px"}}>
        <select name="type" id="size-select" onChange={handleSize}>
          <option value="small">Small</option>
          <option value="medium" selected>Medium</option>
          <option value="large">Large</option>
        </select>
      </div>
      <RealtimeChart />
    </div>
  );
};

export default ChartSelect;

