import '../styling/ResultsTemplate.css'
import {
    createChart,
    type IChartApi,
    LineSeries,
    type LineData,
    HistogramSeries,
    type HistogramData,
} from "lightweight-charts";
import { useState, useRef, useEffect, type ReactElement, use } from 'react';
import nextArrow from "../assets/next.png";
import pause from "../assets/pause.png"

/*
const equityTest: LineData<number>[] =  [{ "time" : 1, "value" : 101500},{ "time" : 2, "value" : 101780},{ "time" : 3, "value" : 100085},{ "time" : 3, "value" : 102185},{ "time" : 4, "value" : 105635},{ "time" : 4, "value" : 107555},{ "time" : 5, "value" : 106444},{ "time" : 6, "value" : 107274},{ "time" : 7, "value" : 105829},{ "time" : 8, "value" : 108455}];

const ddTest : LineData<number>[] =  [{ "time" : 1, "value" : 0 },{ "time" : 2, "value" : 0 },{ "time" : 3, "value" : -0.016654 },{ "time" : 3, "value" : 0 },{ "time" : 4, "value" : 0 },{ "time" : 4, "value" : 0 },{ "time" : 5, "value" : -0.010330 },{ "time" : 6, "value" : -0.002613 },{ "time" : 7, "value" : -0.016048 },{ "time" : 8, "value" : 0 }];

type TableEntry = {
        "timestamp": number;
        "orderType": string;
        "amountOfStock": number;
        "moneyGained": number;
        "totalMoney": number;
    }

const tableTest: TableEntry[] = [{"timestamp" : 1, "orderType" : "BUY", "amountOfStock" : 10, "moneyGained" : -1500, "totalMoney" : 100000},{"timestamp" : 2, "orderType" : "BUY", "amountOfStock" : 5, "moneyGained" : -760, "totalMoney" : 99500},{"timestamp" : 3, "orderType" : "SELL", "amountOfStock" : 8, "moneyGained" : 1240, "totalMoney" : 99000},{"timestamp" : 3, "orderType" : "BUY", "amountOfStock" : 3, "moneyGained" : -2100, "totalMoney" : 99000},{"timestamp" : 4, "orderType" : "BUY", "amountOfStock" : 2, "moneyGained" : -1420, "totalMoney" : 101000},{"timestamp" : 4, "orderType" : "BUY", "amountOfStock" : 6, "moneyGained" : -1920, "totalMoney" : 101000},{"timestamp" : 5, "orderType" : "SELL", "amountOfStock" : 4, "moneyGained" : 632, "totalMoney" : 100500},{"timestamp" : 6, "orderType" : "SELL", "amountOfStock" : 1, "moneyGained" : 720, "totalMoney" : 102000},{"timestamp" : 7, "orderType" : "SELL", "amountOfStock" : 3, "moneyGained" : 975, "totalMoney" : 101500},{"timestamp" : 8, "orderType" : "BUY", "amountOfStock" : 7, "moneyGained" : -1120, "totalMoney" : 103000}];
const plTest: HistogramData<number>[] = [{"time" : 3, "value" : 40, "color" : "green"},{"time" : 5, "value" : 28, "color" : "green"},{"time" : 6, "value" : 20, "color" : "green"},{"time" : 7, "value" : 15, "color" : "green"}];
*/
export default function ResultsTemplate() {
    type TableEntry = {
        "timestamp": number;
        "orderType": string;
        "amountOfStock": number;
        "moneyGained": number;
        "totalMoney": number;
    }

    const [sim, setSim] = useState<string>("0");
    const [title, setTitle] = useState<string>("Equity Curve over Time");

    const [chartIndex, setChartIndex] = useState<number>(0);
    const timer = useRef<number | null>(null);

    const [tableData, setTableData] = useState<TableEntry[]>([]);
    const [equityData, setEquityData] = useState<LineData<number>[]>([]);
    const [plData, setPlData] = useState<HistogramData<number>[]>([]);
    const [ddData, setDdData] = useState<LineData<number>[]>([]);

    const [taxes, setTaxes] = useState<number>(0);
    const [comm, setComm] = useState<number>(0);
    const [fee, setFee] = useState<number>(0);
    const [winRate, setWinRate] = useState<number>(0);
    const [equity, setEquity] = useState<number>(0);
    const [trades, setTrades] = useState<number>(0);
    const [profitFactor, setProfitFactor] = useState<number>(0);
    const [EV, setEV] = useState<number>(0);
    const [maxDD, setMaxDD] = useState<number>(0);

    useEffect(() => {  
        const fetchMetrics = async () => {
        try {
            const response = await fetch(
            "/api/resultsTemplate",
            {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ simID: Number(sessionStorage.getItem("simID")) })
            });
            console.log(response);
            sessionStorage.clear();

            if (response.ok) {
                const data = await response.json();
                console.log(response);
                setTableData(data["table"]);
                setEquityData(data["equity"]);
                setPlData(data["pl"]);
                setDdData(data["drawdown"]);
                setTaxes(parseFloat(data.taxes));
                setFee(parseFloat(data.flat));
                setComm(parseFloat(data.percent));
            }
        } catch (err) {
            console.log(err);
        }};

        fetchMetrics();
    }, []);

    function handleTimer() {
        if (timer.current != null) {
          clearInterval(timer.current);
          timer.current = null;
        } else {
            timer.current = setInterval(() => setChartIndex(chartIndex => (chartIndex + 1) % 3), 2000);
        }
    }

    function pauseTimer () {
        if (timer.current != null) {
          clearInterval(timer.current);
          timer.current = null;
        } 
    }

    function CreateChart() {
        const containerRef = useRef<HTMLDivElement | null>(null);
        const chartRef = useRef<IChartApi | null>(null);
        const seriesRef = useRef<any | null>(null);

        useEffect(() => {
            if (!containerRef.current) return;

            const resizeObserver = new ResizeObserver(() => {
                if (!chartRef.current) return;
                chartRef.current.applyOptions({
                    width: containerRef.current!.clientWidth,
                    height: containerRef.current!.clientHeight,
                });
            });

            chartRef.current = createChart(containerRef.current, {
                width: containerRef.current.clientWidth,
                height: containerRef.current.clientHeight,
                layout: {
                    textColor: "black",
                    background: { color: "white" }
                },
                timeScale: {
                    timeVisible: true,
                    secondsVisible: true,
                    tickMarkFormatter: (time: number) => `${time}s`,
                },
                localization: {
                    timeFormatter: (time: number) => `${time}s`,
                },
            });

            resizeObserver.observe(containerRef.current);
            if (chartIndex == 0) {
                seriesRef.current = chartRef.current.addSeries(LineSeries);
                seriesRef.current.setData(equityData);
                setTitle("Equity Curve");

            }
            else if (chartIndex == 1) {
                seriesRef.current = chartRef.current.addSeries(HistogramSeries);
                seriesRef.current.setData(plData);
                setTitle("Profit and Loss Curve");
            }
            else if (chartIndex == 2) {
                seriesRef.current = chartRef.current.addSeries(LineSeries);
                seriesRef.current.setData(ddData);
                setTitle("Drawdown Curve");
            }

            chartRef.current.timeScale().fitContent();

            return () => {
                resizeObserver.disconnect();
                chartRef.current?.remove();
            };
        }, [chartIndex]);

        return <div className="chart" ref={containerRef}></div>
    }

    function TableRow({ algo_entry }: { algo_entry: TableEntry }) {
        return (
            <div className='SimResult_row_container'>
                <div className="table-row-content">
                    <h5>{algo_entry.timestamp}</h5>
                    <h5>{algo_entry.orderType}</h5>
                    <h5>{algo_entry.amountOfStock}</h5>
                    <h5 style={{ color: algo_entry.moneyGained > 0 ? 'rgb(9, 215, 23)' : 'rgb(215, 9, 23)' }}>
                        {algo_entry.moneyGained}
                    </h5>
                    <h5>${algo_entry.totalMoney}</h5>
                </div>
            </div>
        );
    }

    function AlgoTable({ TableEntryList }: { TableEntryList: TableEntry[] }) {
        let tableRows: ReactElement[] = [];
        TableEntryList.forEach((row) => {
            tableRows.push(
                <TableRow algo_entry={row}></TableRow>
            )
        });

        return (
            <>{tableRows} </>
        );
    }

    function download() {
        const json = JSON.stringify(tableData, null, 2);

        const blob = new Blob([json], { type: "application/json" });
        const url = URL.createObjectURL(blob);

        const a = document.createElement("a");
        a.href = url;
        a.download = "metrics.json";
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);

        URL.revokeObjectURL(url);
    }

    return (
        <div className="analytics-parent-container">
            <div className="analytics-container">
                <div className="top-container">
                    <div className="header-container">
                        <h5>Simulation {sim}</h5>
                    </div>

                    <button className="export" onClick={() => download()}>Export</button>
                </div>

                <div className="charts-table-metric-container">
                    <div className="charts-parent-container">
                        <h2 className="chart-title">{title}</h2>
                        <div className="charts-container">
                            <CreateChart></CreateChart>
                        </div>

                        <div className="button-group">
                            <button onClick={() => {
                                setChartIndex(prev => prev == 2 ? prev = 0 : ++prev);
                                pauseTimer()
                            }}>
                                <img src={nextArrow} style={{ transform: "rotate(-90deg)" }}></img>
                            </button>

                            <button onClick={handleTimer}>
                                <img src={pause}></img>
                            </button>

                            <button onClick={() => {
                                setChartIndex(prev => prev == 0 ? prev = 2 : --prev);
                                pauseTimer()
                            }}>
                                <img src={nextArrow} style={{ transform: "rotate(90deg)" }}></img>
                            </button>
                        </div>
                    </div>

                    <div className="table-container">
                        <div className="table-header">
                            <h5>Time</h5>
                            <h5>Order Type</h5>
                            <h5># of Stocks</h5>
                            <h5>Money</h5>
                            <h5>Total Capital</h5>
                        </div>

                        <div className="table">
                            <AlgoTable TableEntryList={tableData}></AlgoTable>
                        </div>
                    </div>
                </div>

                <div className="metric-grid">
                    <div className="metric-container">
                        <p className="metric-title">Percent Commision</p>
                        <p className="metric">{isNaN(comm) ? "Unavailable" : `${comm}%`}</p>
                    </div>

                    <div className="metric-container">
                        <p className="metric-title">Flat Fee</p>
                        <p className="metric">{isNaN(fee) ? "Unavailable" : `-$${fee}`}</p>
                    </div>

                    <div className="metric-container">
                        <p className="metric-title">Taxes</p>
                        <p className="metric">{isNaN(taxes) ? "Unavailable" : `-$${taxes}`}</p>
                    </div>

                    <div className="metric-container">
                        <p className="metric-title">Total Equity</p>
                        <p className="metric">${equity}</p>
                    </div>

                    <div className="metric-container">
                        <p className="metric-title">Win Rate</p>
                        <p className="metric">{winRate}%</p>
                    </div>

                    <div className="metric-container">
                        <p className="metric-title">Profit Factor</p>
                        <p className="metric">{profitFactor}</p>
                    </div>

                    <div className="metric-container">
                        <p className="metric-title">Total Trades</p>
                        <p className="metric">{trades}</p>
                    </div>

                    <div className="metric-container">
                        <p className="metric-title">Expected Value per Trade</p>
                        <p className="metric">${EV}</p>
                    </div>

                    <div className="metric-container">
                        <p className="metric-title">Maximum Drop Down</p>
                        <p className="metric">${maxDD}</p>
                    </div>
                </div>
            </div>


        </div>
    );
}
