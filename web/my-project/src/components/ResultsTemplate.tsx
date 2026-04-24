import '../styling/ResultsTemplate.css'
import {
    createChart,
    type IChartApi,
    LineSeries,
    type LineData,
    HistogramSeries,
    type HistogramData,
} from "lightweight-charts";
import { useState, useRef, useEffect, type ReactElement } from 'react';
import nextArrow from "../assets/next.png";
import pause from "../assets/pause.png"

type TableEntry = {
    "timestamp" : number;
    "orderType" : string;
    "amountOfStock": number;
    "moneyGained": number;
    "totalMoney": number;
  }

const data: LineData<number>[] = [
    { time: 0, value: 1100 },
    { time: 1, value: 1090 },
    { time: 2, value: 1150 },
    { time: 3, value: 1130 },
    { time: 4, value: 1170 },
    { time: 5, value: 1205 },
    { time: 6, value: 1268 },
    { time: 7, value: 1352 },
    { time: 8, value: 1300 },
    { time: 9, value: 1280 },
    { time: 10, value: 1299 },
    { time: 11, value: 1305 },
    { time: 12, value: 1353 },
    { time: 13, value: 1352 },
    { time: 14, value: 1398 },
    { time: 15, value: 1400 },
];

const testData : TableEntry[] = [
  {timestamp : 0, orderType : "N/A", amountOfStock : 0, moneyGained : 0, totalMoney : 200},
  {timestamp : 1, orderType : "Buy", amountOfStock : 2, moneyGained : -6, totalMoney : 194},
  {timestamp : 2, orderType : "Buy", amountOfStock : 5, moneyGained : -10, totalMoney : 184},
  {timestamp : 5, orderType : "Sell", amountOfStock : 3, moneyGained : 20, totalMoney : 164},
  {timestamp : 7, orderType : "Sell", amountOfStock : 1, moneyGained : 3, totalMoney : 167},
  {timestamp : 10, orderType : "Buy", amountOfStock : 10, moneyGained : -5, totalMoney : 162},
  {timestamp : 13, orderType : "Sell", amountOfStock : 7, moneyGained : -15, totalMoney : 147},
  {timestamp : 15, orderType : "Buy", amountOfStock : 27, moneyGained : -58, totalMoney : 89},
  {timestamp : 20, orderType : "Sell", amountOfStock : 9, moneyGained : 60, totalMoney : 149},
  {timestamp : 24, orderType : "Sell", amountOfStock : 1, moneyGained : 1, totalMoney : 150},
  {timestamp : 24, orderType : "Sell", amountOfStock : 1, moneyGained : 1, totalMoney : 150},
  {timestamp : 24, orderType : "Sell", amountOfStock : 1, moneyGained : 1, totalMoney : 150},
  {timestamp : 24, orderType : "Sell", amountOfStock : 1, moneyGained : 1, totalMoney : 150},
  {timestamp : 24, orderType : "Sell", amountOfStock : 1, moneyGained : 1, totalMoney : 150},
  {timestamp : 24, orderType : "Sell", amountOfStock : 1, moneyGained : 1, totalMoney : 150},
];

const HistData: HistogramData<number>[] = [
    { time: 1, value: 50, color: "green" },
    { time: 2, value: -20, color: "red" },
    { time: 3, value: 30, color: "green" },
    { time: 4, value: -10, color: "red" },
    { time: 5, value: 40, color: "green" },
    { time: 6, value: -5, color: "red" },
    { time: 7, value: 60, color: "green" },
    { time: 8, value: -15, color: "red" },
    { time: 9, value: 25, color: "green" },
    { time: 10, value: -30, color: "red" },
    { time: 11, value: 35, color: "green" },
    { time: 12, value: -10, color: "red" },
    { time: 13, value: 20, color: "green" },
    { time: 14, value: -5, color: "red" },
    { time: 15, value: 50, color: "green" },
  ];

