import React from 'react';

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


//dummy data to populate for now 
const dummyRun: SimulationResultPayload = {
    config: {
        initial_capital: 100000,
        tickers: ["AAPL", "MSFT"],
        start_date: "2018-01-01",
        end_date: "2023-01-01",
        trade_fee: 1.50
    },
    metrics: {
        total_trades: 1240,
        winning_trades: 682,
        losing_trades: 558,
        win_rate_percent: 55.0,
        initial_balance: 100000,
        final_balance: 108450.25,
        net_profit: 8450.25
    }
};

// component displays a list of past simulation runs with key metrics and allows users to download the results as JSON files. Basic for now...
const SimulationResults: React.FC = () => {
    
    const pastRuns = [dummyRun];
    const handleDownload = (result: SimulationResultPayload) => {
        const jsonString = JSON.stringify(result, null, 2);
        const blob = new Blob([jsonString], { type: "application/json" });
        const url = URL.createObjectURL(blob);
        
        const link = document.createElement('a');
        link.href = url;
        //names download off tickers
        link.download = `sim_${result.config.tickers.join('_')}.json`; 
        
        document.body.appendChild(link);
        link.click();
        
        document.body.removeChild(link);
        URL.revokeObjectURL(url);
    };//idk I just searched how to donwload files in react and this is what I found...added the link.download to name the file off the tickers tho.

    return (
        <div style={{ marginTop: '30px' }}>
            <h2 style={{ fontSize: '1.2rem', marginBottom: '15px', color: '#333' }}>Simulation History</h2>
            
            {pastRuns.map((run, index) => (
                <div 
                    key={index} 
                    style={{ 
                        display: 'flex', 
                        alignItems: 'center', 
                        justifyContent: 'space-between',
                        backgroundColor: '#fff', 
                        border: '1px solid #ddd', 
                        borderRadius: '8px', 
                        padding: '15px 20px',
                        marginBottom: '10px',
                        boxShadow: '0 2px 4px rgba(0,0,0,0.02)'
                    }}
                >
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

                    {/* RIGHT: Download Action */}
                    <div style={{ flex: '1', display: 'flex', justifyContent: 'flex-end' }}>
                        <button 
                            onClick={() => handleDownload(run)}
                            style={{ 
                                padding: '8px 12px', 
                                backgroundColor: '#28a745', 
                                color: 'white', 
                                border: 'none', 
                                borderRadius: '4px', 
                                cursor: 'pointer',
                                fontSize: '14px',
                                fontWeight: 'bold'
                            }}
                        >
                            ⬇️ JSON
                        </button>
                    </div>
                </div>
            ))}
        </div>
    );
};

export default SimulationResults;