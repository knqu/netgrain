import { useState, useEffect } from 'react';
import '../styling/SimResults.css';

export default function SimResults() {
  const [feeData, setFeeData] = useState("");

  useEffect(() => {
    const calculate = async () => {
      try {
        const response = await fetch('/api/calculateFee');

        if (response.ok) {
          const savedData = await response.text();
          console.log("Test:" + savedData);

          if (savedData && savedData.trim().length > 0) {
            setFeeData(savedData);
            console.log(feeData);
          }
          else {
            setFeeData("Fee Data Unavailable.");
          }
        }
      } catch (err) {
        console.error("Failed to load layout from server", err);
        setFeeData("Fee Data Unavailable.");
      }
    };

    calculate();
  }, []);

  return (
    <h5>{feeData}</h5>
  );
}
