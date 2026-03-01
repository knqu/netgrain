import '../styling/Leaderboard.css';
import React, { type ReactElement } from 'react';

type Leaderboard_row_entry = {
  username : string;
  profit : number;
  rank: number;
}

function Leaderboard_Row({ player_entry } : {player_entry : Leaderboard_row_entry}) {
  return(
    <tr className="row_entry_container">
      <td
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
        <div className='left_content'>
          <h5>{player_entry.rank}</h5>
          <h5>{player_entry.username}</h5>
        </div>
        <div className='right_content'>
          <h5 >${player_entry.profit}</h5>
        </div>
      </td>
    </tr>
  );
}

function Leaderboard_table({ player_entry_list } : { player_entry_list : Leaderboard_row_entry[] }) {
  let tableRows : ReactElement[] = [];
  player_entry_list.forEach((row) => {
    tableRows.push(
      <Leaderboard_Row player_entry={row} key={row.rank}/>
    );
  });

  return (
    <table>
      <tbody>{tableRows}</tbody>
    </table>
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

export default function LeaderboardComponent() {
  return <Leaderboard entries={testData}/>
};

const testData = [
  {username : "daniel_luo", profit: 1000000, rank : 1},
  {username : "Haiyan", profit: 900000, rank : 2},
  {username : "Kevin", profit: 800000, rank : 3},
  {username : "Colin", profit: 700000, rank : 4},
  {username : "James", profit: 600000, rank : 5},
];
