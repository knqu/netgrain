import { type ReactElement, useState, useEffect } from 'react';
import '../styling/AlgoTable.css'

type SimResults_row_entry = {
    timestamp : number;
    orderType : string;
    amountOfStock: number;
    moneyGained: number;
    totalMoney: number;
  }
  
  function SimResults_Row({ algo_entry } : {algo_entry : SimResults_row_entry}) {
    return(
      <div className='SimResult_row_container'>
        <div className="SimResult_row_content">

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
  
  function SimResults_table({ algo_entry_list } : { algo_entry_list : SimResults_row_entry[] }) {
    let tableRows : ReactElement[] = [];
    /* for (let i = 1; i < algo_entry_list.length; i++) {
      let key = algo_entry_list[i];
      let j = i - 1;

      while (j >= 0 && algo_entry_list[j] > key) {
        algo_entry_list[j + 1] = algo_entry_list[j];
        j = j - 1;
      }
    } */
    algo_entry_list.forEach((row) => {
      tableRows.push(
        <SimResults_Row algo_entry={row} key={row.timestamp}/>
      );
    });
  
    return (
      <div className="SimResult_table_container">
        <div className="SimResult_table_head_row_container">
          <div className="SimResult_table_head_row">
            <h5>Time</h5>
            <h5>Type</h5>
            <h5># of Stocks</h5>
            <h5>±Money</h5>
            <h5>Total Money</h5>
          </div>
        </div>
      
        <div className="SimResult_table">
          <table >
            <tbody>{tableRows}</tbody>
          </table>
        </div>
      </div>
    );
  }
  
  
  function SimResults({entries} : {entries : SimResults_row_entry[]}) {
    return (
      <div className='SimResultsContainer'>
        <div className="SimResults">
          <SimResults_table algo_entry_list={entries}></SimResults_table>
        </div>
      </div>
    );
  }

export default function SimulationResultsComponent() {
  return (
    <SimResults entries={testData}/>
  );
};

const testData : SimResults_row_entry[] = [
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
];