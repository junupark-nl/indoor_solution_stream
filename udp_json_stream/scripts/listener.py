import os
import datetime
import socket
import csv
import json
import argparse

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument('--ip', type=str, default='127.0.0.1', help='IP address to listen on')
arg_parser.add_argument('--port', type=int, default=5005, help='Port to listen on')
class JsonLogger:
    def __init__(self):
        now = datetime.datetime.now()
        filepath = os.path.join(os.path.dirname(__file__), "..", "logs", f"{now.strftime('%Y-%m-%d')}")
        if not os.path.exists(filepath):
            os.makedirs(filepath)

        self.filename = os.path.join(filepath, f"{now.strftime('%H;%M;%S')}.csv")
        self.initialized = False
        self.fieldnames = None
    
    def initialize(self, msg):
        self.fieldnames = self.flatten(msg).keys()
        with open(self.filename, "w", newline='') as file:
            writer = csv.DictWriter(file, fieldnames=self.fieldnames)
            writer.writeheader()
        self.initialized = True
    
    def log(self, msg):
        flattened_msg = self.flatten(msg)
        if not self.initialized:
            self.initialize(flattened_msg)
        with open(self.filename, "a", newline='') as file:
            self.writer = csv.DictWriter(file, fieldnames=self.fieldnames)
            self.writer.writerow(flattened_msg)
    
    def flatten(self, msg, parent_key='', sep='.'):
        items = []
        for k, v in msg.items():
            new_key = f"{parent_key}{sep}{k}" if parent_key else k
            if isinstance(v, dict):
                items.extend(self.flatten(v, new_key, sep=sep).items())
            if isinstance(v, list):
                items.append((new_key, json.dumps(v)))
            else:
                items.append((new_key, v))
        return dict(items)

def listener(server_address: tuple):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(server_address)
    
    logger = JsonLogger()
    print("hi")
    
    try:
        while True:
            data, address = sock.recvfrom(4096)
            
            try:
                message = json.loads(data.decode('utf-8'))
                print("Received JSON message:")
                print(json.dumps(message, indent=2))
                logger.log(message)
            except json.JSONDecodeError:
                print('Invalid JSON')
            except UnicodeDecodeError:
                print('Invalid UTF-8')
            except Exception as e:
                print(f'Unknown error: {e}')
    except KeyboardInterrupt:
        print("Shutting down...")
    finally:
        sock.close()
        
if __name__=="__main__":
    args = arg_parser.parse_args()
    listener((args.ip, args.port))