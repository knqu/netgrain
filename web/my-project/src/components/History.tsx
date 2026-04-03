//import { useNavigate } from "react-router-dom";
import "../styling/History.css"
import type { ReactElement } from "react";
import arrow from "../assets/arrow.png"

export default function HistoryComponent() {
  type HistoryTableEntry = {
    "SimName": string;
    "Date" : string;
    "Duration" : string;
    "Profit" : string;
  }

  const testData : HistoryTableEntry[] = [
    {SimName : "Daniel's Sim", Date : "29 March 2026", Duration : "5 seconds", Profit : "+1"}
  ];


  /* Temporarily uncommented for compiling npm run build
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
  */

  function HistoryTableEntryCollapsed({ TableEntry } : {TableEntry : HistoryTableEntry}) {
    return (
      <div className="history-table-entry-container">
        <div className="history-table-contents">
          <h3>{TableEntry.SimName}</h3>
          <h3>{TableEntry.Date}</h3>
          <h3>{TableEntry.Duration}</h3>
          <h3 style={{color : TableEntry.Profit.at(0) == '+' ? 'rgb(9, 215, 23)' : 'rgb(215, 9, 23)'}}>
            {TableEntry.Profit}
          </h3>
        </div>

        <img src={arrow} style={{transform : "rotate(180deg)"}}></img>
      </div>
    );
  }

  function HistoryTable({TableEntryList} : {TableEntryList : HistoryTableEntry[]}) {
    let tableRows : ReactElement[] = [];
    TableEntryList.forEach((row) => {
      tableRows.push(
        <HistoryTableEntryCollapsed TableEntry={row} />
      );
    });

    return (
      <div className="history-parent-container">
        <div className="history-table-header-container">
          <div className="history-table-header">
            <h3>Simulation Name</h3>
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

  return (
    <HistoryTable TableEntryList={testData}></HistoryTable>
  );
}
