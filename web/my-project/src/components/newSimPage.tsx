import "../styling/newSim.css"
import Simulation from "./Simulation"
import CodeEditor from "./CodeEditor"




export default function NewSim() {
    return (
        <div className="outer-sim">
            <div className="grid-item">
                <CodeEditor />
            </div>
            <div className="grid-item">
                <Simulation />
            </div>
        </div>
        
        
    )};