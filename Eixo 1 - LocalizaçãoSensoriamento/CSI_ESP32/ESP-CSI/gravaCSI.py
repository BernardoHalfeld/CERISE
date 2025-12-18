import serial
import time

# --- Configure aqui ---
PORTA_SERIAL = "COM3"  # Ajuste conforme sua porta
BAUD_RATE = 921600     # Deve ser o mesmo configurado no ESP32 (monitor_speed)
DURACAO_CAPTURA = 30   # Segundos
ARQUIVO_SAIDA = "coleta_csi2.csv"
# ----------------------

print(f"Iniciando captura de {DURACAO_CAPTURA} segundos...")
print(f"Ouvindo a porta: {PORTA_SERIAL}")

try:
    ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=1)
    ser.flushInput()

    # (Opcional) Cria o cabeçalho manualmente se soubermos as colunas, 
    # pois as vezes perdemos o cabeçalho original do boot do ESP32.
    # cabecalho = "type,id,mac,rssi,rate,sig_mode,mcs,bandwidth,smoothing,not_sounding,aggregation,stbc,fec_coding,sgi,noise_floor,ampdu_cnt,channel,secondary_channel,local_timestamp,ant,sig_len,rx_state,len,first_word,data\n"

    with open(ARQUIVO_SAIDA, 'w', encoding='utf-8') as f: # Usei 'w' (texto) em vez de 'wb'
        # f.write(cabecalho) # Descomente se quiser forçar o cabeçalho
        
        tempo_inicio = time.time()
        
        while (time.time() - tempo_inicio) < DURACAO_CAPTURA:
            try:
                # Lê a linha e decodifica para texto
                linha_bytes = ser.readline()
                try:
                    linha_texto = linha_bytes.decode('utf-8', errors='ignore').strip()
                except:
                    continue # Ignora erros de decodificação

                if linha_texto:
                    # Imprime tudo no terminal para você ver o que está acontecendo
                    print(linha_texto) 

                    # FILTRO: Só salva no arquivo se for dados de CSI
                    if linha_texto.startswith("CSI_DATA"):
                        f.write(linha_texto + "\n")
                        f.flush() # Garante que grave no disco imediatamente
                    
            except serial.SerialException as e:
                print(f"Erro de conexão serial: {e}")
                break
                
except serial.SerialException as e:
    print(f"ERRO CRÍTICO: Não foi possível abrir {PORTA_SERIAL}.")
    print("Verifique se o monitor serial do VS Code ou IDF não está aberto prendendo a porta.")
except KeyboardInterrupt:
    print("\nCaptura interrompida manualmente.")

finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
    print(f"Captura finalizada. Verifique o arquivo: {ARQUIVO_SAIDA}")