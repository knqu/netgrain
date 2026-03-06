import '../styling/Chart.css';
import React, { useEffect, useRef } from "react";
import {
    createChart,
    CandlestickSeries,
    type CandlestickData,
    type IChartApi,
} from "lightweight-charts";

export default function ChartComponent() {
    const CandleStickChart: React.FC = () => {
        const containerRef = useRef<HTMLDivElement | null>(null);

        useEffect(() => {
            if (!containerRef.current) return;
            const chart: IChartApi = createChart(containerRef.current, {
                width: containerRef.current.clientWidth,
                height: containerRef.current.clientHeight,
            })

            const candleSeries = chart.addSeries(CandlestickSeries);

            const data: CandlestickData[] = [
                { time: "2025-03-01", open: 100, high: 110, low: 95, close: 105 },
                { time: "2025-03-02", open: 105, high: 112, low: 101, close: 108 },
                { time: "2025-03-03", open: 108, high: 115, low: 107, close: 111 },
                { time: "2025-03-04", open: 111, high: 118, low: 109, close: 116 },
                { time: "2025-03-05", open: 116, high: 120, low: 113, close: 114 },
                { time: "2025-03-06", open: 114, high: 119, low: 112, close: 118 },
            ];

            candleSeries.setData(data);
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
                        <select multiple size={1} />
                    </div>

                    <div className="Selector_container">
                        <h5>Select Time</h5>
                        <select multiple size={1} />
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