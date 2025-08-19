#include "freertos/FreeRTOS.h" //Núcleo do sistema operacional FreeRTOS
#include "freertos/task.h" //Criação de tarefas
#include "freertos/event_groups.h" //Suporte a grupos de eventos
#include "esp_wifi.h" //Biblioteca de manipulação wifi
#include "esp_log.h" //Mensagens Logs
#include "esp_event.h" //Eventos do sistema
#include "nvs_flash.h" //Memória não volátil
#include "driver/gpio.h" //Pinos GPIO
#include "lwip/sockets.h" //Sockets TCP/IP
#include "lwip/netdb.h" //Auxiliadores de rede
#include "string.h" //String


#define DEFAULT_SSID "Bernardo.2G" //Nome da rede wifi
#define DEFAULT_PWD "coruja2002" //Senha da rede wifi


#define SERVER_IP "192.168.0.3"  //IP do servidor (PC)
#define SERVER_PORT 1234 //Porta para conexão TCP


static const char *TAG = "socket_sender"; //TAG para logs do ESP-IDF
int sock = -1; //Descritor do socket


void send_rssi_to_server(int rssi) {
    if (sock < 0) return; //Se o sockeet não foi criado, sai da função


    char msg[64];
    snprintf(msg, sizeof(msg), "RSSI: %d dBm\n", rssi); //Formata a string a ser enviada
    send(sock, msg, strlen(msg), 0); //Envia mensagem ao servidor
}


void MotionDetector(void *param){ //Função para monitorar a força do sinal WIFI
    while (1) {
        wifi_ap_record_t ap; //Informações do AP conectado
        esp_err_t err = esp_wifi_sta_get_ap_info(&ap); //Obtem dados da coneexão atual
        if (err == ESP_OK) {
            int strength = ap.rssi; //Força de sinal (RSSI)
            printf("Wi-Fi Signal Strength: %d dBm\n", strength);
            gpio_set_level(GPIO_NUM_19, strength < -50 ? 1 : 0); //Acende o LED caso a força de sinal caia para menos de -50dBm (decibel-miliwatt)


            send_rssi_to_server(strength);  // Envia o valor via socket TCP
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); //Delay de 2 segundos
    }
}


static void event_handler(void* arg, esp_event_base_t event_base, //Função para tratar eventos WIFI e IP
                          int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect(); //conecta ao wifi ao iniciar
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect(); //Reconecta ao wifi caso perca o sinal
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));


        // Ao obter o IP Conecta ao servidor TCP
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(SERVER_PORT);


        //Cria o socket TCP
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            return;
        }
        //Tenta conectar ao servidor TCP
        if (connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            close(sock);
            sock = -1;
            return;
        }


        ESP_LOGI(TAG, "Successfully connected to server");
        xTaskCreate(&MotionDetector, "MotionDetector", 4096, NULL, 5, NULL);//Cria a tarefa para detecção da força de sinal
    }
}


void fast_scan(void) {//Inicializa o Wifi no modo estação
    ESP_ERROR_CHECK(esp_netif_init());//Inicializa a interface de rede
    ESP_ERROR_CHECK(esp_event_loop_create_default()); //Cria loop de eventos padrão


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); //Inicializa o drive WIFI


    //Registro manipuladores de eventos para WIFI e IP
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));


    esp_netif_create_default_wifi_sta();//Cria interface STA (cliente)


    //Define SSID e senha para conexão na rede
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = DEFAULT_SSID,
            .password = DEFAULT_PWD,
        },
    };


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); //Modo cliente
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));//Aplica a configuração
    ESP_ERROR_CHECK(esp_wifi_start());//Inicia o WIFI
}


void app_main(void) {//Função Principal
    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);//Define o pino 19 como saída
    esp_err_t ret = nvs_flash_init();//Inicializa a memória não volátil usada pelo WIFI
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());//Apaga se estiver cheio ou desatualizado
        ret = nvs_flash_init();//Tenta novamente
    }
    ESP_ERROR_CHECK(ret);


    fast_scan();//Inicializa o processo de conexão
}
//Esp conecta-se ao WIFI
//Ao obter um IP, se conecta ao servidor TCP (PC)
//"Pipeline" duas execuções simultâneas (main e rssi detector) - Dual core
//Função detector, coleta o valor do RSSI e ativa um led quando o sinal está fraco
//Envia os valores para o servidor via TCP
