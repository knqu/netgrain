#include <stdio.h>
#include <iostream>
#include <pqxx/pqxx>
#include <fmt/core.h>
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

      /*
       * Helper function to determine if uuid is present in userlogin table
       */
      int uuIDfound(int uuid) {
        pqxx::work tx(*conn);
        std::string query = "SELECT * FROM userlogin WHERE (userid = $1)";
try
        {
          pqxx::row r = tx.exec(query, pqxx::params{uuid}).one_row();
        }
        catch (const std::exception &e)
        {
            return UUID_NOT_FOUND;
        }
        return SUCCESS;
      }

      /*
       * Helper function
       */
      int getUUID(std::string identifier) {
        pqxx::work tx(*conn);
        std::string query = "SELECT * FROM userlogin WHERE (email = $1 OR username = $1)";
        pqxx::row r;
        try
        {
          r = tx.exec(query, pqxx::params{identifier}).one_row();
        }
        catch (const std::exception &e)
        {
          return -1;
        }
        return r[0].as<int>();
      }

      /*
       * Helper function
       */
      std::string getUsername(int uuID) {
        pqxx::work tx(*conn);
        std::string query = "SELECT * FROM userlogin WHERE (userid = $1)";
        pqxx::row r;
        try
        {
          r = tx.exec(query, pqxx::params{uuID}).one_row();
        }
        catch (const std::exception &e)
        {
          std::cerr << e.what();
          return "";
        }
        return r[1].c_str();
      }

  public:
    static ConnectorSingleton& getInstance() {
      if (!instance) {
          instance = new ConnectorSingleton();
          try
          {
            #if _WIN64
              conn = new pqxx::connection(
                "host=localhost "
                "dbname=postgres "
                "user=cnath"
              );
            #elif __APPLE__
              conn = new pqxx::connection(
                "host=localhost "
                "dbname=netgrain_db"
              );
            #endif
          }
          catch (const std::exception &e)
          {
            std::cout << "Please launch your psql service\n";
            abort();
          }
      }
      return *instance;
    }

    /*
     * Login: determines if given username/email and password is found in userlogin table
     */
    bool login(std::string identifier, std::string password) {
      pqxx::work tx(*conn);
      std::string query = "SELECT * FROM userlogin WHERE (password = $2) AND (email = $1 OR username = $1)";
      try
      {
        pqxx::row r = tx.exec(query, pqxx::params{identifier, password}).one_row();
      }
      catch (const std::exception &e)
      {
        std::cout << "User not found\n";
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

      try // Check if duplicate username
      {
        std::string query = "SELECT * FROM userlogin WHERE (username = $1)";
        pqxx::result r = tx.exec(query, pqxx::params{username}).no_rows();
      }
      catch (const std::exception &e)
      {

        std::cerr << e.what() << std::endl;
        fmt::print("USERNAME_ALREADY_REGISTERED: {}\n", username);
        return USERNAME_ALREADY_REGISTERED;
      }

      try // Check if duplicate email
      {
        std::string query = "SELECT * FROM userlogin WHERE (email = $1)";
        pqxx::result r = tx.exec(query, pqxx::params{email}).no_rows();
      }
      catch (const std::exception &e)
      {
        fmt::print("EMAIL_ALREADY_REGISTERED: {}\n", email);
        return EMAIL_ALREADY_REGISTERED;
      }

      try // insert new credentials
      {
        std::string query = "INSERT INTO userlogin (username, email, password) VALUES ($3, $1, $2)";
        tx.exec(query, pqxx::params{email, password, username});
        tx.commit();
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }

      fmt::print("SUCCESS\n");
      return SUCCESS;
    }

    /*
     * When user modifies dashboard, changes are updated automatically to userID.file
     * let web server deal with file IO? database will just hold reference
     */
    int linkCustomGUILayout(std::string identifier, std::string path_to_file) {
      int userID = getUUID(identifier);

      if (uuIDfound(userID) == UUID_NOT_FOUND) {
        return UUID_NOT_FOUND;
      }

      pqxx::work tx(*conn);

      try {
        std::string query = "UPDATE userlogin SET customguicomponentlayout = $2 WHERE userid = $1";
        tx.exec(query, pqxx::params{userID, path_to_file});
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
    int userHasCustomLayout(std::string identifier) {
      int userID = getUUID(identifier);

      if (uuIDfound(userID) == -1) {
        fmt::print("UUID NOT FOUND\n");
        return UUID_NOT_FOUND;
      }

      pqxx::work tx(*conn);
      std::string result = "";

      try {
        std::string query = "SELECT * FROM userlogin WHERE userid = $1";
        pqxx::row r = tx.exec(query, pqxx::params{userID}).one_row();
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
    std::string getCustomGUILayout(std::string identifier) {
      int userID = getUUID(identifier);

      if (uuIDfound(userID) == -1) {
        fmt::print("UUID NOT FOUND\n");
        return "";
      }

      pqxx::work tx(*conn);
      std::string result = "";

      try {
        std::string query = "SELECT * FROM userlogin WHERE userid = $1";
        pqxx::row r = tx.exec(query, pqxx::params{userID}).one_row();
        result = r[4].as<std::string>();
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }
      return result;
    }

    // TODO: basic mockup actions of alg
    std::string fetchSimulation(int simID, std::string identifier) {
      int uuid = getUUID(identifier);
      pqxx::work tx(*conn);
      pqxx::row r;
      try
      {
        std::string query = "SELECT * FROM pastSimulations WHERE (userid = $1 AND simid = $2)";
        r = tx.exec(query, pqxx::params{uuid, simID}).one_row();
      }
      catch (const std::exception &e)
      {
        std::cerr << e.what() << "\n";
        return "";
      }
      return r[2].c_str();
    }
    
    int changePassword(std::string identifier, std::string newPassword) {
      int uuID = getUUID(identifier);

      if (uuIDfound(uuID) == -1) {
        fmt::print("UUID_NOT_FOUND");
        return UUID_NOT_FOUND;
      }

      pqxx::work tx(*conn);

      try
      {
        std::string query = "UPDATE userlogin SET password = $1 WHERE (userid = $2)";
        pqxx::result r = tx.exec(query, pqxx::params{newPassword, uuID}).no_rows();
      }
      catch (const std::exception &e)
      {
        return -1;
      }

      tx.commit();
      return SUCCESS;
    }

    // Persistent Change
    // adding again, uuid doesn't exist, 
    // convert simulation time into timestamp
    // simulationTime should be string HH:MM:SS, there are some workarounds needed for other time libraries
    int addLeaderboardAttempt(std::string identiifer, int profit, std::string simulationTime) {
      int uuID = getUUID(identiifer);

      if (uuIDfound(uuID) == -1) {
        fmt::print("UUID_NOT_FOUND");
        return UUID_NOT_FOUND;
      }

      pqxx::work tx(*conn);

      // Check if user already made attempt
      try
      {
        std::string query = "SELECT * FROM leaderboard WHERE (userid = $1)";
        pqxx::result r = tx.exec(query, pqxx::params{uuID}).no_rows();
      }
      catch (const std::exception &e)
      {
        return LEADERBOARD_ATTEMPT_MADE;
      }

      try {
        std::string query = "INSERT INTO leaderboard (userid, profit, simulationtime) VALUES ($1, $2, $3)";
        tx.exec(query, pqxx::params{uuID, profit, simulationTime});
        tx.commit();
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }

      fmt::print("SUCCESS");
      return SUCCESS;
    }

    std::string fetchLayout(std::string identifier) {
      pqxx::work tx(*conn);
      std::string query = "SELECT * FROM leaderboard ORDER BY profit DESC, simulationtime DESC";
      pqxx::result r = tx.exec(query);

      tx.abort();

    }

    /*
     * Currently prints out to console, no testing for this yet
     */
    std::string fetchLeaderBoard() {
      pqxx::work tx(*conn);
      std::string query = "SELECT * FROM leaderboard ORDER BY profit DESC, simulationtime DESC";
      pqxx::result r = tx.exec(query);
      std::string result;

      if (r.affected_rows() == 0) {
        result.append("[]");
        return result;
      }
      tx.abort();

      result.append("[");

      int entry = 1;

      for (auto row = std::begin(r); row != std::end(r); row++) {
        std::string tmp = "{";
        int i = 0;
        for (auto field = std::begin(row); field != std::end(row); field++) {
          if (i == 0) {
            std::stringstream ss;
            ss << entry;
            tmp.append("\"rank\" : " + ss.str());
            tmp.append(", ");
            tmp.append("\"username\" : \"" + getUsername(field.as<int>()) + "\"");
            tmp.append(", ");
          }
          else if (i == 1) {
            tmp.append("\"profit\" : ");
            tmp.append(field->c_str());
            tmp.append(", ");
          }
          else if (i == 2) {
            tmp.append("\"time\" : \"");
            tmp.append(field->c_str());
            tmp.append("\"");
          }
          i++;
        }
        tmp.append("},");
        result.append(tmp);
        entry++;
      }

      if (result.length() > 0) {
        result.pop_back();
      }
      result.append("]");

      return result;
    }

    /*
     * pass in customPresetID = -1 if no global preset was used
     */
    int createSimulation(std::string identifer, std::string analytics, std::string configUsed, int globalPresetID) {
      int uuID = getUUID(identifer);

      if (uuIDfound(uuID) == uuID) {
        return UUID_NOT_FOUND;
      }

      pqxx::work tx(*conn);

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

      try // determine if duplicate global preset exists
      {
        std::string query = "SELECT * FROM globalcustompresets WHERE (scaling = $1) AND (volatility = $2) AND (liquidity = $3) AND (tradingvolume = $4)";
        pqxx::result r = tx.exec(query, pqxx::params{scaling, volatility, liquidity, tradingVol}).no_rows();
      }
      catch (const std::exception &e)
      {
          return DUPLICATE_PRESET_FOUND;
      }

      try {
        std::string query = "INSERT INTO globalcustompresets (scaling, volatility, liquidity, tradingvolume) VALUES ($1, $2, $3, $4)";
        tx.exec(query, pqxx::params{scaling, volatility, liquidity, tradingVol});
      }
      catch (const std::exception &e)
      {
          std::cerr << e.what() << std::endl;
      }
      tx.commit();

      // Grab the global preset ID after insertion
      std::string query = "SELECT * FROM globalcustompresets WHERE (scaling = $1) AND (volatility = $2) AND (liquidity = $3) AND (tradingvolume = $4)";
      pqxx::row r = tx.exec(query, pqxx::params{scaling, volatility, liquidity, tradingVol}).one_row();
      tx.commit();

      return r[0].as<int>();
    }
};

/*
int main() {
  ConnectorSingleton::getInstance().changePassword("ok", "asdfasdf");
}
*/
