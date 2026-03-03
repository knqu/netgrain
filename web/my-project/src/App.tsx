import { useState } from 'react'
import reactLogo from './assets/react.svg'
import viteLogo from '/vite.svg'
import './App.css'

import { BrowserRouter as Router, Route, Routes, Link, BrowserRouter } from 'react-router-dom';
import Home from './components/Home';
import Account from './components/Account';
import Leaderboard from './components/Leaderboard';
import Simulation from './components/Simulation';
import SimulationResults from './components/SimulationResults';
import Login from './components/Login';
import Registration from './components/Registration';

function App() {
  return (
    <Router>
      <nav className="fixed top-0 left-0 right-0">
        <ul className="list-none">
          <li className="float-left p-5"><Link to="/">Home</Link></li>
          <li className="float-left p-5"><Link to="/account">Account</Link></li>
          <li className="float-left p-5"><Link to="/leaderboard">Leaderboard</Link></li>
          <li className="float-left p-5"><Link to="/sim">Simulation/Generate</Link></li>
          <li className="float-left p-5"><Link to="/sim-results">Simulation Results</Link></li>
        </ul>
      </nav>

      <Routes>
        <Route path="/" element={<Home />} />
        <Route path="/account" element={<Account />} />
        <Route path="/leaderboard" element={<Leaderboard />} />
        <Route path="/sim" element={<Simulation />} />
        <Route path="/sim-results" element={<SimulationResults />} />
        <Route path="/login" element={<Login />} />
        <Route path='/registration' element={<Registration />} />
      </Routes>
    </Router>
  );
}

export default App