export default function ResultsTemplate() {
    const [timeTitle, setTimeTitle] = useState<string>("Equity Curve over Time");
    const [tradeTitle, setTradeTitle] = useState<string>("Profit and Loss Curve");

    const [timeIndex, setTimeIndex] = useState<number>(0);
    const [tradeIndex, setTradeIndex] = useState<number>(3);
    
    const [timeTimer, setTimeTimer] = useState<number | null>(null);
    const [tradeTimer, setTradeTimer] = useState<number | null>(null);

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
                const response = await fetch('/api/resultsTemplate', {
                    method: 'GET'
                });

                if (response.ok) {
                    const data = await response.json();
                    setComm(data.percent);
                    setFee(data.flat);
                    setTaxes(data.taxes);
                } else {
                    setComm(-1);
                    setFee(-1);
                    setTaxes(-1);
                }
            } catch (error) {
                setComm(-1);
                setFee(-1);
                setTaxes(-1);
            }
        };

        fetchMetrics();
    }, []);

    function handleTimer(index : number) {
      if (index < 3) {
        if (timeTimer != null) {
          clearInterval(timeTimer);
          setTimeTimer(null);
        } else {
          setTimeTimer(setInterval(() => setTimeIndex(timeIndex => {
            timeIndex++;
            timeIndex = timeIndex % 3;
            return timeIndex;
          }), 2000));
        }
      }
      else {
        if (tradeTimer != null) {
          clearInterval(tradeTimer);
          setTradeTimer(null);
        } else {
          setTradeTimer(setInterval(() => setTradeIndex(tradeIndex => {
            tradeIndex++;
            tradeIndex = (tradeIndex % 3) + 3;
            return tradeIndex;
          }), 2000));
        }
      }
    }

    function pauseTimer(index : number) {
        if (index < 3) {
            if (timeTimer != null) {
                clearInterval(timeTimer);
                setTimeTimer(null);
            }
        }
        else {
            if (tradeTimer != null) {
                clearInterval(tradeTimer);
                setTradeTimer(null);
            }
        }
    }

    function CreateChart({ index } : { index : number}) {
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
            if (index == 0) {
                seriesRef.current = chartRef.current.addSeries(LineSeries);
                seriesRef.current.setData(data);
                setTimeTitle("Equity Curve over Time");

            }
            else if (index == 1) {
                seriesRef.current = chartRef.current.addSeries(LineSeries);
                seriesRef.current.setData(data);
                setTimeTitle("Drawdown Curve");
            }
            else if (index == 2) {
                seriesRef.current = chartRef.current.addSeries(HistogramSeries);
                seriesRef.current.setData(HistData);
                setTimeTitle("Rolling Sharpe");
            }
            else if (index == 3) {
                seriesRef.current = chartRef.current.addSeries(HistogramSeries);
                seriesRef.current.setData(HistData);
                setTradeTitle("Profit and Loss Curve");
            }
            else if (index == 4) {
                seriesRef.current = chartRef.current.addSeries(LineSeries);
                seriesRef.current.setData(data);
                setTradeTitle("Equity Curve over Trade");
            }
            else if (index == 5) {
                seriesRef.current = chartRef.current.addSeries(HistogramSeries);
                seriesRef.current.setData(HistData);
                setTradeTitle("MFE");
            }
            
            chartRef.current.timeScale().fitContent();

            return () => {
                resizeObserver.disconnect();
                chartRef.current?.remove();
            };
        }, []);

        return <div className="chart" ref={containerRef}></div>
    }

  function TableRow({ algo_entry } : {algo_entry : TableEntry}) {
    return(
      <div className='SimResult_row_container'>
        <div className="table-row-content">
          <h5>{algo_entry.timestamp}</h5>
          <h5>{algo_entry.orderType}</h5>
          <h5>{algo_entry.amountOfStock}</h5>
          <h5 style={{color: algo_entry.moneyGained > 0 ? 'rgb(9, 215, 23)' : 'rgb(215, 9, 23)'}}>
            {algo_entry.moneyGained}
          </h5>
          <h5>${algo_entry.totalMoney}</h5>
        </div>
      </div>
    );
  }

  function AlgoTable( {TableEntryList } : { TableEntryList : TableEntry[]}) {
    let tableRows : ReactElement[] = [];
    TableEntryList.forEach((row) => {
        tableRows.push(
            <TableRow algo_entry={row}></TableRow>
        )
    });

    return (
        <>{ tableRows } </>
    );
  }

    return (
        <div className="analytics-parent-container">
            <div className="analytics-container">
                <div className="top-container">
                    <div className="header-container">
                        <h5>Simulation Name</h5>
                        <h5>00 Month 0000</h5>
                    </div>

                    <button className="export">Export</button>
                </div>

                <div className="charts-table-metric-container">
                    <div className="charts-parent-container">
                        <h2 className="chart-title">{timeTitle}</h2>
                        <div className="charts-container">
                            <CreateChart index={timeIndex}></CreateChart>
                        </div>

                        <div className="button-group">
                            <button onClick={() => {
                                setTimeIndex(timeIndex => {
                                    timeIndex++;
                                    timeIndex = timeIndex % 3;
                                    return timeIndex;
                                });
                                handleTimer(timeIndex);
                            }}>
                                <img src={nextArrow} style={{ transform: "rotate(-90deg)" }}></img>
                            </button>

                            <button onClick={() => pauseTimer(timeIndex)}>
                                <img src={pause}></img>
                            </button>

                            <button onClick={() => {
                                setTimeIndex(timeIndex => {
                                    timeIndex--;
                                    if (timeIndex < 0) {
                                        timeIndex = 2;
                                    }
                                    return timeIndex;
                                });
                                handleTimer(timeIndex);
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
                            <AlgoTable TableEntryList={testData}></AlgoTable>
                        </div>
                    </div>

                    <div className="charts-parent-container">
                        <h2 className="chart-title">{tradeTitle}</h2>
                        <div className="charts-container">
                            <CreateChart index={tradeIndex}></CreateChart>
                        </div>

                        <div className="button-group">
                            <button onClick={() => {
                                setTradeIndex(tradeIndex => {
                                    tradeIndex++;
                                    tradeIndex = (timeIndex % 3) + 3;
                                    return tradeIndex;
                                });
                                handleTimer(tradeIndex);
                            }}>
                                <img src={nextArrow} style={{ transform: "rotate(-90deg)" }}></img>
                            </button>

                            <button onClick={() => pauseTimer(tradeIndex)}>
                                <img src={pause}></img>
                            </button>

                            <button onClick={() => {
                                setTradeIndex(tradeIndex => {
                                    tradeIndex--;
                                    if (tradeIndex < 3) {
                                        tradeIndex = 5;
                                    }
                                    return tradeIndex;
                                });
                                handleTimer(tradeIndex);
                            }}>
                                <img src={nextArrow} style={{ transform: "rotate(90deg)" }}></img>
                            </button>
                        </div>
                    </div>

                    <div className="metric-grid">
                        <div className="metric-container">
                            <p className="metric-title">Percent Commision</p>
                            <p className="metric">{comm === -1 ? "N/A" : `${comm}%`}</p>
                        </div>

                        <div className="metric-container">
                            <p className="metric-title">Flat Fee</p>
                            <p className="metric">{fee === -1 ? "N/A" : `-$${fee}`}</p>
                        </div>

                        <div className="metric-container">
                            <p className="metric-title">Taxes</p>
                            <p className="metric">{taxes === -1 ? "N/A" : `-$${taxes}`}</p>
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

            
        </div>
    );
}
