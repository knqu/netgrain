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

  const [flatFee, percentageFee, shortTermTax] = (feeData === "Fee Data Unavailable.") ? ["Fee Data Unavailable", "Fee Data Unavailable", "Fee Data Unavailable"] : (feeData || "").split(",");

  return (
    <div>
    <h5>Flat Commission Fee: {flatFee}</h5>
    <h5>Percentage-based Fee: {percentageFee}</h5>
    <h5>Short-term taxes: {shortTermTax}</h5>
    </div>
  );
}
