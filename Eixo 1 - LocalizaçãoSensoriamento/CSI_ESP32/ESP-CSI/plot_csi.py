import matplotlib.pyplot as plt
import numpy as np
import math
import csv
import re

# Configurações do CSI
num_links = 1
num_subcarriers = 64
values_per_link = num_subcarriers * 2   # 64 real + 64 imag

smooth_phase = True
window_size = 5


# ------------------------------
# LER DADOS DO CSV
# ------------------------------
def ler_dados_csi(arquivo_csv):
    csi_samples = []

    with open(arquivo_csv, 'r') as f:
        reader = csv.reader(f)

        for row in reader:
            if len(row) == 0:
                continue

            if row[0] == "CSI_DATA":
                vetor_str = row[-1]

                match = re.search(r'\[(.*?)\]', vetor_str)
                if match:
                    valores = match.group(1).split(',')
                    valores = list(map(int, valores))
                    csi_samples.append(valores)

    return csi_samples


# ------------------------------
# CARREGAR OS DADOS
# ------------------------------
csi_samples = ler_dados_csi('coleta_csi2.csv')
num_samples = len(csi_samples)

print("Amostras carregadas:", num_samples)
print("Exemplo de vetor CSI:", csi_samples[0][:20], "...")
print("Total de valores no vetor:", len(csi_samples[0]))


# Matrizes amplitude e fase
amplitudes = [[ [0 for _ in range(num_samples)] for _ in range(num_subcarriers) ]]
phases = [[ [0 for _ in range(num_samples)] for _ in range(num_subcarriers) ]]


# ------------------------------
# PROCESSAMENTO DOS DADOS
# Ignorar subportadora 0
# ------------------------------
for t, csi in enumerate(csi_samples):

    real = csi[0:values_per_link:2]      # 64 reais
    imag = csi[1:values_per_link:2]      # 64 imag

    # Ignorar a subportadora 0
    for sub in range(1, num_subcarriers):
        ampl = math.sqrt(real[sub]**2 + imag[sub]**2)
        amplitudes[0][sub][t] = ampl

        fase = math.degrees(math.atan2(imag[sub], real[sub]))
        phases[0][sub][t] = fase


# ------------------------------
# SUAVIZAÇÃO DA FASE
# ------------------------------
if smooth_phase:
    for sub in range(1, num_subcarriers):   # também ignora o 0
        phases[0][sub] = np.convolve(
            phases[0][sub],
            np.ones(window_size)/window_size,
            mode='same'
        )


# ------------------------------
# PLOTS
# ------------------------------
time = list(range(num_samples))
colors = plt.cm.jet(np.linspace(0, 1, num_subcarriers))


# ------------------------------
# PLOT AMPLITUDE
# ------------------------------
plt.figure(figsize=(14, 6))
for sub in range(1, num_subcarriers):     # ignora no plot
    plt.plot(time, amplitudes[0][sub], color=colors[sub], alpha=0.6)

plt.title("Amplitude CSI (1 Link – 63 Subportadoras — subportadora 0 ignorada)")
plt.xlabel("Amostra")
plt.ylabel("Amplitude")
plt.grid(True)
plt.tight_layout()
plt.show()


# ------------------------------
# PLOT FASE
# ------------------------------
plt.figure(figsize=(14, 6))
for sub in range(1, num_subcarriers):     # ignora no plot
    plt.plot(time, phases[0][sub], color=colors[sub], alpha=0.6)

plt.title("Fase CSI (1 Link – 63 Subportadoras — subportadora 0 ignorada)")
plt.xlabel("Amostra")
plt.ylabel("Fase (graus)")
plt.grid(True)
plt.tight_layout()
plt.show()
