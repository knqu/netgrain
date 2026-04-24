DO $$ 
BEGIN
    EXECUTE (
        SELECT string_agg('DROP TABLE IF EXISTS ' || tablename || ' CASCADE;', ' ')
        FROM pg_tables
        WHERE schemaname = 'public'
    );
END $$;

CREATE TABLE userLogin (
  userID serial PRIMARY KEY,
  username text NOT NULL,
  email text NOT NULL,
  password text NOT NULL,
  customGUIComponentLayout text
);

CREATE TABLE leaderboard (
  userID int UNIQUE,
  profit int NOT NULL,
  simulationTime text NOT NULL,
  FOREIGN KEY (userID) REFERENCES userLogin(userID)
);

CREATE TABLE globalCustomPresets (
  presetID serial PRIMARY KEY,
  scaling int NOT NULL,
  volatility int NOT NULL,
  liquidity int NOT NULL,
  tradingVolume int NOT NULL
);

CREATE TABLE pastSimulations (
  simID serial PRIMARY KEY,
  userID int NOT NULL,
  path_to_data text NOT NULL,
  configUsed text NOT NULL,
  dateOfSim text NOT NULL,
  customPresetUsedIfApplicable int UNIQUE,
  FOREIGN KEY (userID) REFERENCES userLogin(userID),
  FOREIGN KEY (customPresetUsedIfApplicable) REFERENCES globalCustomPresets(presetID)
);

CREATE TABLE sim_fills (
  id serial PRIMARY KEY,
  sim_id int NOT NULL REFERENCES pastSimulations(simID),
  order_id int NOT NULL,
  ticker text NOT NULL,
  quantity int NOT NULL,
  fill_price int NOT NULL,
  side text NOT NULL,
  bar_timestamp int NOT NULL
);

CREATE TABLE sim_orders (
  id serial PRIMARY KEY,
  sim_id int NOT NULL REFERENCES pastSimulations(simID),
  order_id int NOT NULL,
  ticker text NOT NULL,
  quantity int NOT NULL,
  target_price int NOT NULL,
  side text NOT NULL,
  order_type text NOT NULL,
  status text NOT NULL
);

CREATE TABLE sim_balance_log (
  id serial PRIMARY KEY,
  sim_id int NOT NULL REFERENCES pastSimulations(simID),
  balance int NOT NULL,
  bar_timestamp int NOT NULL
);

CREATE TABLE sim_positions (
  id serial PRIMARY KEY,
  sim_id int NOT NULL REFERENCES pastSimulations(simID),
  ticker text NOT NULL,
  quantity int NOT NULL,
  cost_basis int NOT NULL,
  market_price int not NULL,
  bar_timestamp int NOT NULL
);

CREATE INDEX idx_sim_fills_sim_id ON sim_fills(sim_id);
CREATE INDEX idx_sim_orders_sim_id ON sim_orders(sim_id);
CREATE INDEX idx_sim_balance_log_sim_id ON sim_balance_log(sim_id);
CREATE INDEX idx_sim_positions_sim_id ON sim_positions(sim_id);
