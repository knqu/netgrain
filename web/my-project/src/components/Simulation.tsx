import { useNavigate } from "react-router-dom";

async function saveSim() {
  const navigate = useNavigate();

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

function Simulation() {
  return (
    <div>
      <h1>Simulation Page</h1>
      <button onClick={async () => {await saveSim()}}>Finish Simulation</button>
    </div>
  );
}

export default Simulation;

