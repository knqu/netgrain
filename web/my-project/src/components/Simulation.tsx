import React, { useState } from 'react';
import '../styling/Simulation.css';

export interface StockParams {
    ticker: string;
    base_price: number;
    liquidity: number;
    volatility: number;
    market_cap: number;
}

export interface SimulationConfigRequest {
    initial_capital: number;
    stocks: StockParams[];
    start_date: string;
    end_date: string;
    trade_fee: number;
}

const Simulation: React.FC = () => {
    // --- SIMULATION STATE ---
    const [initialCapital, setInitialCapital] = useState<number | string>(100000);

    // (stocks list)
    const [stocks, setStocks] = useState<StockParams[]>([
      { ticker: "", base_price: 0, liquidity: 0, volatility: 0, market_cap: 0},
    ]);

    const addStock = () => {
      setStocks([...stocks, { ticker: "", base_price: 0, liquidity: 0, volatility: 0, market_cap: 0 }]);
    };

    const removeStock = (index: number) => {
      setStocks(stocks.filter((_, i) => i !== index));
    };

    const updateStock = (index: number, field: keyof StockParams, value: string | number) => {      setStocks(currentStocks => {
        const currStocks = [...currentStocks];

        currStocks[index] = { 
          ...currStocks[index], 
          [field]: field === 'ticker' ? value : Number(value) 
        };

        return currStocks; 
      });
    };

    const [startDate, setStartDate] = useState<string>("");
    const [endDate, setEndDate] = useState<string>("");
    const [tradeFee, setTradeFee] = useState<number | string>(1.50);
    
    const [isRunning, setIsRunning] = useState<boolean>(false);
    const [engineStatus, setEngineStatus] = useState<string>("");
    const [runResult, setRunResult] = useState<any>(null);
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
            setUploadStatus("Please select a file first.");
            return;
        }

        setUploadStatus("Uploading...");

        try {
            const response = await fetch("http://localhost:8080/api/upload", {
                method: "POST",
                headers: {
                    "x-file-name": uploadFile.name 
                },
                body: uploadFile
            });

            if (response.ok) {
                const ticker = await response.text();
                setUploadStatus(`Success! ${ticker} loaded into the engine.`);
                
                // UPDATE: Add the uploaded ticker to the new 'stocks' array structure!
                setStocks(currentStocks => {
                    // Check if it's already in the list
                    const alreadyExists = currentStocks.some(s => s.ticker === ticker);
                    if (alreadyExists) return currentStocks;

                    // If the first empty slot is unused, overwrite it
                    if (currentStocks.length === 1 && currentStocks[0].ticker === "") {
                        return [{ ...currentStocks[0], ticker: ticker }];
                    }
                    
                    // Otherwise, add a new row
                    return [...currentStocks, { ticker: ticker, base_price: 0, liquidity: 0, volatility: 0, market_cap: 0 }];
                });

            } else if (response.status === 409) {
                setUploadStatus(`File already exists in the backend.`);
            } else {
                setUploadStatus(`Upload failed: ${response.statusText}`);
            }
        } catch (error) {
            // Good practice: Log the REAL error to the console so it doesn't get hidden!
            console.error("THE REAL UPLOAD ERROR IS:", error); 
            setUploadStatus("Network error. Check F12 Developer Console!");
        }
    };

    // --- SIMULATION HANDLER ---
    const handleStart = async () => {
        if (Number(initialCapital) <= 0) {
            alert("Initial capital must be greater than 0!");
            return;
        }

        const payload: SimulationConfigRequest = {
            initial_capital: Number(initialCapital),
            stocks: stocks,
            start_date: startDate,
            end_date: endDate,
            trade_fee: Number(tradeFee)
        };

        console.log(JSON.stringify(payload))


        setIsRunning(true);
        setEngineStatus("Sending configuration to engine...");

        try {
            const response = await fetch("http://localhost:8080/api/simulate", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload)
            });

            if (response.ok) {
                const resultText = await response.text(); 
                try {
                    const cleanJSON = resultText.replace(/,\s*}/g, '}').replace(/,\s*\]/g, ']');
                    
                    const parsedData = JSON.parse(cleanJSON);
                    
                    setRunResult(parsedData);
                    setEngineStatus("Simulation Complete!");
                } catch (e) {
                    console.error("JSON Parse Error:", e);
                    setEngineStatus(`Simulation Complete! (Raw Output):\n\n${resultText}`);
                }
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

                        {stocks.map((stock, index) => (
                          <div key={index} className="sim-stock-card">
                              <div className="sim-form-row">
                                  <div className="sim-form-col">
                                      <label className="sim-label">Ticker</label>
                                      <input 
                                          type="text" 
                                          className="sim-input"
                                          value={stock.ticker}
                                          onChange={(e) => updateStock(index, 'ticker', e.target.value.toUpperCase())}
                                          placeholder="e.g. AAPL"
                                      />
                                  </div>
                                  <div className="sim-form-col">
                                      <label className="sim-label">Base Price ($)</label>
                                      <input 
                                          type="number" 
                                          min="0"
                                          className="sim-input"
                                          value={stock.base_price}
                                          onChange={(e) => updateStock(index, 'base_price', e.target.value)}
                                      />
                                  </div>
                              </div>

                              <div className="sim-form-row" style={{ marginTop: '10px' }}>
                                  <div className="sim-form-col">
                                      <label className="sim-label">Volatility</label>
                                      <input 
                                          type="number" 
                                          min="0"
                                          className="sim-input"
                                          value={stock.volatility}
                                          onChange={(e) => updateStock(index, 'volatility', e.target.value)}
                                      />
                                  </div>
                                  <div className="sim-form-col">
                                      <label className="sim-label">Liquidity</label>
                                      <input 
                                          type="number" 
                                          min="0"
                                          className="sim-input"
                                          value={stock.liquidity}
                                          onChange={(e) => updateStock(index, 'liquidity', e.target.value)}
                                      />
                                  </div>
                                  <div className="sim-form-col">
                                      <label className="sim-label">Market Cap</label>
                                      <input 
                                          type="number" 
                                          min="0"
                                          className="sim-input"
                                          value={stock.market_cap}
                                          onChange={(e) => updateStock(index, 'market_cap', e.target.value)}
                                      />
                                  </div>
                              </div>

                              {stocks.length > 1 && (
                                  <button 
                                      className="sim-btn-secondary" 
                                      onClick={() => setStocks(stocks.filter((_, i) => i !== index))}
                                  >
                                      Remove Stock
                                  </button>
                              )}
                          </div>
                    ))}

                    <button className="sim-btn-secondary" onClick={addStock} style={{ marginBottom: '20px' }}>
                        + Add Stock
                    </button>

                        <button 
                            className="sim-btn-primary"
                            onClick={handleStart}
                        >
                            Run Backtest
                        </button>
                    </div>
                </>
           ) : ( //ADDED THIS PART IN ORDER TO BE ABLE TO SEE METRICS AND DOWNLOAD UPON SIMULATION COMPLETION -> SHOWS THE DUMMY VAL I GENERATED FOR NOW
                <div className="sim-panel">
                    <h2>Engine Status</h2>
                    
                    {runResult ? (
                        <div style={{ backgroundColor: '#fff', border: '1px solid #ddd', borderRadius: '8px', padding: '15px 20px', marginBottom: '20px', boxShadow: '0 2px 4px rgba(0,0,0,0.02)' }}>
                            <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
                                
                                {/* LEFT: Config */}
                                <div style={{ flex: '1' }}>
                                    <div style={{ fontWeight: 'bold', fontSize: '16px', color: '#007bff', marginBottom: '4px' }}>
                                        {runResult.config.stocks.map((s: any) => s.ticker || s.name).join(', ')}
                                    </div>
                                    <div style={{ fontSize: '12px', color: '#666' }}>
                                        Cap: ${runResult.config.initial_capital?.toLocaleString()} | Fee: ${runResult.config.trade_fee?.toFixed(2)}
                                    </div>
                                </div>

                                {/* MIDDLE: Metrics */}
                                <div style={{ display: 'flex', gap: '30px', flex: '2', justifyContent: 'center' }}>
                                    <div style={{ textAlign: 'center' }}>
                                        <div style={{ fontSize: '11px', color: '#888', textTransform: 'uppercase' }}>Net Profit</div>
                                        <div style={{ fontWeight: 'bold', color: runResult.metrics.net_profit >= 0 ? 'green' : 'red' }}>
                                            ${runResult.metrics.net_profit?.toLocaleString()}
                                        </div>
                                    </div>
                                    <div style={{ textAlign: 'center' }}>
                                        <div style={{ fontSize: '11px', color: '#888', textTransform: 'uppercase' }}>Win Rate</div>
                                        <div style={{ fontWeight: 'bold' }}>{runResult.metrics.win_rate_percent?.toFixed(1)}%</div>
                                    </div>
                                    <div style={{ textAlign: 'center' }}>
                                        <div style={{ fontSize: '11px', color: '#888', textTransform: 'uppercase' }}>Trades</div>
                                        <div style={{ fontWeight: 'bold' }}>{runResult.metrics.total_trades}</div>
                                    </div>
                                </div>

                                {/* RIGHT: Actions */}
                                <div style={{ flex: '1', display: 'flex', justifyContent: 'flex-end' }}>
                                    <button 
                                        onClick={() => {
                                            const jsonString = JSON.stringify(runResult, null, 2);
                                            const blob = new Blob([jsonString], { type: "application/json" });
                                            const url = URL.createObjectURL(blob);
                                            const link = document.createElement('a');
                                            link.href = url;
                                            const tickerNames = runResult.config.stocks.map((s: any) => s.ticker || s.name).join('_');
                                            link.download = `sim_${tickerNames}.json`;
                                            document.body.appendChild(link);
                                            link.click();
                                            document.body.removeChild(link);
                                            URL.revokeObjectURL(url);
                                        }}
                                        style={{ padding: '8px 12px', backgroundColor: '#28a745', color: 'white', border: 'none', borderRadius: '4px', cursor: 'pointer', fontSize: '14px', fontWeight: 'bold' }}
                                    >
                                        ⬇️ JSON
                                    </button>
                                </div>
                            </div>
                        </div>
                    ) : (
                        <pre className="sim-status-box">
                            {engineStatus}
                        </pre>
                    )}

                    <button 
                        className="sim-btn-secondary"
                        onClick={() => {
                            setIsRunning(false);
                            setRunResult(null); 
                        }}
                    >
                        ← Back to Settings
                    </button>
                </div>
            )}

        </div>
    );
};

export default Simulation;