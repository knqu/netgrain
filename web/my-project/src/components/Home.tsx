import { useState } from 'react';

import GridComponent from './GridComponent';
import LeaderboardComponent from './Leaderboard';
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

  function handleDashboardClick(desiredPage : string) {
    console.log(desiredPage);
    if (desiredPage === currentPage) {
      return;
    }
    setPage(desiredPage);
  }

  return (
    <div className="homepageContainer">
    <Dashboard onAdd={addWidget}></Dashboard>

    <div className="Grid_and_Leaderboard">
    <GridComponent widgets={widgets} removeWidget={removeWidget}></GridComponent>
    <LeaderboardComponent></LeaderboardComponent>
    </div>
    </div>
    
  );
};
