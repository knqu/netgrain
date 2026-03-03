import React, { useState } from 'react';
import '../styling/Simulation.css';

export interface SimulationConfigRequest {
    initial_capital: number;
    tickers: string[];
    start_date: string;
    end_date: string;
    trade_fee: number;
}

const Simulation: React.FC = () => {
    // --- SIMULATION STATE ---
    const [initialCapital, setInitialCapital] = useState<number | string>(100000);
    const [tickersInput, setTickersInput] = useState<string>("AAPL, MSFT");
    const [startDate, setStartDate] = useState<string>("");
    const [endDate, setEndDate] = useState<string>("");
    const [tradeFee, setTradeFee] = useState<number | string>(1.50);
    
    const [isRunning, setIsRunning] = useState<boolean>(false);
    const [engineStatus, setEngineStatus] = useState<string>("");

    // --- UPLOAD STATE ---
    const [uploadFile, setUploadFile] = useState<File | null>(null);
    const [uploadStatus, setUploadStatus] = useState<string>("");

    // --- FILE UPLOAD HANDLERS ---
    const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        if (e.target.files && e.target.files.length > 0) {
            const file = e.target.files[0];
            //only CSV/TXT
            if (!file.name.endsWith('.csv') && !file.name.endsWith('.txt')) {
                setUploadStatus("Error: All files must be .csv or .txt.");
                setUploadFile(null);
                e.target.value = ""; // clear the input
                return;
            }
            setUploadFile(file);
            setUploadStatus(""); // clear old status
        }
    };

    const handleUpload = async () => {
        if (!uploadFile) {
            setUploadStatus("⚠️ Please select a file first.");
            return;
        }

        setUploadStatus("Uploading...");

        try {
            const response = await fetch("http://localhost:8080/api/upload", {
                method: "POST",
                headers: {
                    // C++ expects this header exactly as written in your main.cpp
                    "x-file-name": uploadFile.name 
                },
                body: uploadFile // Send the raw binary file directly
            });

            if (response.ok) {
                const ticker = await response.text();
                setUploadStatus(`Success! ${ticker} loaded into the engine.`);
                
                // Quality of life: automatically add it to the tickers input if it's not there!
                if (!tickersInput.includes(ticker)) {
                    setTickersInput(prev => prev ? `${prev}, ${ticker}` : ticker);
                }
            } else if (response.status === 409) {
                setUploadStatus(`File already exists in the backend.`);
            } else {
                setUploadStatus(`Upload failed: ${response.statusText}`);
            }
        } catch (error) {
            console.error("Upload Error:", error);
            setUploadStatus("Network error. Is the C++ server running?");
        }
    };

    // --- SIMULATION HANDLER ---
    const handleStart = async () => {
        if (Number(initialCapital) <= 0) {
            alert("Initial capital must be greater than 0!");
            return;
        }

        const parsedTickers = tickersInput
            .split(',')
            .map(t => t.trim().toUpperCase())
            .filter(t => t.length > 0);

        if (parsedTickers.length === 0) {
            alert("Please enter at least one ticker.");
            return;
        }

        const payload: SimulationConfigRequest = {
            initial_capital: Number(initialCapital),
            tickers: parsedTickers,
            start_date: startDate,
            end_date: endDate,
            trade_fee: Number(tradeFee)
        };

        setIsRunning(true);
        setEngineStatus("Sending configuration to engine...");

        try {
            const response = await fetch("http://localhost:8080/api/simulate", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload)
            });

            if (response.ok) {
                const result = await response.text(); 
                setEngineStatus(`Simulation Complete!\n\n${result}`);
            } else {
                setEngineStatus(`Engine Error: ${response.statusText}`);
            }
        } catch (error) {
            console.error("Network Error:", error);
            setEngineStatus("Failed to connect to the C++ Engine. Is it running?");
        }
    };

    // --- UI RENDER ---
    return (
        <div className="sim-container">
            
            {!isRunning ? (
                <>
                    {/* --- NEW: FILE UPLOAD PANEL --- */}
                    <div className="sim-panel" style={{ marginBottom: '20px' }}>
                        <h2 style={{ fontSize: '1.2rem', marginBottom: '15px' }}>Upload Historical Data</h2>
                        
                        <div className="sim-form-group" style={{ display: 'flex', gap: '10px' }}>
                            <input 
                                type="file" 
                                className="sim-input" 
                                accept=".csv,.txt"
                                onChange={handleFileChange}
                            />
                            <button 
                                className="sim-btn-secondary" 
                                style={{ marginTop: '0', whiteSpace: 'nowrap' }}
                                onClick={handleUpload}
                                disabled={!uploadFile}
                            >
                                Upload
                            </button>
                        </div>
                        
                        {uploadStatus && (
                            <div style={{ fontSize: '14px', marginTop: '10px', color: uploadStatus.includes('O') ? 'green' : (uploadStatus.includes('X') ? 'red' : '#555') }}>
                                <strong>{uploadStatus}</strong>
                            </div>
                        )}
                    </div>

                    {/* --- EXISTING: SIMULATION SETTINGS PANEL --- */}
                    <div className="sim-panel">
                        <h2>Simulation Settings</h2>
                        
                        <div className="sim-form-group">
                            <label className="sim-label">Initial Capital ($)</label>
                            <input 
                                type="number" 
                                className="sim-input"
                                value={initialCapital} 
                                onChange={(e) => setInitialCapital(e.target.value)}
                            />
                        </div>

                        <div className="sim-form-group">
                            <label className="sim-label">Tickers (comma separated)</label>
                            <input 
                                type="text" 
                                className="sim-input"
                                value={tickersInput} 
                                onChange={(e) => setTickersInput(e.target.value)}
                                placeholder="e.g. AAPL, MSFT"
                            />
                        </div>

                        <div className="sim-form-row">
                            <div className="sim-form-col">
                                <label className="sim-label">Start Date</label>
                                <input 
                                    type="date" 
                                    className="sim-input"
                                    value={startDate} 
                                    onChange={(e) => setStartDate(e.target.value)}
                                />
                            </div>
                            <div className="sim-form-col">
                                <label className="sim-label">End Date</label>
                                <input 
                                    type="date" 
                                    className="sim-input"
                                    value={endDate} 
                                    onChange={(e) => setEndDate(e.target.value)}
                                />
                            </div>
                        </div>

                        <div className="sim-form-group">
                            <label className="sim-label">Trading Fee ($ per trade)</label>
                            <input 
                                type="number" 
                                step="0.01"
                                className="sim-input"
                                value={tradeFee} 
                                onChange={(e) => setTradeFee(e.target.value)}
                            />
                        </div>

                        <button 
                            className="sim-btn-primary"
                            onClick={handleStart}
                        >
                            Run Backtest
                        </button>
                    </div>
                </>
            ) : (
                <div className="sim-panel">
                    <h2>Engine Status</h2>
                    <pre className="sim-status-box">
                        {engineStatus}
                    </pre>
                    <button 
                        className="sim-btn-secondary"
                        onClick={() => setIsRunning(false)}
                    >
                        ← Back to Settings
                    </button>
                </div>
            )}

        </div>
    );
};

export default Simulation;