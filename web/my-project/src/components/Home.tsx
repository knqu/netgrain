import React from 'react';
import { useState } from 'react';

import LeaderboardComponent from './Leaderboard';
import '../styling/Home.css'

export default function AppHome() {
  const [currentPage, setPage] = useState<string>("Dashboard");

  function Dashboard() {
    return (
      <div className="DashboardContainter">
        <div className="DashboardButtonContainer">
          <DashboardButton button="Dashboard"></DashboardButton>
          <DashboardButton button="History"></DashboardButton>
          <DashboardButton button="Simulation"></DashboardButton>
        </div>
      </div>
    );
  }

  function DashboardButton({ button } : {button : string}) {
  return (
    <button
      className={`${button}Button`}
      onClick={() => handleDashboardClick(button)}>
    {button}
    </button>
  );
}

  function handleDashboardClick(desiredPage : string) {
    if (desiredPage === currentPage) {
      return;
    }
    setPage(desiredPage);
  }

  function GridComponent() {
    return (
      // write the code here
      <div className="Grid">
      </div>
    );
  }

  return (
    <div className="homepageContainer">
      <Dashboard></Dashboard>
      
      <div className="Grid_and_Leaderboard">
        <GridComponent></GridComponent>
        <LeaderboardComponent></LeaderboardComponent>
      </div>
    </div>
  );
};

