import yfinance as yf
import pandas as pd
import os

# Base data directory
base_dir = "data"

# Expanded Asset Dictionary
assets = {
    "Crypto": {
        "BTC": "BTC-USD", "ETH": "ETH-USD", "BNB": "BNB-USD", "SOL": "SOL-USD", 
        "XRP": "XRP-USD", "ADA": "ADA-USD", "AVAX": "AVAX-USD", "DOT": "DOT-USD", 
        "LINK": "LINK-USD", "LTC": "LTC-USD", "SHIB": "SHIB-USD"
    },
    "Forex": {
        "EURUSD": "EURUSD=X", "USDJPY": "JPY=X", "GBPUSD": "GBPUSD=X", 
        "AUDUSD": "AUDUSD=X", "USDCAD": "CAD=X", "USDCHF": "CHF=X", 
        "NZDUSD": "NZDUSD=X", "EURJPY": "EURJPY=X", "GBPJPY": "GBPJPY=X", 
        "EURGBP": "EURGBP=X"
    },
    "Futures": {
        "GOLD": "GC=F", "SILVER": "SI=F", "COPPER": "HG=F", "PLATINUM": "PL=F", 
        "CRUDE_OIL": "CL=F", "NAT_GAS": "NG=F", "CORN": "ZC=F", "WHEAT": "ZW=F", 
        "SP500_FUT": "ES=F", "NASDAQ_FUT": "NQ=F"
    }
}

print("Downloading day-by-day historical data into organized folders...")

for asset_class, tickers in assets.items():
    print(f"\n--- Fetching {asset_class} ---")
    
    # NEW: Create a specific subfolder for this asset class
    class_dir = os.path.join(base_dir, asset_class)
    os.makedirs(class_dir, exist_ok=True)
    
    for name, symbol in tickers.items():
        print(f"Fetching {name} ({symbol})...")
        
        df = yf.download(symbol, period="5y", interval="1d", progress=False)
        
        if df.empty:
            print(f"  -> ❌ Failed to grab data for {name}")
            continue

        if isinstance(df.columns, pd.MultiIndex):
            df.columns = df.columns.get_level_values(0)

        df.reset_index(inplace=True)
        df['Date'] = df['Date'].dt.strftime('%Y-%m-%d')
        df['Open Int'] = 0

        try:
            final_df = df[['Date', 'Open', 'High', 'Low', 'Close', 'Volume', 'Open Int']]
        except KeyError:
            print(f"  -> ❌ Missing columns for {name}, skipping.")
            continue

        final_df.fillna(0, inplace=True)

        # NEW: Save to the specific subfolder instead of the main data folder
        file_path = os.path.join(class_dir, f"{name}.csv")
        final_df.to_csv(file_path, index=False)
        print(f"  -> ✅ Saved to {file_path}")

print("\n🚀 All bulk data downloaded and organized into subfolders!")