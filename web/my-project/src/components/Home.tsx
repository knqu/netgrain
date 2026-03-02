import { useState } from 'react';

import LeaderboardComponent from './Leaderboard';
import '../styling/Home.css'

export default function AppHome() {
  const [currentPage, setPage] = useState<string>("Dashboard");

  function DashboardPage() {
    return (
      <div className="Grid_and_Leaderboard">
        <GridComponent></GridComponent>
        <LeaderboardComponent></LeaderboardComponent>
      </div>
    );
  }

  function HistoryPage() {
    return (
      <h1>History</h1>
    );
  }

  function SimulationPage() {
    return (
      <h1>Simulation</h1>
    );
  }

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
    const isActive = button === currentPage;
    console.log(button + " is " + isActive);
    return (
      <button
        className={`${button}Button navButtons ${isActive ? 'active' : ''}`}
        onClick={() => handleDashboardClick(button)}
      >
      {button}
      </button>
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
    if (currentPage === "Dashboard") {
      return <DashboardPage />;
    }
    else if (currentPage === "History") {
      return <HistoryPage />;
    }
    else if (currentPage === "Simulation") {
      return <SimulationPage />;
    }
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

      {renderPage()}
    </div>
    
  );
};

