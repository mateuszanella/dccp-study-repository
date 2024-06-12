import socket
import threading
import time

class CongestionControlServer:
    def __init__(self, host='127.0.0.1', port=9999, interval=1.0, max_rate=5):
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.server_socket.bind((host, port))
        self.packet_count = 0
        self.interval = interval
        self.max_rate = max_rate  # Máximo de pacotes por intervalo
        self.lock = threading.Lock()
        self.running = True

    def start(self):
        print(f"Servidor UDP ouvindo em {host}:{port}")
        threading.Thread(target=self.control_congestion).start()
        while self.running:
            data, addr = self.server_socket.recvfrom(1024)
            with self.lock:
                self.packet_count += 1
                if self.packet_count <= self.max_rate:
                    print(f"Dados recebidos de {addr}: {data.decode()}")
                    self.server_socket.sendto(b"Acknowledgment recebido", addr)
                else:
                    print(f"Pacote de {addr} descartado para controle de congestionamento")

    def control_congestion(self):
        while self.running:
            time.sleep(self.interval)
            with self.lock:
                print(f"Pacotes recebidos no último intervalo: {self.packet_count}")
                self.packet_count = 0

    def stop(self):
        self.running = False
        self.server_socket.close()

if __name__ == "__main__":
    host = '127.0.0.1'
    port = 9999
    server = CongestionControlServer(host, port)
    try:
        server.start()
    except KeyboardInterrupt:
        server.stop()
