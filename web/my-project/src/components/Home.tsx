import { useState } from 'react';

import GridComponent from './GridComponent';
import LeaderboardComponent from './Leaderboard';
import HistoryComponent from './History';
import Simulation from './Simulation';

import '../styling/Home.css'

import { Responsive, useContainerWidth } from 'react-grid-layout';
import '/node_modules/react-grid-layout/css/styles.css';
import '/node_modules/react-resizable/css/styles.css';

export default function AppHome() {
  const [currentPage, setPage] = useState<string>("Dashboard");
  const [widgets, setWidgets] = useState<number[]>([1,2,3]); // initial set of widgets

  function playDailyMarket() {
    setPage("Simulation");
    // send config file and special reqeust to play daily market with the file
  }

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

  function DailyMarketComponenet() {
    return (
        <div className='dailyMarketContainer'>
            <h1 className='dailyMarketTitle'>Daily Market</h1>
            
            <div className='dailyMarketButtonsContainer'>
                <input className="dailyMarketButtons" id="dailyMarketFileUploadButton" type="file" accept=".py, .tar, .yml" name="dailyMarketFiles" multiple/>
                <button className="dailyMarketButtons" id="dailyMarketPlayButton" onClick={() => {setPage("Simulation")}}>Play</button>
            </div>
        </div>
    );
}

  function GridComponent() {
    const { width, containerRef, mounted } = useContainerWidth();
      const [widgets, setWidgets] = useState<number[]>([1,2,3]); // initial set of widgets

    const layoutArray = widgets.map((id, index) => ({
      i: id.toString(),
      x: (index % 3) * 4,
      y: Math.floor(index / 3) * 2,
      w: 4,
      h: 2,
    }));

    const responsiveLayouts = {
      lg: layoutArray,
      md: layoutArray,
    };

    return (
      <div ref={containerRef} className="Grid">
      {mounted && (
        <Responsive
        layouts={responsiveLayouts}
        breakpoints={{ lg: 1100, md: 996, sm: 768, xs: 480, xxs: 0 }}
        cols={{ lg: 12, md: 10, sm: 6, xs: 4, xxs: 2 }}
        width={width}
        rowHeight={100}
        >
        {widgets.map((id) => (
          <div 
          key={id.toString()} 
          className="relative group bg-slate-600 rounded-lg shadow-xl flex items-center justify-center text-white font-bold border border-white/5"
          >
          <div className="drag-handle absolute top-2 left-2 cursor-grab opacity-50 group-hover:opacity-100 p-1">
          ⠿
          </div>

          <button
          className="absolute top-2 right-2 text-xs p-1 bg-red-500 rounded opacity-0 group-hover:opacity-100 transition-opacity z-20"
          onMouseDown={(e) => e.stopPropagation()} 
          onClick={() => removeWidget(id)}
          >
          ✕
          </button>

          WIDGET {id}
          </div>

        ))}
        </Responsive>
      )}
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
            <GridComponent></GridComponent>            
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
    }
  }

  return (
    <div className="homepageContainer">
      <Dashboard onAdd={addWidget}></Dashboard>

      {renderPage()}
    </div>
  );
};
