import asyncio
import logging
import warnings
import re
import time
from datetime import datetime
from amqtt.broker import Broker

# 1. Disable warnings
warnings.filterwarnings("ignore")

# Helper function to get standardized timestamp
def get_timestamp():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# 2. PROFESSIONAL LOG HANDLER
class ProfessionalLogHandler(logging.StreamHandler):
    def __init__(self):
        super().__init__()
        self.last_disconnect_time = 0

    def emit(self, record):
        msg = record.getMessage()
        current_time = time.time()
        ts = get_timestamp()
        
        # 1. CONNECT
        if "ConnectVariableHeader(client_id=" in msg:
            match = re.search(r"client_id=([^,]+)", msg)
            if match:
                print(f"[{ts}] [CONNECT] Client '{match.group(1)}' established connection.")
                
        # 2. SUBSCRIBE
        elif "handling subscription" in msg:
            client_id = msg.split(" handling subscription")[0].strip().split()[-1]
            print(f"[{ts}] [SUBSCRIBE] Client '{client_id}' registered for topic updates.")
            
        # 3. TELEMETRY DATA
        elif "Processing broadcast message:" in msg and "clientId=" in msg and "data':" in msg:
            try:
                client_id = re.search(r"clientId=([^,]+)", msg).group(1)
                data_match = re.search(r"data': b'(.+?)'}", msg)
                if data_match:
                    print(f"[{ts}] [TELEMETRY] Client '{client_id}' -> Payload: {data_match.group(1)}")
            except:
                pass
                
        # 4. DISCONNECT (Graceful)
        elif "_on_enter_disconnected" in msg and "clientId=" in msg:
            match = re.search(r"clientId=([^,]+)", msg)
            if match:
                print(f"[{ts}] [DISCONNECT] Client '{match.group(1)}' disconnected gracefully.")
                self.last_disconnect_time = current_time
        
        # 5. ERROR / LOST CONNECTION
        elif "ConnectionAbortedError" in msg or "ConnectionResetError" in msg or "Connection lost" in msg:
            if current_time - self.last_disconnect_time > 1.0:
                print(f"[{ts}] [ERROR] A client connection was dropped unexpectedly.")
                self.last_disconnect_time = current_time

# SETTING UP THE LOGGER
logger = logging.getLogger("amqtt")
logger.handlers = []
logger.propagate = False
logger.setLevel(logging.DEBUG) 
logger.addHandler(ProfessionalLogHandler())

broker_config = {
    'listeners': {
        'default': {
            'type': 'tcp',
            'bind': '0.0.0.0:1883'
        }
    }
}

async def broker_coro():
    broker = Broker(broker_config)
    await broker.start()
    print("=" * 70)
    print(f"[{get_timestamp()}] [SYSTEM] LOCAL MQTT BROKER STARTED ON PORT 1883")
    print(f"[{get_timestamp()}] [SYSTEM] Awaiting publisher/subscriber connections...")
    print("=" * 70)

def start_broker():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    try:
        loop.run_until_complete(broker_coro())
        loop.run_forever()
    except KeyboardInterrupt:
        print(f"\n[{get_timestamp()}] [SYSTEM] Broker shut down safely.")

if __name__ == "__main__":
    start_broker()