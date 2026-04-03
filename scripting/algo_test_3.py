import asyncio
import websockets
import sys
from queue import Queue

# --------------------------------------------------------------------------------
# USER SCRIPT
# --------------------------------------------------------------------------------

def calc_point(price):
  return price - 10

# --------------------------------------------------------------------------------
# USER SCRIPT
# --------------------------------------------------------------------------------

calc_queue = Queue()

async def receive_msg(ws):
  try:
    async for message in ws:
      price_point = float(message)
      result = calc_point(price_point)
      calc_queue.put(result)
      await ws.send(f"point:{result}")
  except websockets.ConnectionClosed:
    print("disconnected")
    sys.exit(0)

# async def send_loop(ws):
#   try:
#     while (True):
#       await asyncio.sleep(8)
#   except websockets.ConnectionClosed:
#     print("disconnected")

async def client_handler():
  url = "ws://localhost:5555/"
  try:
    async with websockets.connect(url) as websocket:
      print("connected to " + url)
      receive_task = asyncio.create_task(receive_msg(websocket))

      # send_task = asyncio.create_task(send_loop(websocket))
      # await asyncio.gather(receive_task, send_task)
      await asyncio.gather(receive_task)
  except Exception:
    print("connection failed")

if __name__ == "__main__":
  try:
    asyncio.run(client_handler())
  except KeyboardInterrupt:
    sys.exit(0)

