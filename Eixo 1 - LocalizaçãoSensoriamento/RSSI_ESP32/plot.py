#Abre o arquivo .txt com os dados RSSI
#Plota um gráfico de valores RSSI ao longo do tempo
import matplotlib.pyplot as plt
import re


ARQUIVO_SAIDA = "C:\\Users\\Bernardo\\Scan_wifi\\dados_rssi.txt"


def extrair_rssi(caminho):
    rssi = []
    with open(caminho, "r") as f:
        for linha in f:
            match = re.search(r'RSSI:\s*(-?\d+)', linha)
            if match:
                rssi.append(int(match.group(1)))
    return rssi


def plotar_rssi(rssi):
    plt.plot(rssi, marker='o')
    plt.title("RSSI ao longo do tempo")
    plt.xlabel("Tempo (amostras)")
    plt.ylabel("RSSI (dBm)")
    plt.axhline(y=-50, color='red', linestyle='--', label='Threshold -50 dBm')  # Linha horizontal em -50
    plt.grid(True)
    plt.gca().invert_yaxis()  # RSSI mais forte = mais negativo
    plt.show()


if __name__ == "__main__":
    rssi = extrair_rssi(ARQUIVO_SAIDA)
    if rssi:
        plotar_rssi(rssi)
    else:
        print("Nenhum dado RSSI encontrado.")
