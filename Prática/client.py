import socket
import time

def start_udp_client(host='127.0.0.1', port=9999):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    for i in range(20):  # Enviar múltiplas mensagens para simular carga
        message = f"Mensagem {i} para servidor simulado DCCP"
        client_socket.sendto(message.encode(), (host, port))
        print(f"Mensagem {i} enviada para {host}:{port}")
        
        try:
            response, _ = client_socket.recvfrom(1024)
            print(f"Resposta do servidor: {response.decode()}")
        except socket.timeout:
            print(f"Tempo limite excedido para a mensagem {i}")
        
        time.sleep(0.1)  # Simula controle de taxa no cliente
    
    client_socket.close()

if __name__ == "__main__":
    start_udp_client()
