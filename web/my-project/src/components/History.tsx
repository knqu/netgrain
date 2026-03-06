import { useNavigate } from "react-router-dom";

async function fetchSim() {
  const navigate = useNavigate();

  try {
    const response =
      await fetch(
        "http://localhost:18080/api/fetchSim",
        {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: "{submitted_simID : simID}"
        }
      );

    if (response.status == 200) {
      navigate("/simResults");
    }
  } catch (err) {
    console.log(err);
  }
}

function HistoryComponent() {
  return (
    <div>
      <h1>History Page</h1>
      <button onClick={async () => { await fetchSim() }}>Past Simulation</button>
    </div>
  );
}

export default HistoryComponent;