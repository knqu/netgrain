import React, { useState, useEffect } from 'react';
//sim result payload used in the hardcoded run 
export interface SimulationResultPayload {
    config: {
        initial_capital: number;
        tickers: string[];
        start_date: string;
        end_date: string;
        trade_fee: number;
    };
    metrics: {
        total_trades: number;
        winning_trades: number;
        losing_trades: number;
        win_rate_percent: number;
        initial_balance: number;
        final_balance: number;
        net_profit: number;
    };
}

const SimulationResults: React.FC = () => {
    const [pastRuns, setPastRuns] = useState<SimulationResultPayload[]>([]);
    const [statusMsg, setStatusMsg] = useState<string>("Fetching data from C++ Engine...");

    useEffect(() => {
        //hardcode run will update in future when simulation is set in stone and metrics are set in stone
        const fetchHardcodedRun = async () => {
            try {
                const response = await fetch("http://localhost:8080/api/results");
                if (response.ok) {
                    const data = await response.json();
                    if (!data.error) {
                        setPastRuns(prev => [...prev, data]);
                    }
                }
            } catch (error) {
                console.error("Fetch error:", error);
            }
        };
        fetchHardcodedRun();
    }, []);

    const handleFileUpload = (e: React.ChangeEvent<HTMLInputElement>) => {
        const file = e.target.files?.[0];
        if (!file) return;

        const reader = new FileReader();
        reader.onload = (event) => {
            try {
                const jsonContent = event.target?.result as string;
                const uploadedResult: SimulationResultPayload = JSON.parse(jsonContent);
                if (uploadedResult.metrics && uploadedResult.config) {
                    setPastRuns(prev => [...prev, uploadedResult]);
                } else {
                    alert("Invalid JSON format.");
                }
            } catch (err) {
                alert("Failed to parse JSON file.");
            }
        };
        reader.readAsText(file);
        e.target.value = "";
    };

    //remove card function
    const removeRun = (indexToRemove: number) => {
        // We use the original index from the pastRuns array
        setPastRuns(prev => prev.filter((_, index) => index !== indexToRemove));
    };

    const handleDownload = (result: SimulationResultPayload) => {
        const jsonString = JSON.stringify(result, null, 2);
        const blob = new Blob([jsonString], { type: "application/json" });
        const url = URL.createObjectURL(blob);
        const link = document.createElement('a');
        link.href = url;
        link.download = `sim_${result.config.tickers.join('_')}.json`; 
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
        URL.revokeObjectURL(url);
    };

    return (
        <div style={{ padding: '20px', maxWidth: '900px', margin: '0 auto' }}>
            
            {/* UPLOAD UI SECTION */}
            <div style={{ 
                backgroundColor: '#f1f3f5', 
                padding: '20px', 
                borderRadius: '8px', 
                marginBottom: '30px',
                border: '2px dashed #ced4da',
                textAlign: 'center'
            }}>
                <h3 style={{ marginTop: 0, fontSize: '1rem', color: '#495057' }}>Import Past Results</h3>
                <input 
                    type="file" 
                    accept=".json" 
                    onChange={handleFileUpload} 
                    style={{ fontSize: '14px' }}
                />
            </div>

            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '20px' }}>
                <h2 style={{ fontSize: '1.5rem', color: '#333' }}>Simulation History</h2>
            </div>
            
            {pastRuns.length === 0 ? (
                <div style={{ textAlign: 'center', padding: '40px', backgroundColor: '#f8f9fa', borderRadius: '8px', color: '#666' }}>
                    {statusMsg}
                </div>
            ) : (
                /* We map in reverse order for display, but we keep track of the 
                   ORIGINAL index to make sure we delete the correct item.
                */
                pastRuns.map((run, index) => (
                    <div 
                        key={index} 
                        style={{ 
                            display: 'flex', 
                            flexDirection: 'column', // Stack the main row and a potential "Delete" bar
                            backgroundColor: '#fff', 
                            border: '1px solid #ddd', 
                            borderRadius: '8px', 
                            marginBottom: '10px',
                            boxShadow: '0 2px 4px rgba(0,0,0,0.02)',
                            overflow: 'hidden'
                        }}
                    >
                        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', padding: '15px 20px' }}>
                            {/* LEFT: Configuration Info */}
                            <div style={{ flex: '1' }}>
                                <div style={{ fontWeight: 'bold', fontSize: '16px', color: '#007bff', marginBottom: '4px' }}>
                                    {run.config.tickers.join(', ')}
                                </div>
                                <div style={{ fontSize: '12px', color: '#666' }}>
                                    Cap: ${run.config.initial_capital.toLocaleString()} | Fee: ${run.config.trade_fee.toFixed(2)}
                                </div>
                            </div>

                            {/* MIDDLE: Key Metrics */}
                            <div style={{ display: 'flex', gap: '30px', flex: '2', justifyContent: 'center' }}>
                                <div style={{ textAlign: 'center' }}>
                                    <div style={{ fontSize: '11px', color: '#888', textTransform: 'uppercase' }}>Net Profit</div>
                                    <div style={{ fontWeight: 'bold', color: run.metrics.net_profit >= 0 ? 'green' : 'red' }}>
                                        ${run.metrics.net_profit.toLocaleString()}
                                    </div>
                                </div>
                                <div style={{ textAlign: 'center' }}>
                                    <div style={{ fontSize: '11px', color: '#888', textTransform: 'uppercase' }}>Win Rate</div>
                                    <div style={{ fontWeight: 'bold' }}>{run.metrics.win_rate_percent.toFixed(1)}%</div>
                                </div>
                                <div style={{ textAlign: 'center' }}>
                                    <div style={{ fontSize: '11px', color: '#888', textTransform: 'uppercase' }}>Trades</div>
                                    <div style={{ fontWeight: 'bold' }}>{run.metrics.total_trades}</div>
                                </div>
                            </div>

                            {/* RIGHT: Actions */}
                            <div style={{ flex: '1', display: 'flex', justifyContent: 'flex-end', gap: '10px' }}>
                                <button 
                                    onClick={() => handleDownload(run)}
                                    title="Download JSON"
                                    style={{ padding: '8px 12px', backgroundColor: '#28a745', color: 'white', border: 'none', borderRadius: '4px', cursor: 'pointer', fontSize: '14px' }}
                                >
                                    ⬇️
                                </button>
                                <button 
                                    onClick={() => removeRun(index)}
                                    title="Remove Card"
                                    style={{ padding: '8px 12px', backgroundColor: '#dc3545', color: 'white', border: 'none', borderRadius: '4px', cursor: 'pointer', fontSize: '14px' }}
                                >
                                    🗑️
                                </button>
                            </div>
                        </div>
                    </div>
                )).reverse() // Show newest at the top
            )}
        </div>
    );
};

export default SimulationResults;