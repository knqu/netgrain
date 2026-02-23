CREATE TABLE userLogin (
  userID serial PRIMARY KEY,
  username text NOT NULL,
  email text NOT NULL,
  password text NOT NULL,
  customGUIComponentLayout text
);

CREATE TABLE leaderboard (
  userID int UNIQUE,
  profit int,
  simulationTime timestamp,
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
  simID serial,
  analytics text NOT NULL,
  configUsed text NOT NULL,
  userID int NOT NULL,
  customPresetUsedIfApplicable int UNIQUE,
  FOREIGN KEY (userID) REFERENCES userLogin(userID),
  FOREIGN KEY (customPresetUsedIfApplicable) REFERENCES globalCustomPresets(presetID)
);
