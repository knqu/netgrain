import '../styling/Leaderboard.css';
import { useState, type ReactElement, useEffect } from 'react';

export default function LeaderboardComponent() {
  const [up, setDirection] = useState<boolean>(false);
  const [entries, setEntries] = useState<Leaderboard_row_entry[]>([
    {"username" : "daniel_luo", profit: 1000000, "rank" : 1, "time" : "00:00:03"},
    {"username" : "Haiyan", profit: 900000, "rank" : 2, "time" : "01:50:45"},
    {"username" : "Kevin", profit: 800000, "rank" : 3, "time" : "02:30:15"},
    {"username" : "Colin", profit: 700000, "rank" : 4, "time" : "10:42:43"},
    {"username" : "James", profit: 600000, "rank" : 5, "time": "15:35:10"},
  ]);

  useEffect(() => {
    const fetchLeaderboard = async () => {
      try {
        const response = await fetch(
          "http://localhost:18080/api/fetchLeaderboard",
          {
            method: "GET",
          }
        );
        if (response.status == 200 && response.body != null) {
          const responseStr = await response.text();
          console.log(responseStr)
          const leaderboardJSON = JSON.parse(responseStr);
          setEntries(leaderboardJSON)
        }
      } catch (err) {
        console.log(err);
      }
    };
    fetchLeaderboard(); 
  }, []);

  type Leaderboard_row_entry = {
    "username" : string;
    "profit" : number;
    "rank": number;
    "time": string;
  }

  function Leaderboard_Row({ player_entry } : {player_entry : Leaderboard_row_entry}) {
    return(
      <div className="row_entry_container">
        <div
          className='row_entry_content'
          style={{
            backgroundColor :
              player_entry.rank === 1 ? '#68E2EB' :
              player_entry.rank === 2 ? '#FBCD37' :
              player_entry.rank === 3 ? '#C7D6DA' :
              'none',
            color :
              player_entry.rank <= 3 ? 'black' : 'white',

          }}
        >
          <div className='content'>
            <h5>{player_entry.rank}</h5>
            <h5>{player_entry.username}</h5>
            <h5>{player_entry.time}</h5>
            <h5>${player_entry.profit}</h5>
          </div>
        </div>
      </div>
    );
  }

  function convertTime(timeStr : string) {
    return (Number(timeStr.substring(0,2)) * 3600) + (Number(timeStr.substring(3,5)) * 60) + (Number(timeStr.substring(6)));
  }

  function Leaderboard_table({ player_entry_list } : { player_entry_list : Leaderboard_row_entry[] }) {
    let tableRows : ReactElement[] = [];
    
    if (up == true) {
      for (let i = 1; i < player_entry_list.length; i++) {
        let item = player_entry_list[i];
        let key = convertTime(item["time"]);
        let j = i - 1;

        while (j >= 0 && convertTime(player_entry_list[j]["time"]) > key) {
          player_entry_list[j + 1] = player_entry_list[j];
          j = j - 1;
        }
        player_entry_list[j + 1] = item;
      }
    }
    else {
      for (let i = 1; i < player_entry_list.length; i++) {
        let item = player_entry_list[i];
        let key = convertTime(item["time"]);
        let j = i - 1;

        while (j >= 0 && convertTime(player_entry_list[j]["time"]) < key) {
          player_entry_list[j + 1] = player_entry_list[j];
          j = j - 1;
        }
        player_entry_list[j + 1] = item;
      }
    }

    player_entry_list.forEach((row) => {
      tableRows.push(
        <Leaderboard_Row player_entry={row} key={row.rank}/>
      );
    });

    return (
      <div className="tableContainer">
        <div className="leaderboard_table_head_row_container">
          <div className="leaderboard_table_head_row">
            <h5>Rank</h5>
            <h5>Type</h5>
            <button className="button_header" onClick={() => {setDirection(!up)}}>Time</button>
            <h5>Profit</h5>
          </div>
        </div>

        <table>
          <tbody>{tableRows}</tbody>
        </table>
      </div>
      
    );
}


  function Leaderboard({entries} : {entries : Leaderboard_row_entry[]}) {
    return (
      <div className='LeaderboardContainer'>
        <div className="Leaderboard">
          <h1 className='Leaderboard_text'>Leaderboard</h1>
          <Leaderboard_table player_entry_list={entries}></Leaderboard_table>
        </div>
      </div>
    );
  }
  return (
    <Leaderboard entries={entries}/>
  );
};

/*const testData = [
    {username : "daniel_luo", profit: 1000000, rank : 1, time : "00:00:03"},
    {username : "Haiyan", profit: 900000, rank : 2, time : "01:50:45"},
    {username : "Kevin", profit: 800000, rank : 3, time : "02:30:15"},
    {username : "Colin", profit: 700000, rank : 4, time : "10:42:43"},
    {username : "James", profit: 600000, rank : 5, time: "15:35:10"},
  ];*/
