/* This file serves as the basis for the algorithms used for
 * the metrics in the post sim page
 */


 // winrate: profitable trades / total trades * 100
 
double winrate(int winning_trades, int total) {
  return winning_trades / total;
}

 // risk-reward: avg. win / avg. loss
double risk_reward(double avg_win, double avg_loss) {
  return avg_win / avg_loss;
}
 // max draw-down (peak - trough) / peak * 100
double max_draw_down(double peak, double trough) {
  return (peak - trough) / peak * 100;
}

// profit factor gross profit / gross loss
double profit_factor(double profit, double loss) {
  return profit / loss;
}

// sharpe ratio (return - risk-free-rate) / sd dev
double sharpe_ratio(double ret, double risk_rate, double s_dev) {
  return (ret - risk_rate) / s_dev;
}