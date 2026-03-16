import { useNavigate } from "react-router-dom";

function Simulation() {
  const navigate = useNavigate();

  async function saveSim() {
  
  

  try {
    const response = await fetch(
      "http://localhost:18080/api/saveSim",
      {
        method: "GET",
      }
    );

    if (response.status == 200) {
      navigate("/simResults"); 
    }
  } catch (error) {
    console.log(error);
  }
}

  return (
    <div>
      <h1>Simulation Page</h1>
      <button onClick={async () => {saveSim()}}>Finish Simulation</button>
    </div>
  );
}

export default Simulation;

