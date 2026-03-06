import '../styling/Chart.css';
import React, { useEffect, useRef, useState } from "react";
import {
    createChart,
    CandlestickSeries,
    type CandlestickData,
    type IChartApi,
} from "lightweight-charts";

export default function ChartComponent() {
    const [stockData, setStockData] = useState<CandlestickData[]>([
        { time: "2025-03-01", open: 100, high: 110, low: 95, close: 105 },
        { time: "2025-03-02", open: 105, high: 112, low: 101, close: 108 },
        { time: "2025-03-03", open: 108, high: 115, low: 107, close: 111 },
        { time: "2025-03-04", open: 111, high: 118, low: 109, close: 116 },
        { time: "2025-03-05", open: 116, high: 120, low: 113, close: 114 },
        { time: "2025-03-06", open: 114, high: 119, low: 112, close: 118 },
    ]);
    const [stock, setStock] = useState<string>("aapl");

    function loadData(stock : string) {
        switch (stock) {
            case "aapl":
                setStockData([
                    {time : "2017-10-26", open: 156.67, high: 157.26, low: 156.22, close : 156.85},
                    {time : "2017-10-27", open: 158.72, high: 163.01, low: 158.13, close : 162.12},
                    {time : "2017-10-30", open: 163.3, high: 167.47, low: 163.13, close : 166.12},
                    {time : "2017-10-31", open: 167.3, high: 169.04, low: 166.34, close : 168.43},
                    {time : "2017-11-01", open: 169.26, high: 169.33, low: 165.02, close : 166.29},
                    {time : "2017-11-02", open: 167.04, high: 167.9, low: 164.69, close : 167.51},
                    {time : "2017-11-03", open: 173.38, high: 173.64, low: 170.51, close : 171.88},
                    {time : "2017-11-06", open: 171.75, high: 174.36, low: 171.1, close : 173.63},
                    {time : "2017-11-07", open: 173.29, high: 174.51, low: 173.29, close : 174.18},
                    {time : "2017-11-08", open: 174.03, high: 175.61, low: 173.71, close : 175.61},
                    {time : "2017-11-09", open: 174.48, high: 175.46, low: 172.52, close : 175.25},
                    {time : "2017-11-10", open: 175.11, high: 175.38, low: 174.27, close : 174.67},
                ]);
                break;
            case "amzn":
                setStockData([
                    {time : "2017-10-26", open : 980.33, high: 982.9, low: 968.55, close: 972.43},
                    {time : "2017-10-27", open : 1057.01, high: 1105.58, low: 1050.55, close: 1100.95},
                    {time : "2017-10-30", open : 1095.01, high: 1122.79, low: 1093.56, close: 1110.85},
                    {time : "2017-10-31", open : 1109, high: 1110.54, low: 1101.12, close: 1105.28,},
                    {time : "2017-11-01", open : 1105.4, high: 1108.97, low: 1096.74, close: 1103.68},
                    {time : "2017-11-02", open : 1097.81, high: 1101.94, low: 1086.87, close: 1094.22},
                    {time : "2017-11-03", open : 1091.15, high: 1112.68, low: 1088.52, close: 1111.6},
                    {time : "2017-11-06", open : 1109.15, high: 1125.41, low: 1108.77, close: 1120.66},
                    {time : "2017-11-07", open : 1124.74, high: 1130.6, low: 1117.5, close: 1123.17},
                    {time : "2017-11-08", open : 1122.82, high: 1135.54, low: 1119.11, close: 1132.88},
                    {time : "2017-11-09", open : 1125.96, high: 1129.62, low: 1115.77, close: 1129.13},
                    {time : "2017-11-10" , open: 1126.1, high: 1131.75, low: 1124.06, close: 1125.35}
                ]);
                break;
            case "googl" :
                setStockData([
                    {time : "2017-10-26", open : 998.47, high : 1006.51, low : 990.47, close: 991.42},
                    {time : "2017-10-27", open : 1030.99, high : 1063.62, low : 1027.46, close: 1033.67},
                    {time : "2017-10-30", open : 1029.16, high : 1039.83, low : 1022.33, close: 1033.13},
                    {time : "2017-10-31", open : 1033, high : 1041, low : 1026.3, close: 1033.04},
                    {time : "2017-11-01", open : 1036.32, high : 1047.86, low : 1034, close: 1042.6},
                    {time : "2017-11-02", open : 1039.99, high : 1045.52, low : 1028.66, close: 1042.97},
                    {time : "2017-11-03", open : 1042.75, high : 1050.66, low : 1037.65, close: 1049.99},
                    {time : "2017-11-06", open : 1049.1, high : 1052.59, low : 1042, close: 1042.68},
                    {time : "2017-11-07", open : 1049.65, high : 1053.41, low : 1043, close: 1052.39},
                    {time : "2017-11-08", open : 1050.05, high : 1062.69, low : 1047.05, close: 1058.29},
                    {time : "2017-11-09", open : 1048, high : 1050.88, low : 1035.85, close: 1047.72},
                    {time : "2017-11-10", open : 1043.87, high : 1046.63, low : 1041.22, close: 1044.15},
                ]);
                break;
        }
    }
    const CandleStickChart: React.FC = () => {
        const containerRef = useRef<HTMLDivElement | null>(null);

        useEffect(() => {
            if (!containerRef.current) return;
            const chart: IChartApi = createChart(containerRef.current, {
                width: containerRef.current.clientWidth,
                height: containerRef.current.clientHeight,
            })

            const candleSeries = chart.addSeries(CandlestickSeries);

            candleSeries.setData(stockData);
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

        return <div className="candleChart" ref={containerRef} />
    };

    function ChartHeader() {
        return (
            <div className="Chart_header_container">
                <div className="Chart_header">
                    <div className="Selector_container">
                        <h5>Select Stocks</h5>
                        <select required value={stock} size={1} onChange={(e) => {loadData(e.target.value); setStock(e.target.value)}} id="stockSelector">
                            <option value="aapl">Apple</option>
                            <option value="amzn">Amazon</option>
                            <option value="googl">Google</option>
                        </select>
                    </div>
                </div>
            </div>
        );
    }

    // Chart body
    function Chart() {
        return (
            <div className="Chart_outer_container">
                <div className="Chart_inner_container">
                    <div className="Chart" >
                        <CandleStickChart></CandleStickChart>
                    </div>
                </div>
            </div>
        );
    }

    return (
        <div className="ChartContainer">
            <ChartHeader />
            <Chart />
        </div>
    );
}