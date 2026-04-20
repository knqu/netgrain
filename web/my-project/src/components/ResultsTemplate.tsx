import '../styling/ResultsTemplate.css'

import ChartComponent from './Chart';
import SimulationResultsComponent from './AlgoTable'
import Metrics from './Metrics';

export default function ResultsTemplate() {
    return (
        <div className="Analytics_container">
            <div className="Analytics_header_container">
                    <h5>Simulation Name</h5>
                    <h5>00 Month 0000</h5>
            </div>

            <div>
                <div className="Chart_Table_container">
                    <ChartComponent></ChartComponent>
                    <SimulationResultsComponent></SimulationResultsComponent>
                </div>

                <div className="">
                    <Metrics></Metrics>
                </div>
            </div>
        </div>
    );
}