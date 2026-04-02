import { useNavigate } from "react-router-dom";
import { useState, useRef, useEffect, type ReactElement } from 'react';

import { LineSeries, createChart, type LineData, type HistogramData, type IChartApi, HistogramSeries } from 'lightweight-charts';

import "../styling/History.css";
import arrow from "../assets/arrow.png";
import expand from "../assets/expand.png";
import nextArrow from "../assets/next.png";
import pause from "../assets/pause.png";

export default function HistoryComponent() {
  type HistoryTableEntry = {
    "SimName": string;
    "Date": string;
    "Duration": string;
    "Profit": string;
    "ID"?: string;
  }

  const testData: HistoryTableEntry[] = [
    { SimName: "Daniel's Sim", Date: "29 March 2026", Duration: "5 seconds", Profit: "+1" },
    { SimName: "Haiyan's Sim", Date: "30 March 2026", Duration: "15 seconds", Profit: "-1049" },
    { SimName: "Kevin's Sim", Date: "31 March 2026", Duration: "115 seconds", Profit: "0" },
    { SimName: "Colin's Sim", Date: "1 April 2026", Duration: "995 seconds", Profit: "+34" },
  ];

  const testLineData: LineData<number>[] = [
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

  const testDrawDownData: LineData<number>[] = [
    { time: 1, value: 0 },
    { time: 2, value: -2 },
    { time: 3, value: -1.5 },
    { time: 4, value: -3 },
    { time: 5, value: 0 },
    { time: 6, value: -1 },
    { time: 7, value: -4 },
    { time: 8, value: -2.5 },
    { time: 9, value: -1 },
    { time: 10, value: 0 },
    { time: 11, value: -0.5 },
    { time: 12, value: -3.5 },
    { time: 13, value: -2 },
    { time: 14, value: -1 },
    { time: 15, value: 0 },
  ]

  const testPLData: HistogramData<number>[] = [
    { time: 1, value: 50, color: "green" },
    { time: 2, value: 30, color: "green" },
    { time: 3, value: 60, color: "green" },
    { time: 4, value: 50, color: "green" },
    { time: 5, value: 90, color: "green" },
    { time: 6, value: 85, color: "green" },
    { time: 7, value: 145, color: "green" },
    { time: 8, value: 130, color: "green" },
    { time: 9, value: 155, color: "green" },
    { time: 10, value: 125, color: "green" },
    { time: 11, value: 160, color: "green" },
    { time: 12, value: 150, color: "green" },
    { time: 13, value: 170, color: "green" },
    { time: 14, value: 165, color: "green" },
    { time: 15, value: 205, color: "green" },
  ];

  const testIndividualPLData: HistogramData<number>[] = [
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

  const navigate = useNavigate();
  async function fetchSim() {
    try {
      const response =
        await fetch(
          "http://localhost:18080/api/fetchSim",
          {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: "{submitted_simID : simID}"
          }
        );

      if (response.status == 200) {
        navigate("/simResults");
      }
    } catch (err) {
      console.log(err);
    }
  }

  function DisplayLineChart({ chartIndex }: { chartIndex: boolean }) {
    const containerRef = useRef<HTMLDivElement | null>(null);
    const chartRef = useRef<IChartApi | null>(null);
    const lineRef = useRef<any | null>(null);

    useEffect(() => {
      if (!containerRef.current) return;

      chartRef.current = createChart(containerRef.current, {
        width: 520,
        height: 300,
        layout: {
          textColor: "black",
          background: { color: 'white' }
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
      lineRef.current = chartRef.current.addSeries(LineSeries);

      return () => {
        chartRef.current?.remove();
      };
    }, []);

    useEffect(() => {
      if (!chartRef.current) return;

      const data = chartIndex == true ? testLineData : testDrawDownData

      lineRef.current.setData(data);
      chartRef.current.timeScale().fitContent;
    }, [chartIndex]);

    return (
      <div className="lineChart" ref={containerRef}></div>
    );
  }


  function DisplayTradeChart({ chartIndex }: { chartIndex: boolean }) {
    const containerRef = useRef<HTMLDivElement | null>(null);
    const chartRef = useRef<IChartApi | null>(null);
    const histRef = useRef<any | null>(null);

    useEffect(() => {
      if (!containerRef.current) return;

      chartRef.current = createChart(containerRef.current, {
        width: 520,
        height: 300,
        layout: {
          textColor: "black",
          background: { color: 'white' }
        },
        timeScale: {
          timeVisible: true,
          secondsVisible: true,
          tickMarkFormatter: (time: number) => `Trade ${time}`,
        },
        localization: {
          timeFormatter: (time: number) => `Trade ${time}`,
        },
      });

      histRef.current = chartRef.current.addSeries(HistogramSeries, { color: "#26a69a" });

      return () => {
        chartRef.current?.remove();
      };
    }, []);

    useEffect(() => {
      if (!chartRef.current) return;

      const data = chartIndex == true ? testPLData : testIndividualPLData;

      histRef.current.setData(data);
      chartRef.current.timeScale().fitContent;
    }, [chartIndex])

    return (
      <div className="lineChart" ref={containerRef}></div>
    );
  }

  function HistoryTableEntryCollapsed({ TableEntry }: { TableEntry: HistoryTableEntry }) {
    const [collapsed, setCollapsed] = useState<boolean>(true);
    const [lineIndex, setLineIndex] = useState<boolean>(true);
    const [tradeIndex, setTradeIndex] = useState<boolean>(true);
    const [lineTimer, setLineTimer] = useState<number | null>(null);
    const [tradeTimer, setTradeTimer] = useState<number | null>(null);

    function handleCollapse() {
      setCollapsed(!collapsed);
      handleTimer(true, !collapsed);
      handleTimer(false, !collapsed);
    }

    function handleTimer(line: boolean, collapsedArg: boolean) {
      if (collapsedArg == true) {
        if (lineTimer != null) {
          clearInterval(lineTimer);
          setLineTimer(null);
        }
        if (tradeTimer != null) {
          clearInterval(tradeTimer);
          setTradeTimer(null);
        }
        return
      }

      if (line == true) {
        if (lineTimer != null) {
          clearInterval(lineTimer);
          setLineTimer(null);
        } else {
          setLineTimer(setInterval(() => setLineIndex(lineIndex => !lineIndex), 2000));
        }
      } else {
        if (tradeTimer != null) {
          clearInterval(tradeTimer);
          setTradeTimer(null);
        } else {
          setTradeTimer(setInterval(() => setTradeIndex(tradeIndex => !tradeIndex), 2000));
        }
      }
    }

    return (
      <div className="history-table-parent-container">
        <div className="history-table-entry-container">
          <div className="history-table-contents">
            <h3 id="table-entry-sim-name">{TableEntry.SimName}</h3>
            <h3>{TableEntry.Date}</h3>
            <h3>{TableEntry.Duration}</h3>
            <h3 style={{ color: TableEntry.Profit.at(0) == '+' ? 'rgb(9, 215, 23)' : 'rgb(215, 9, 23)' }}>
              {TableEntry.Profit}
            </h3>

            <button className="arrow-button" onClick={handleCollapse}>
              <img id="history-table-arrow" src={arrow} style={{ transform: collapsed ? "rotate(180deg)" : "rotate(0deg)" }}></img>
            </button>
          </div>
        </div>

        <div className="history-table-entry-expanded-container" style={{ display: collapsed ? "none" : "grid" }}>
          <div className="history-table-entry-expanded-linecharts-container">
            <DisplayLineChart chartIndex={lineIndex}></DisplayLineChart>
            <div className="button-group">
              <button onClick={() => {
                setLineIndex(lineIndex => !lineIndex);
                handleTimer(true, collapsed);
              }}>
                <img src={nextArrow} style={{ transform: "rotate(-90deg)" }}></img>
              </button>

              <button onClick={() => handleTimer(true, collapsed)}>
                <img src={pause}></img>
              </button>

              <button onClick={() => {
                setLineIndex(lineIndex => !lineIndex);
                handleTimer(true, collapsed);
              }}>
                <img src={nextArrow} style={{ transform: "rotate(90deg)" }}></img>
              </button>
            </div>
          </div>

          <div className="history-table-entry-expanded-tradecharts-container">
            <DisplayTradeChart chartIndex={tradeIndex}></DisplayTradeChart>
            <div className="button-group">
              <button onClick={() => {
                setTradeIndex(tradeIndex => !tradeIndex);
                handleTimer(false, collapsed);
              }}>
                <img src={nextArrow} style={{ transform: "rotate(-90deg)" }}></img>
              </button>
              <button onClick={() => handleTimer(false, collapsed)}>
                <img src={pause}></img>
              </button>
              <button onClick={() => {
                setTradeIndex(tradeIndex => !tradeIndex);
                handleTimer(false, collapsed);
              }}>
                <img src={nextArrow} style={{ transform: "rotate(90deg)" }} ></img>
              </button>
            </div>
          </div>

          <div className="history-table-entry-expanded-stats-container">
            <div className="history-table-entry-expanded-stats">
              <div className="stat-headers">
                <h3>Return on Capital</h3>
                <h3>Average Trade Return</h3>
                <h3>Win Rate over Time</h3>
              </div>

              <div className="stat-infos">
                <h3>$1</h3>
                <h3>$20</h3>
                <h3>$50</h3>
              </div>
            </div>
          </div>

          <button className="expand-button" onClick={() => fetchSim}>
            <img id="history-table-expand" src={expand}></img>
          </button>
        </div>
      </div>
    );
  }

  function HistoryTable({ TableEntryList }: { TableEntryList: HistoryTableEntry[] }) {
    let tableRows: ReactElement[] = [];
    TableEntryList.forEach((row) => {
      tableRows.push(
        <HistoryTableEntryCollapsed TableEntry={row} />
      );
    });

    return (
      <div className="history-parent-container">
        <div className="history-table-header-container">
          <div className="history-table-header">
            <h3 id="sim-name">Simulation Name</h3>
            <h3>Date</h3>
            <h3>Duration</h3>
            <h3>Profit</h3>
          </div>
        </div>

        <div className="HistoryTable">
          {tableRows}
        </div>
      </div>
    );
  }

  let histEntries: HistoryTableEntry[] = [];
  useEffect(() => {  
    const fetchHist = async () => {
      try {
        const response = await fetch(
          "http://localhost:18080/api/fetchHistory",
          {
            method: "GET",
          }
        );

        if (response.status == 200 && response.body != null) {
          const responseStr = await response.text();
          histEntries = JSON.parse(responseStr);
        }
      } catch (err) {
        console.log(err);
      }
    };
    fetchHist();
  }, []);

  return (
    <HistoryTable TableEntryList={histEntries.length == 0 ? testData : histEntries}></HistoryTable>
  );
}