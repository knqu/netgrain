import './App.css'

import { BrowserRouter as Router, Route, Routes } from 'react-router-dom';
import Home from './components/Home';
import Account from './components/Account';
import Leaderboard from './components/Leaderboard';
import Simulation from './components/Simulation';
import AlgoTable from './components/AlgoTable';
import Login from './components/Login';
import Registration from './components/Registration';
import ChartComponent from './components/Chart';
import SimResults from './components/SimResults';

function App() {
  return (
    <Router>
      <Routes>
        <Route path="/" element={<Login />} />
        <Route path='/registration' element={<Registration />} />
        <Route path="/account" element={<Account />} />
        <Route path="/home" element={<Home />} />
        <Route path="/leaderboard" element={<Leaderboard />} />
        <Route path="/sim" element={<Simulation />} />
        <Route path="/simResults" element={<SimResults />} />
        <Route path="/chart" element={<ChartComponent />} />
        <Route path="/algoTable" element={<AlgoTable />} />
      </Routes>
    </Router>
  );
}

export default App