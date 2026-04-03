import asyncio
import websockets
import sys
from queue import Queue
import atexit

# --------------------------------------------------------------------------------
# USER SCRIPT
# --------------------------------------------------------------------------------

def calc_point(price):
  return price + 100

class Simulation:
  def __init__(self, drift, volatility, price, target):
    self.drift = drift
    self.volatility = volatility
    self.price = price
    self.target = target

simulations = [
  Simulation(0.2, 0.3, 100, 150),
  Simulation(0.4, 0.4, 1000, 100),
  Simulation(0.1, 0.1, 500, 300),
  Simulation(0.1, 0.2, 800, 550),
]

# --------------------------------------------------------------------------------
# USER SCRIPT
# --------------------------------------------------------------------------------

sim_queues = []
calc_result_bufs = []

for i in range(0, len(simulations)):
  q = Queue()
  sim_queues.append(q)

for i in range(0, len(simulations)):
  calc_result_bufs.append([])

async def receive_msg(ws):
  try:
    async for message in ws:
      substrs = message.split(":")
      id = int(substrs[0])
      price_point = float(substrs[1])
      calculated = calc_point(price_point)
      sim_queues[id - 1].put(calculated)
      calc_result_bufs[id - 1].append(calculated)
  except websockets.ConnectionClosed:
    print("disconnected")

# async def send_loop(ws):
#   try:
#     while (True):
#       await asyncio.sleep(8)
#   except websockets.ConnectionClosed:
#     print("disconnected")

async def print_loop():
  print_str = ""
  for i, _ in enumerate(sim_queues):
    print_str += f"SIM EVENT {i:>2}"
    if i != len(sim_queues) - 1:
      print_str += " | "
  print(print_str)

  while (True):
    await asyncio.sleep(0.5)

    print_str = ""
    for i, sim_q in enumerate(sim_queues):
      print_str += f"{sim_q.get():>12.4f}"
      if i != len(sim_queues) - 1:
        print_str += " | "
    print(print_str)

async def client_handler():
  url = "ws://localhost:5555/"
  try:
    async with websockets.connect(url) as websocket:
      print("connected to " + url)

      for sim in simulations:
        await asyncio.sleep(0.6)
        await websocket.send(f"sim:{sim.drift}!{sim.volatility}!{sim.price}!{sim.target}")

      receive_task = asyncio.create_task(receive_msg(websocket))

      # send_task = asyncio.create_task(send_loop(websocket))
      # await asyncio.gather(receive_task, send_task)
      await asyncio.gather(receive_task, print_loop())
  except Exception:
    print("connection failed")

@atexit.register
def print_metrics():
  print()
  if (len(sim_queues) != 0):
    print('-' * (12 * len(sim_queues) + 3 * (len(sim_queues)) - 1))
    print_str = ""
    avgs_sum = 0.0
    for i, buf in enumerate(calc_result_bufs):
      sum = 0.0
      for point in buf:
        sum += point
      avg = sum / len(buf)
      avg = avg / buf[0]
      print_str += f"{avg:>12.4f}"
      if i != len(sim_queues) - 1:
        print_str += " | "
      avgs_sum += avg
    print(print_str)
    print(f"AVERAGE PERF: {avgs_sum / len(calc_result_bufs)}")

if __name__ == "__main__":
  try:
    asyncio.run(client_handler())
  except KeyboardInterrupt:
    sys.exit(0)

