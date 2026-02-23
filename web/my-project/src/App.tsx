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
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<Home />} />
        <Route path="/account" element={<Account />} />
        <Route path="/leaderboard" element={<Leaderboard />} />
        <Route path="/sim" element={<Simulation />} />
        <Route path="/sim-results" element={<SimulationResults />} />
        <Route path="/login" element={<Login />} />
        <Route path='/registration' element={<Registration />} />
      </Routes>
    </BrowserRouter>
  );
}

export default App
