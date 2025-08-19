#Define o servidor TCP (escutando em qualquer IP na porta 1234)
#Aguarda alguma conexão ser ativada (escutando)
#Recebe os dados via TCP
#Salva os valores com o tempo real de recebimento em um arquivo .txt
import socket
import datetime


HOST = '0.0.0.0'
PORT = 1234
ARQUIVO_SAIDA = "dados_rssi.txt"


def salvar_dado(dado):
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(ARQUIVO_SAIDA, "a") as f:
        f.write(f"{timestamp};{dado}\n")


def servidor_tcp():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen(1)
        print(f"Servidor TCP escutando em {HOST}:{PORT}...")
        conn, addr = s.accept()
        print(f"Conectado por {addr}")


        with conn:
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                dado_recebido = data.decode(errors='ignore').strip()
                print(dado_recebido)
                salvar_dado(dado_recebido)


if __name__ == "__main__":
    servidor_tcp()
