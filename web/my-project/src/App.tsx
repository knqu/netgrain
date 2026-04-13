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
import ChartSelect from './components/ChartSelect';
import SimResults from './components/SimResults';
import Verification from './components/Verification';
import ForgotPassword from './components/ForgotPassword';
import CodeEditor from './components/CodeEditor';
//import NewSim from './components/newSimPage'
import SimulationRun from './components/SimulationRun';
import LiveChartComponent from './components/LiveChart';
import ResultsTemplate from './components/ResultsTemplate';

function App() {
  return (
    <Router>
      <Routes>
        <Route path="/login" element={<Login />} />
        <Route path='/registration' element={<Registration />} />
        <Route path="/account" element={<Account />} />
        <Route path="/home" element={<Home />} />
        <Route path="/leaderboard" element={<Leaderboard />} />
        <Route path="/sim" element={<Simulation />} />
        <Route path="/simResults" element={<SimResults />} />
        <Route path="/chart" element={<ChartComponent />} />
        <Route path="/chartSelect" element={<ChartSelect />} />
        <Route path="/algoTable" element={<AlgoTable />} />
        <Route path="/verification" element={<Verification />} />
        <Route path="/forgot" element={<ForgotPassword />} />
        <Route path="/codeEditor" element={<CodeEditor />} />
        <Route path="*" element={<Login />} />
        <Route path="/liveChart" element={<LiveChartComponent />} />
        <Route path="/simRun" element={<SimulationRun />} />
        <Route path="/resultsTemplate" element={<ResultsTemplate />} />
      </Routes>
    </Router>
  );
}

export default App
