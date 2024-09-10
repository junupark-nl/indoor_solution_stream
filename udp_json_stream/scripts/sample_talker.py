import socket
import json
import argparse
import time

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument('--ip', type=str, default='127.0.0.1', help='IP address to listen on')
arg_parser.add_argument('--port', type=int, default=5005, help='Port to listen on')

def talker(server_address: tuple):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    while True:
        message = {
            'message': 'Hello, world!',
            'number': 37,
            'timestamp': time.time(),
            'nested': {
                'nestedness': True,
                'data': [1, 2, 3, 4, 5]
            }
        }    
        
        try:
            sent = sock.sendto(json.dumps(message).encode('utf-8'), server_address)
            print(f'Sent {sent} bytes')
        except Exception as e:
            print(f'Unknown error: {e}')

if __name__=="__main__":
    args = arg_parser.parse_args()
    talker((args.ip, args.port))