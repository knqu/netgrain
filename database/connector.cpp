#include <stdio.h>
#include <iostream>
#include <pqxx/pqxx>
#include <fmt/core.h>
#include <fmt/base.h>
#include <regex>

enum error_codes {
  UUID_NOT_FOUND, 
  INVALID_EMAIL_FORMAT,
  EMAIL_ALREADY_REGISTERED,
  USERNAME_ALREADY_REGISTERED,
  INVALID_CREDENTIALS,
  LEADERBOARD_ATTEMPT_MADE,
  SUCCESS,
  CUSTOM_DASHBOARD_CONFIG_NOT_FOUND,
  PRESET_ID_NOT_FOUND,
  DUPLICATE_PRESET_FOUND,
};

/*
 * General Connector class (used Singleton design pattern) that can be used to interact with PostgreSQL database
 */
class ConnectorSingleton {
  private:
      ConnectorSingleton() = default;
      ~ConnectorSingleton() = default;
      inline static ConnectorSingleton* instance = nullptr;
      inline static pqxx::connection* conn;

      /*
       * Helper function to determine if email provided is correct format (doesn't check if it actually exists)
       */
      int isValidEmail(std::string email) {
        std::regex self_regex("[A-Za-z0-9]*@[A-Za-z9-9]*.com$");
        if (regex_match(email, self_regex))
          return SUCCESS;
        return INVALID_EMAIL_FORMAT;
      }

      int uuIDfound(int uuid) {
        pqxx::work tx(*conn);
        pqxx::params p;
        p.append(uuid);

        // std::string query = "SELECT * FROM userlogin WHERE (userid = " + std::to_string(uuid) + ")";
        std::string query = "SELECT * FROM userlogin WHERE (userid = $1)";

        try
        {
          pqxx::row r = tx.exec(query, p).one_row(); // one_row checks if only one entry found, exception otherwise
        }
        catch (const std::exception &e)
        {
            return UUID_NOT_FOUND;
        }
        return SUCCESS;
      }

  public:
    static ConnectorSingleton& getInstance() {
      if (!instance) {
          instance = new ConnectorSingleton();
          try
          {
            conn = new pqxx::connection(
              "host=localhost "
              "dbname=netgrain_db");
          }
          catch (const std::exception &e)
          {
              std::cerr << e.what() << std::endl;
          }
      }
      return *instance;
    }

    /*
     * Login: determines if given username/email and password is found in userlogin table
     */
    int login(std::string identifier, std::string password) {
      pqxx::work tx(*conn);
      pqxx::params p;
      p.append(identifier);
      p.append(password);

      // std::string query = "SELECT * FROM userlogin WHERE (password = \'" + password + "\') AND (email = \'" + identifier + "\' OR username = \'" + identifier + "\')";
      std::string query = "SELECT * FROM userlogin WHERE (password = $2) AND (email = $1 OR username = $1)";

      try
      {
        pqxx::row r = tx.exec(query, p).one_row(); // one_row checks if only one entry found, exception otherwise
      }
      catch (const std::exception &e)
      {
        return false;
      }
      return true;
    }

