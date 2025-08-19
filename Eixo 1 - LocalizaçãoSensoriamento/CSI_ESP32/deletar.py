import matplotlib.pyplot as plt
import numpy as np
import math
import re

num_links = 3
num_subcarriers = 64
values_per_link = num_subcarriers * 2

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

for t, csi in enumerate(csi_samples):
    for link in range(num_links):
        offset = link * values_per_link

        real = csi[offset:offset + values_per_link:2]
        imag = csi[offset + 1:offset + values_per_link:2]
        amp = [math.sqrt(i**2 + r**2) for i, r in zip(imag, real)]

        for sub in range(num_subcarriers):
            amplitudes[link][sub][t] = amp[sub]

time = list(range(num_samples))
colors = plt.cm.jet(np.linspace(0, 1, num_subcarriers)) 

fig, axs = plt.subplots(3, 1, figsize=(14, 10))
fig.suptitle("Amplitude das Subportadoras ao longo do tempo por Link")

for link in range(num_links):
    ax = axs[link]
    for sub in range(num_subcarriers):
        ax.plot(time, amplitudes[link][sub], color=colors[sub], label=f'SC {sub}' if link == 0 else "")
    ax.set_title(f"Link {link}")
    ax.set_xlabel("Tempo (amostra)")
    ax.set_ylabel("Amplitude")
    ax.grid(True)

handles = [plt.Line2D([0], [0], color=colors[i], label=f'SC {i}') for i in range(0, num_subcarriers, 8)]
axs[0].legend(handles=handles, bbox_to_anchor=(1.05, 1), loc='upper left', title="Subportadoras")

plt.tight_layout(rect=[0, 0, 0.85, 0.95])
plt.show()