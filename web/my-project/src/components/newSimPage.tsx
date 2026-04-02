import { useState } from 'react';

import "../styling/newSim.css"


import Simulation from "./Simulation"
import CodeEditor from "./CodeEditor"
import SimResults from "./SimResults";


const NewSim: React.FC = () => {
    const [currentPage, setPage] = useState<string>("Sim");
    const startSim = async () => {
        setPage("Start");
    };    
    

    function renderPage() {
        switch (currentPage) {
            case "Sim":
                return (
                    <div className="outer-sim">
                        <div className="action-board">
                        <button className="startSim"
                        onClick={startSim}>
                        Start Simulation
                        </button>
                    </div>
                    <div className="sim-grid">
                        <div className="grid-item">
                            <CodeEditor />
                        </div>
                        <div className="grid-item">
                            <Simulation />
                        </div>
                    </div>
            
                    </div>
                );
                case "Start":
                    return (<SimResults />);
        }
    }
    return (
        <>
            {renderPage()}
        </>
        
            
            
    );
};

export default NewSim;