    /*
     * Return userID of user to identify during user session
     * Permanent Change
     * pre-condition: if valid format, email actually exists
     */
    int addUser(std::string email, std::string password, std::string username) {
      if (isValidEmail(email) == INVALID_EMAIL_FORMAT) {
        fmt::print("{}: INVALID_EMAIL_FORMAT\n", email);
        return INVALID_EMAIL_FORMAT;
      }

      fmt::print("\nTesting email {} username {}\n", email, username);
      pqxx::work tx(*conn);
      pqxx::params p;
      p.append(email);
      p.append(password);
      p.append(username);

      // Check if duplicate username
      try
      {
        std::string query = "SELECT * FROM userlogin WHERE (username = $1)";
        pqxx::result r = tx.exec(query, pqxx::params{username}).no_rows(); // one_row checks if only one entry found, exception otherwise
      }
      catch (const std::exception &e)
      {

        std::cerr << e.what() << std::endl;
        fmt::print("USERNAME_ALREADY_REGISTERED: {}\n", username);
        return USERNAME_ALREADY_REGISTERED;
      }

      // Check if duplicate email
      try
      {
        std::string query = "SELECT * FROM userlogin WHERE (email = $1)";
        pqxx::result r = tx.exec(query, pqxx::params{email}).no_rows(); // one_row checks if only one entry found, exception otherwise
      }
      catch (const std::exception &e)
      {
        fmt::print("EMAIL_ALREADY_REGISTERED: {}\n", email);
        return EMAIL_ALREADY_REGISTERED;
      }

      try // insert new credentials
      {
        // std::string query = "INSERT INTO userlogin (username, email, password) VALUES (\'" + username + "\',\'" + email + "\',\'" + password + "\')";
        std::string query = "INSERT INTO userlogin (username, email, password) VALUES ($3, $1, $2)";
        tx.exec(query, p);
        tx.commit();
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }

      pqxx::work tx2(*conn);

      fmt::print("Hi {} {}\n", email, username);

      // Grab the userID after insertion
      // std::string query = "SELECT * FROM userlogin WHERE (email = \'" + email+ "\')";
      std::string query = "SELECT * FROM userlogin WHERE (email = $1)";
      pqxx::row r = tx2.exec(query, pqxx::params{email}).one_row();

      fmt::print("SUCCESS\n");
      return r[0].as<int>();
    }

    /*
     * When user modifies dashboard, changes are updated automatically to userID.file
     * let web server deal with file IO? database will just hold reference
     */
    int linkCustomGUILayout(int userID, std::string path_to_file) {
      if (uuIDfound(userID) == UUID_NOT_FOUND) {
        return UUID_NOT_FOUND;
      }

      pqxx::work tx(*conn);
      pqxx::params p;
      p.append(userID);
      p.append(path_to_file);

      try {
        // std::string query = "UPDATE userlogin SET customguicomponentlayout = \'" + path_to_file + "\' WHERE userid = " + std::to_string(userID);
        std::string query = "UPDATE userlogin SET customguicomponentlayout = $2 WHERE userid = $1";
        tx.exec(query, p);
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }

      tx.commit();

      return SUCCESS;
    }

    /*
     *
     */
    int userHasCustomLayout(int userID) {
      if (uuIDfound(userID) == UUID_NOT_FOUND) {
        fmt::print("UUID NOT FOUND\n");
        return UUID_NOT_FOUND;
      }

      pqxx::work tx(*conn);
      pqxx::params p;
      p.append(userID);

      std::string result = "";

      try {
        // std::string query = "SELECT * FROM userlogin WHERE userid = " + std::to_string(userID);
        std::string query = "SELECT * FROM userlogin WHERE userid = $1";
        pqxx::row r = tx.exec(query, p).one_row();
        result = r[4].as<std::string>();
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }

      if (result.empty()) {
        fmt::print("CUSTOM_DASHBOARD_CONFIG_NOT_FOUND\n");
        return CUSTOM_DASHBOARD_CONFIG_NOT_FOUND;
      }
      fmt::print("SUCCESS: {}\n", result);
      return SUCCESS;
    }

    /*
     * Pre-condition: userHasCustomLayout() was called to confirm
     */
    std::string getCustomGUILayout(int userID) {
      pqxx::work tx(*conn);
      pqxx::params p;
      p.append(userID);

      std::string result = "";
      try {
        std::string query = "SELECT * FROM userlogin WHERE userid = $1";
        pqxx::row r = tx.exec(query, p).one_row();
        result = r[4].as<std::string>();
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }
      return result;
    }

