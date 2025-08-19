import matplotlib.pyplot as plt
import numpy as np
import math
import re

num_links = 3
num_subcarriers = 64
values_per_link = num_subcarriers * 2
smooth_phase = True
window_size = 5   

subportadoras_para_plotar = [0, 44]  

def ler_dados_csi(arquivo):
    csi_samples = []
    with open(arquivo, 'r') as f:
        for linha in f:
            if "CSI_DATA" in linha:
                match = re.search(r'\[(.*?)\]', linha)
                if match:
                    dados_str = match.group(1)
                    dados = list(map(int, dados_str.split()))
                    csi_samples.append(dados)
    return csi_samples

csi_samples = ler_dados_csi('dados_csi.txt')
num_samples = len(csi_samples)

amplitudes = [[[0 for _ in range(num_samples)] for _ in range(num_subcarriers)] for _ in range(num_links)]
phases = [[[0 for _ in range(num_samples)] for _ in range(num_subcarriers)] for _ in range(num_links)]

for t, csi in enumerate(csi_samples):
    for link in range(num_links):
        offset = link * values_per_link
        real = csi[offset:offset + values_per_link:2]
        imag = csi[offset + 1:offset + values_per_link:2]
        
        for sub in range(num_subcarriers):
            amplitudes[link][sub][t] = math.sqrt(imag[sub]**2 + real[sub]**2)
            phases[link][sub][t] = math.degrees(math.atan2(imag[sub], real[sub]))

if smooth_phase:
    for link in range(num_links):
        for sub in range(num_subcarriers):
            phases[link][sub] = np.convolve(phases[link][sub], np.ones(window_size)/window_size, mode='same')

time = list(range(num_samples))
cores = plt.cm.jet(np.linspace(0, 1, len(subportadoras_para_plotar))) 

fig_amp, axs_amp = plt.subplots(3, 1, figsize=(14, 10))
fig_amp.suptitle(f"Amplitude das Subportadoras {subportadoras_para_plotar} por Link")

for link in range(num_links):
    ax = axs_amp[link]
    for idx, sub in enumerate(subportadoras_para_plotar):
        ax.plot(time, amplitudes[link][sub], color=cores[idx], label=f'SC {sub}')
    ax.set_title(f"Link {link}")
    ax.set_xlabel("Tempo (amostra)")
    ax.set_ylabel("Amplitude")
    ax.legend()
    ax.grid(True)

fig_phase, axs_phase = plt.subplots(3, 1, figsize=(14, 10))
fig_phase.suptitle(f"Fase das Subportadoras {subportadoras_para_plotar} por Link{' (suavizadas)' if smooth_phase else ''}")

for link in range(num_links):
    ax = axs_phase[link]
    for idx, sub in enumerate(subportadoras_para_plotar):
        ax.plot(time, phases[link][sub], color=cores[idx], label=f'SC {sub}')
    ax.set_title(f"Link {link}")
    ax.set_xlabel("Tempo (amostra)")
    ax.set_ylabel("Fase (graus)")
    ax.legend()
    ax.grid(True)

plt.tight_layout()
plt.show()