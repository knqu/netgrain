import "../styling/SimulationRun.css"
import { useState } from 'react';
import ChartComponent from './LiveChart';
//import type LiveChartProps from './LiveChart';

// --- Main Dashboard ---
export default function SimulationRun() {

  const [activeStock, setActiveStock] = useState('1');
  const wsUri = "ws://localhost:5555/";
  const websocket = new WebSocket(wsUri);
  const wsUri2 = "ws://localhost:5556/";
  const websocket2 = new WebSocket(wsUri2);
  return (
    <div className="chart-wrapper">
      <div style={{ marginBottom: '10px' }}>
        <button onClick={() => setActiveStock('1')}>Stock 1</button>
        <button onClick={() => setActiveStock('2')}>Stock 2</button>
      </div>
      <div className={activeStock === '1' ? 'show-chart' : 'hide-chart'}>
        <ChartComponent ws={websocket} />
      </div>
      <div className={activeStock === '2' ? 'show-chart' : 'hide-chart'}>
        <ChartComponent ws={websocket2}/>
      </div>
      
    </div>
  );
}
