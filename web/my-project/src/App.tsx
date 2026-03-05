import './App.css'

import { BrowserRouter as Router, Route, Routes } from 'react-router-dom';
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
      <Routes>
        <Route path="/" element={<Login />} />
        <Route path="/account" element={<Account />} />
        <Route path="/leaderboard" element={<Leaderboard />} />
        <Route path="/sim" element={<Simulation />} />
        <Route path="/sim-results" element={<SimulationResults />} />
        <Route path="/home" element={<Home />} />
        <Route path='/registration' element={<Registration />} />
      </Routes>
    </Router>
  );
}

export default App