import matplotlib.pyplot as plt
import numpy as np
import math
import re

num_links = 3
num_subcarriers = 64
values_per_link = num_subcarriers * 2
smooth_phase = True 
window_size = 5     

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
            amp = math.sqrt(imag[sub]**2 + real[sub]**2)
            amplitudes[link][sub][t] = amp
            
            phase = math.degrees(math.atan2(imag[sub], real[sub]))
            phases[link][sub][t] = phase

if smooth_phase:
    phases_smooth = [[[0 for _ in range(num_samples)] for _ in range(num_subcarriers)] for _ in range(num_links)]
    for link in range(num_links):
        for sub in range(num_subcarriers):
            phases_smooth[link][sub] = np.convolve(
                phases[link][sub], 
                np.ones(window_size)/window_size, 
                mode='same'
            )
    phases = phases_smooth

time = list(range(num_samples))
colors = plt.cm.jet(np.linspace(0, 1, num_subcarriers))

fig_amp, axs_amp = plt.subplots(3, 1, figsize=(14, 10))
fig_amp.suptitle("Amplitude das Subportadoras ao longo do tempo por Link")

for link in range(num_links):
    ax = axs_amp[link]
    for sub in range(num_subcarriers):
        ax.plot(time, amplitudes[link][sub], color=colors[sub], alpha=0.6, label=f'SC {sub}' if link == 0 else "")
    ax.set_title(f"Link {link}")
    ax.set_xlabel("Tempo (amostra)")
    ax.set_ylabel("Amplitude")
    ax.grid(True)

handles_amp = [plt.Line2D([0], [0], color=colors[i], label=f'SC {i}') for i in range(0, num_subcarriers, 8)]
axs_amp[0].legend(handles=handles_amp, bbox_to_anchor=(1.05, 1), loc='upper left', title="Subportadoras")

fig_phase, axs_phase = plt.subplots(3, 1, figsize=(14, 10))
fig_phase.suptitle(f"Fase das Subportadoras ao longo do tempo por Link{' (suavizadas)' if smooth_phase else ''}")

for link in range(num_links):
    ax = axs_phase[link]
    for sub in range(num_subcarriers):
        ax.plot(time, phases[link][sub], color=colors[sub], alpha=0.6, label=f'SC {sub}' if link == 0 else "")
    ax.set_title(f"Link {link}")
    ax.set_xlabel("Tempo (amostra)")
    ax.set_ylabel("Fase (graus)")
    ax.grid(True)

handles_phase = [plt.Line2D([0], [0], color=colors[i], label=f'SC {i}') for i in range(0, num_subcarriers, 8)]
axs_phase[0].legend(handles=handles_phase, bbox_to_anchor=(1.05, 1), loc='upper left', title="Subportadoras")

plt.tight_layout()
plt.show()