    // Persistent Change
    // adding again, uuid doesn't exist, 
    // convert simulation time into timestamp
    // simulationTime should be string HH:MM:SS, there are some workarounds needed for other time libraries
    int addLeaderboardAttempt(int uuID, int profit, std::string simulationTime) {
      fmt::print("\n");

      if (uuIDfound(uuID) == UUID_NOT_FOUND) {
        fmt::print("UUID_NOT_FOUND");
        return UUID_NOT_FOUND;
      }

      pqxx::work tx(*conn);
      pqxx::params p;
      p.append(uuID);
      p.append(profit);
      p.append(simulationTime);

      // Check if user already made attempt
      try
      {
        std::string query = "SELECT * FROM leaderboard WHERE (userid = $1)";
        pqxx::result r = tx.exec(query, pqxx::params{uuID}).no_rows(); // one_row checks if only one entry found, exception otherwise
      }
      catch (const std::exception &e)
      {
        return LEADERBOARD_ATTEMPT_MADE;
      }

      try {
        std::string query = "INSERT INTO leaderboard (userid, profit, simulationtime) VALUES ($1, $2, $3)";
        tx.exec(query, p);
        tx.commit();
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }

      fmt::print("SUCCESS");
      return SUCCESS;
    }

    /*
     * Currently prints out to console, no testing for this yet
     */
    void fetchLeaderBoard() {
      pqxx::work tx(*conn);
      std::string query = "SELECT * FROM leaderboard ORDER BY profit DESC, simulationtime DESC";
      pqxx::result r = tx.exec(query);

      for (auto row = std::begin(r); row != std::end(r); row++)
      {
        for (auto field = std::begin(row); field != std::end(row); field++)
          fmt::print("{}\t", field->c_str());
        fmt::print("\n");
      }
      return;
    }

