import {
  AreaSeries,
  createChart,
  ColorType,
  type DeepPartial,
  type TimeChartOptions,
  CandlestickSeries,
  type UTCTimestamp
} from 'lightweight-charts';

import './style.css';

document.querySelector<HTMLDivElement>('#app')!.innerHTML = `
<h1>Hello, World!</h1>
<div id="chartContainer"></div>
<div id="realtimeContainer"></div>
`

const chartOptions: DeepPartial<TimeChartOptions> = {
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

const firstChart = createChart(
  document.getElementById('chartContainer')!,
  chartOptions,
);

const areaSeries = firstChart.addSeries(
  AreaSeries,
  {
    lineColor: '#2962ff', topColor: '#2962ff',
    bottomColor: 'rgba(41, 98, 255, 0.28)',
  },
);

areaSeries.setData(
  [
    { time: '2023-12-22', value: 32.51 },
    { time: '2023-12-23', value: 31.11 },
    { time: '2023-12-24', value: 27.02 },
    { time: '2023-12-25', value: 27.32 },
    { time: '2023-12-26', value: 25.17 },
    { time: '2023-12-27', value: 28.89 },
    { time: '2023-12-28', value: 25.46 },
    { time: '2023-12-29', value: 23.92 },
    { time: '2023-12-30', value: 22.68 },
    { time: '2023-12-31', value: 22.67 },
  ]
);

const candlestickSeries = firstChart.addSeries(
  CandlestickSeries,
  {
    upColor: '#26a69a', downColor: '#ef5350',
    borderVisible: false,
    wickUpColor: '#26a69a', wickDownColor: '#ef5350',
  },
);

candlestickSeries.setData(
  [
    { time: '2023-12-22', open: 75.16, high: 82.84, low: 36.16, close: 45.72 },
    { time: '2023-12-23', open: 45.12, high: 53.90, low: 45.12, close: 48.09 },
    { time: '2023-12-24', open: 60.71, high: 60.71, low: 53.39, close: 59.29 },
    { time: '2023-12-25', open: 68.26, high: 68.26, low: 59.04, close: 60.50 },
    { time: '2023-12-26', open: 67.71, high: 105.85, low: 66.67, close: 91.04 },
    { time: '2023-12-27', open: 91.04, high: 121.40, low: 82.70, close: 111.40 },
    { time: '2023-12-28', open: 111.51, high: 142.83, low: 103.34, close: 131.25 },
    { time: '2023-12-29', open: 131.33, high: 151.17, low: 77.68, close: 96.43 },
    { time: '2023-12-30', open: 106.33, high: 110.20, low: 90.39, close: 98.10 },
    { time: '2023-12-31', open: 109.87, high: 114.69, low: 85.66, close: 111.26 },
  ]
);

firstChart.timeScale().fitContent();

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

  let lastCandle;
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
    }
  }

  return {
    initialData,
    realtimeUpdates,
  };
}

const realtimeChartOptions: DeepPartial<TimeChartOptions> = {
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

const realtimeChart = createChart(
  document.getElementById('realtimeContainer')!,
  realtimeChartOptions,
);

const realtimeSeries = realtimeChart.addSeries(
  CandlestickSeries,
  {
    upColor: '#26a69a', downColor: '#ef5350',
    borderVisible: false,
    wickUpColor: '#26a69a', wickDownColor: '#ef5350',
  },
);

const data = generateData(2500, 20, 1000);

realtimeSeries.setData(data.initialData);
realtimeChart.timeScale().fitContent();
realtimeChart.timeScale().scrollToPosition(5, true);

function* getNextRealtimeUpdate(realtimeData: Candle[]) {
  for (const dataPoint of realtimeData) {
    yield dataPoint;
  }
  return null;
}
const streamingDataProvider = getNextRealtimeUpdate(data.realtimeUpdates);

const intervalID = setInterval(() => {
  const update = streamingDataProvider.next();
  if (update.done) {
    clearInterval(intervalID);
    return;
  }
  realtimeSeries.update(update.value);
}, 100);


