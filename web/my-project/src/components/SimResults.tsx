import '../styling/SimResults.css'

import ChartComponent from './Chart';
import SimulationResultsComponent from './AlgoTable'

export default function SimResults() {
    return (
        <div className="Analytics_container">
            <div className="Analytics_header_container">
                    <h5>Simulation Name</h5>
                    <h5>00 Month 0000</h5>
            </div>

            <div className="Chart_Table_container">
                <ChartComponent></ChartComponent>
                <SimulationResultsComponent></SimulationResultsComponent>
            </div>
        </div>
    );
}