    /*
     * pass in customPresetID = -1 if no global preset was used
     */
    int createSimulation(int uuID, std::string analytics, std::string configUsed, int globalPresetID) {
      if (uuIDfound(uuID) == UUID_NOT_FOUND) {
        return UUID_NOT_FOUND;
      }

      pqxx::work tx(*conn);
      pqxx::params p;
      p.append(analytics);
      p.append(configUsed);
      p.append(uuID);
      p.append(globalPresetID);

      try {
        std::string query = "INSERT INTO pastsimulations (analytics, configused, userid) VALUES ($1, $2, $3)";
        tx.exec(query, pqxx::params{analytics, configUsed, uuID});
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }

      if (globalPresetID != -1) {
        // determine if the globalPresetID is valid
        std::string query = "SELECT * FROM globalcustompresets WHERE (presetid = $1)";

        try
        {
          pqxx::row r = tx.exec(query, pqxx::params{globalPresetID}).one_row(); // one_row checks if only one entry found, exception otherwise
        }
        catch (const std::exception &e)
        {
            return PRESET_ID_NOT_FOUND;
        }

        try {
          std::string query = "UPDATE pastsimulations SET custompresetusedifapplicable = $1 WHERE userid = $2";
          tx.exec(query, pqxx::params{configUsed, uuID});
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
      }
      tx.commit();

      return SUCCESS;
    }

    // createCustomGlobalPreset(...)
    int createCustomGlobalPreset(int scaling, int volatility, int liquidity, int tradingVol) {
      pqxx::work tx(*conn);
      pqxx::params p;
      p.append(scaling);
      p.append(volatility);
      p.append(liquidity);
      p.append(tradingVol);


      // determine if duplicate global preset exists
      try
      {
      std::string query = "SELECT * FROM globalcustompresets WHERE (scaling = $1) AND (volatility = $2) AND (liquidity = $3) AND (tradingvolume = $4)";
        pqxx::result r = tx.exec(query, p).no_rows(); // one_row checks if only one entry found, exception otherwise
      }
      catch (const std::exception &e)
      {
          return DUPLICATE_PRESET_FOUND;
      }

      try {
        std::string query = "INSERT INTO globalcustompresets (scaling, volatility, liquidity, tradingvolume) VALUES ($1, $2, $3, $4)";
        tx.exec(query, p);
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }
      tx.commit();

      // Grab the global preset ID after insertion
      std::string query = "SELECT * FROM globalcustompresets WHERE (scaling = $1) AND (volatility = $2) AND (liquidity = $3) AND (tradingvolume = $4)";
      pqxx::row r = tx.exec(query, p).one_row();
      tx.commit();
      return r[0].as<int>();
    }
};

void test() {
  fmt::print("\n---Initializing Testing---\n");

  ConnectorSingleton::getInstance();

  /*
   * To testers, you may need to addUser() up to 7 users for below tests to work, otherwise just change literals below
   */

  /*
   * TESTS FOR TABLE USERLOGIN
   */

  // Table userlogin -- addUser (aka registering a user)
  ConnectorSingleton::getInstance().addUser("uniqueEmail@email.com", "12341234", "uniqueUser"); //insert unique user
  ConnectorSingleton::getInstance().addUser("uniqueEmail2@email.com", "12341234", "uniqueUser2"); //insert unique user
  assert(ConnectorSingleton::getInstance().addUser("different@gmail.com", "", "uniqueUser") == USERNAME_ALREADY_REGISTERED);
  assert(ConnectorSingleton::getInstance().addUser("uniqueEmail@email.com", "", "asdf") == EMAIL_ALREADY_REGISTERED);

  assert(ConnectorSingleton::getInstance().addUser("thisIsNotAnEmail", "", "") == INVALID_EMAIL_FORMAT);

  // Table userlogin -- login
  assert(ConnectorSingleton::getInstance().login("uniqueEmail@email.com", "12341234")); // Test Case 1 (user exists): verify login successfully
  
  assert(! ConnectorSingleton::getInstance().login("hiIDontExist.com", "pleaseFail")); // Test Case 2 (user doesn't exist): verify login failed
  
  // Table userlogin -- link + get customGUI tied to a user
  // Pre-condition: web server handles file IO (initialize and write to file)
  assert(ConnectorSingleton::getInstance().linkCustomGUILayout(1000, "/path/to/file") == UUID_NOT_FOUND); // attempt to link file with not found uuid
  assert(ConnectorSingleton::getInstance().userHasCustomLayout(1000) == UUID_NOT_FOUND); // attempt to look for file with not found uuid
  assert(ConnectorSingleton::getInstance().userHasCustomLayout(2) == CUSTOM_DASHBOARD_CONFIG_NOT_FOUND); // see if it detects user without config

  assert(ConnectorSingleton::getInstance().linkCustomGUILayout(1, "/path/to/file") == SUCCESS); // link file with found uuid
  assert(ConnectorSingleton::getInstance().userHasCustomLayout(1) == SUCCESS); // see if file was written from above
  assert(! ConnectorSingleton::getInstance().getCustomGUILayout(1).compare("/path/to/file")); // determine if written successfully

  /*
   * TESTS FOR LEADERBOARD
   */
  // CHOOSE DIFFERENT EMAIL AND USERNAME HERE EVERY TIME
  std::string email = "asdfaasdf@gmail.com";
  std::string username = "alsdfsdkjfa";
  int uuid = ConnectorSingleton::getInstance().addUser(email, "helllo", username); //insert unique user

  fmt::print("SUCCESS TEST\n");
  assert(ConnectorSingleton::getInstance().addLeaderboardAttempt(uuid, 100000, "12:12:12") == SUCCESS); // normal write
  fmt::print("ATTEMPT TEST\n");
  assert(ConnectorSingleton::getInstance().addLeaderboardAttempt(uuid, 200000, "14:14:14") == LEADERBOARD_ATTEMPT_MADE); // same user makes another attempt
  fmt::print("UUID NOT FOUND TEST\n");
  assert(ConnectorSingleton::getInstance().addLeaderboardAttempt(-1, 200000, "14:14:14") == UUID_NOT_FOUND); // invalid user makes attempt

  /*
   * TESTS FOR A USER's SIMULATION
   */
  assert(ConnectorSingleton::getInstance().createSimulation(2, "insertSomeSmartAnalytics", "/path/to/config/used", -1) == SUCCESS); // normal write
  assert(ConnectorSingleton::getInstance().createSimulation(2, "insertSomeSmartAnalytics", "/path/to/config/used", 100) == PRESET_ID_NOT_FOUND); // unknown preset ID

  /*
   * TESTS FOR GLOBAL PRESETS
   */
  assert(ConnectorSingleton::getInstance().createCustomGlobalPreset(10, 10, 10, 10) == DUPLICATE_PRESET_FOUND);
}
