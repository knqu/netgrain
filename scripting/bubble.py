import asyncio
import websockets
import sys

async def create_flash_crash(ws):
  await ws.send("flash_crash")

async def create_bubble(ws, threshold):
  await ws.send(f"bubble:{threshold}")

async def receive_msg(ws):
  try:
    async for message in ws:
      print(message)
  except websockets.ConnectionClosed:
    print("disconnected")

async def send_loop(ws):
  try:
    while (True):
      await asyncio.sleep(2)

      await create_bubble(ws, 240)
      print("EVENT CALLED: bubble")
      await create_bubble(ws, 300)
      print("EVENT CALLED: bubble")

      await asyncio.sleep(40)
  except websockets.ConnectionClosed:
    print("disconnected")

async def client_handler():
  url = "ws://localhost:5555/"
  try:
    async with websockets.connect(url) as websocket:
      print("connected to " + url)
      receive_task = asyncio.create_task(receive_msg(websocket))
      send_task = asyncio.create_task(send_loop(websocket))

      await asyncio.gather(receive_task, send_task)
  except Exception:
    print("connection failed")

if __name__ == "__main__":
  try:
    asyncio.run(client_handler())
  except KeyboardInterrupt:
    sys.exit(0)

