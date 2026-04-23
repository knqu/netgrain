import React, { useState, useRef, useEffect } from 'react';
import { useNavigate } from "react-router-dom";
import "../styling/newSim.css"
import EditorContainer  from "./CodeEditor";
import SimulationRun from './SimulationRun';
import '../styling/Simulation.css';
import * as monaco from 'monaco-editor';

function SimulationExec() {
  const navigate = useNavigate();

  async function saveSim() {
    try {
      const response = await fetch(
        "/api/saveSim",
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

void SimulationExec;

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
    script: String;
    // include script
}

const Simulation: React.FC = () => {
    const navigate = useNavigate();

    const saveSim = async () => {
        try {
            const response = await fetch("/api/saveSim", {
                method: "GET",
            });
            if (response.status === 200) {
                navigate("/simResults"); 
            }
        } catch (error) {
            console.log(error);
        }
    };
    void saveSim;

    // --- NEW: ASSET CLASS STATE ---
    const [marketData, setMarketData] = useState<Record<string, string[]>>({});
    const [activeAssetClass, setActiveAssetClass] = useState<string>("Stocks");

    const editorInstanceRef = useRef<monaco.editor.IStandaloneCodeEditor | null>(null);
    // --- SIMULATION STATE ---
    const [initialCapital, setInitialCapital] = useState<number | string>(100000);
    const [stocks, setStocks] = useState<StockParams[]>([
      { ticker: "", base_price: 0, liquidity: 0, volatility: 0, market_cap: 0},
    ]);
    const [startDate, setStartDate] = useState<string>("");
    const [endDate, setEndDate] = useState<string>("");
    const [tradeFee, setTradeFee] = useState<number | string>(1.50);
    const [isRunning, setIsRunning] = useState<boolean>(false);
    const [engineStatus, setEngineStatus] = useState<string>("");
    const [runResult, setRunResult] = useState<any>(null);
    
    // --- UPLOAD STATE ---
    const [uploadFile, setUploadFile] = useState<File | null>(null);
    const [uploadStatus, setUploadStatus] = useState<string>("");


    const [currentPage, setPage] = useState<string>("Sim");

    // --- NEW: FETCH AVAILABLE TICKERS ON LOAD ---
    useEffect(() => {
        const fetchMarketData = async () => {
            try {
                const response = await fetch("/api/market");
                if (response.ok) {
                    const data = await response.json();
                    setMarketData(data);
                    
                    // Default to whatever the first available class is if "Stocks" is missing
                    if (Object.keys(data).length > 0 && !data["Stocks"]) {
                        setActiveAssetClass(Object.keys(data)[0]);
                    }
                }
            } catch (error) {
                console.error("Failed to fetch market data:", error);
            }
        };
        fetchMarketData();
    }, []);

    const addStock = () => {
      setStocks([...stocks, { ticker: "", base_price: 0, liquidity: 0, volatility: 0, market_cap: 0 }]);
    };

    const removeStock = (index: number) => {
      setStocks(stocks.filter((_, i) => i !== index));
    };
    void removeStock;

    const updateStock = (index: number, field: keyof StockParams, value: string | number) => {      
      setStocks(currentStocks => {
        const currStocks = [...currentStocks];
        currStocks[index] = { 
          ...currStocks[index], 
          [field]: field === 'ticker' ? value : Number(value) 
        };
        return currStocks; 
      });
    };

    // --- FILE UPLOAD HANDLERS ---
    const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        if (e.target.files && e.target.files.length > 0) {
            const file = e.target.files[0];
            if (!file.name.endsWith('.csv') && !file.name.endsWith('.txt')) {
                setUploadStatus("Error: All files must be .csv or .txt.");
                setUploadFile(null);
                e.target.value = ""; 
                return;
            }
            setUploadFile(file);
            setUploadStatus(""); 
        }
    };

    const handleUpload = async () => {
        if (!uploadFile) {
            setUploadStatus("Please select a file first.");
            return;
        }

        setUploadStatus("Uploading...");

        try {
            const response = await fetch("/api/upload", {
                method: "POST",
                headers: {
                    "x-file-name": uploadFile.name,
                    "x-asset-class": activeAssetClass // Pass the active class to C++
                },
                body: uploadFile
            });

            if (response.ok) {
                const ticker = await response.text();
                setUploadStatus(`Success! ${ticker} loaded into ${activeAssetClass}.`);
                
                // Re-fetch the market data so the new ticker instantly appears in the dropdown
                const marketRes = await fetch("/api/market");
                if (marketRes.ok) {
                    setMarketData(await marketRes.json());
                }

                setStocks(currentStocks => {
                    const alreadyExists = currentStocks.some(s => s.ticker === ticker);
                    if (alreadyExists) return currentStocks;

                    if (currentStocks.length === 1 && currentStocks[0].ticker === "") {
                        return [{ ...currentStocks[0], ticker: ticker }];
                    }
                    
                    return [...currentStocks, { ticker: ticker, base_price: 0, liquidity: 0, volatility: 0, market_cap: 0 }];
                });

            } else if (response.status === 409) {
                setUploadStatus(`File already exists in the backend.`);
            } else {
                setUploadStatus(`Upload failed: ${response.statusText}`);
            }
        } catch (error) {
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

        const pythonScript = editorInstanceRef.current?.getValue() || "";
        const payload: SimulationConfigRequest = {
            initial_capital: Number(initialCapital),
            stocks: stocks,
            start_date: startDate,
            end_date: endDate,
            trade_fee: Number(tradeFee),
            script: pythonScript,
            // include script here
        };

        setIsRunning(true);
        setEngineStatus("Sending configuration to engine...");

        try {
            const response = await fetch("/api/simulate", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload)
            });
            console.log(JSON.stringify(payload));

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
    function ConfigUI() {
    return (
        <div className="sim-container">
            
            {!isRunning ? (
                <div className="sim-setup">
                    {/* --- FILE UPLOAD PANEL --- */}
                    <div className="upload-panel">
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
                                <div style={{ fontSize: '14px', marginTop: '10px', color: uploadStatus.includes('Success') ? 'green' : (uploadStatus.includes('failed') || uploadStatus.includes('Error') ? 'red' : '#555') }}>
                                    <strong>{uploadStatus}</strong>
                                </div>
                            )}
                        </div>
                    </div>
                    {/* --- SIMULATION SETTINGS PANEL --- */}
                    {/* <div className="sim-panel"> */}

                        
                        {/* --- MASTER ASSET CLASS SELECTOR --- */}
                        <div className="sim-asset-selector">
                        <div className="sim-form-group">
                            <label className="sim-label">Asset Class</label>
                            <select 
                                className="sim-input"
                                value={activeAssetClass}
                                onChange={(e) => {
                                    setActiveAssetClass(e.target.value);
                                    // Reset stocks when switching classes so you don't mix them
                                    setStocks([{ ticker: "", base_price: 0, liquidity: 0, volatility: 0, market_cap: 0 }]);
                                }}
                                style={{ fontWeight: 'bold', backgroundColor: '#f8f9fa' }}
                            >
                                {Object.keys(marketData).length > 0 ? (
                                    Object.keys(marketData).map((className) => (
                                        <option key={className} value={className}>{className}</option>
                                    ))
                                ) : (
                                    <option value="Stocks">Stocks</option>
                                )}
                            </select>
                        </div>

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
                        </div>
                        <div className="stocks">
                            <div className="sim-stock-cards">
                                {stocks.map((stock, index) => (
                                <div key={index} className="sim-stock-card">
                                    <div className="sim-form-row">
                                        <div className="sim-form-col">
                                            <label className="sim-label">Ticker</label>
                                            {/* --- FILTERED TICKER DROPDOWN --- */}
                                            <select 
                                                className="sim-input"
                                                value={stock.ticker}
                                                onChange={(e) => updateStock(index, 'ticker', e.target.value)}
                                            >
                                                <option value="">Select a Ticker...</option>
                                                {(marketData[activeAssetClass] || []).map((t) => (
                                                    <option key={t} value={t}>{t}</option>
                                                ))}
                                            </select>
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
                            </div>
                            <button className="sim-btn-secondary" onClick={addStock} style={{ marginBottom: '20px' }}>
                                + Add Stock
                            </button>
                        </div>
                        {/*<button 
                            className="sim-btn-primary"
                            onClick={handleStart}
                        >
                            Run Backtest
                        </button>*/}
                    {/* </div> */}
                </div>
            ) : (
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
    }

    const startSim = async () => {
        handleStart();
        setPage("Start");
    };    

    const handleLangChange = (event: any) => {
        if (editorInstanceRef.current != null)
        {
            const target = event.target! as HTMLSelectElement;
            const model = editorInstanceRef.current!.getModel()!;
            monaco.editor.setModelLanguage(model, target.value);
        }
    };

    function renderPage() {
        switch (currentPage) {
            case "Sim":
                return (
                    <div className="outer-sim">
                        <div className="action-board">
                            <select name="type" id="type-select" onChange={handleLangChange}>
                                <option value="python" selected>Python</option>
                                <option value="cpp">C++</option>
                            </select>
                            <button
                                className="startSim"
                                onClick={startSim}>
                                Start Simulation
                            </button>
                        </div>
                        <div className="sim-grid">
                            <div className="grid-item">
                                <EditorContainer onMount={(editor) => {(editorInstanceRef.current = editor)}} />
                            </div>
                            <div className="grid-item">
                                <ConfigUI />
                            </div>
                        </div>
                    </div>
                );
                case "Start":
                    return (<SimulationRun num_stocks={stocks.length}/>);
        }
    }

    return(
        <>
            {renderPage()}
        </>
    )


};

export default Simulation;
