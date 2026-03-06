import '../styling/Chart.css'

function ChartHeader() {
    return (
        <div className="Chart_header_container">
            <div className="Chart_header">
                <div className="Selector_container">
                    <h5>Select Stocks</h5>
                    <select multiple size={1} />
                </div>

                <div className="Selector_container">
                    <h5>Select Stocks</h5>
                    <select multiple size={1} />
                </div>
            </div>
        </div>
    );
}

function Chart() {
    return (
        <div className="Chart_outer_container">
            <div className="Chart_inner_container">
                <div className="Chart">

                </div>
            </div>
        </div>
    );
}

export default function ChartComponent() {
    return (
        <div className="ChartContainer">
            <ChartHeader></ChartHeader>
            <Chart></Chart>
        </div>
    );
}