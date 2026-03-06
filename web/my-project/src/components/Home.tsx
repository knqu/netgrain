import { useState } from 'react';

import LeaderboardComponent from './Leaderboard';
import HistoryComponent from './History';
import Simulation from './Simulation';
import Generation from './Generation';
import GridComponent from './GridComponent';

import '../styling/Home.css'

import { Responsive, useContainerWidth } from 'react-grid-layout';
import '/node_modules/react-grid-layout/css/styles.css';
import '/node_modules/react-resizable/css/styles.css';

export default function AppHome() {
  const [currentPage, setPage] = useState<string>("Dashboard");
  const [widgets, setWidgets] = useState<number[]>([1,2,3]); // initial set of widgets

  const addWidget = () => { // add another widget
    setWidgets(prev => [...prev, prev.length > 0 ? Math.max(...prev) + 1 : 1]);
  };

  const removeWidget = (idToRemove: number) => { // delete specific widget
    setWidgets((prev) => prev.filter((id) => id !== idToRemove));
  };

  function Dashboard({ onAdd }: { onAdd : () => void}) {
    return (
      <div className="DashboardContainter">
      <div className="DashboardButtonContainer">
      <DashboardButton button="Dashboard"></DashboardButton>
      <DashboardButton button="History"></DashboardButton>
      <DashboardButton button="Simulation"></DashboardButton>
      <DashboardButton button="Generation"></DashboardButton>
      <DashboardButton button="Add Widget" onClickOverride={onAdd}></DashboardButton>
      </div>
      </div>
    );
  }

  function DashboardButton({ button, onClickOverride }: { button: string, onClickOverride?: () => void }) {
    return (
      <button
      className={`${button.replace(/\s+/g, '')}Button`}
      onClick={() => {
        if (onClickOverride) {
          onClickOverride();
        } else {
          handleDashboardClick(button);
        }
      }}>
      {button}
      </button>
    );
  }

  async function playDailyMarket() {
    setPage("Simulation");
    /*const response = fetch(
      "http://localhost:18080/api/attemptDaily",
      {
        method: "POST",
        headers : {"Content-Type" : "application/json"},
        body : JSON.stringify({})
      }
    )*/
  }

  function DailyMarketComponenet() {
    return (
        <div className='dailyMarketContainer'>
            <h1 className='dailyMarketTitle'>Daily Market</h1>
            
            <div className='dailyMarketButtonsContainer'>
                <input className="dailyMarketButtons" id="dailyMarketFileUploadButton" type="file" accept=".py, .tar, .yml" name="dailyMarketFiles" multiple/>
                <button className="dailyMarketButtons" id="dailyMarketPlayButton" onClick={() => {playDailyMarket()}}>Play</button>
            </div>
        </div>
    );
}
  function handleDashboardClick(desiredPage : string) {
    console.log(desiredPage);
    if (desiredPage === currentPage) {
      return;
    }
    setPage(desiredPage);
  }

  function renderPage() {
    switch (currentPage) {
      case "Dashboard":
        return (
          <div className="Grid_and_Leaderboard">
            <GridComponent widgets={widgets} removeWidget={removeWidget}></GridComponent>
            <div className="Leaderboard_and_DailyMarket">
              <DailyMarketComponenet></DailyMarketComponenet>
              <LeaderboardComponent></LeaderboardComponent>
            </div>
          </div>
        );
      case "History" :
        return (
          <HistoryComponent />
        );
      case "Simulation" :
        return (
          <Simulation />
        );
      case "Generation" :
        return (
          <Generation />
        );
    }
  }

  return (
    <div className="homepageContainer">
      <Dashboard onAdd={addWidget}></Dashboard>

      {renderPage()}
    </div>
  );
};
