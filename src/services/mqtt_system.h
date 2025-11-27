/**
 * @file mqtt_system.h
 * @brief Interface pública do sistema MQTT IoT para ESP32.
 *
 * Define estruturas de dados, funções de inicialização e APIs
 * para publicação de telemetria e monitoramento.
 *
 * @author Moacyr Francischetti Correa
 * @date 2025
 */

#ifndef MQTT_SYSTEM_H
#define MQTT_SYSTEM_H

/* Includes */
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/* Configurações e definições públicas */

/**
 * Configurações padrão do sistema.
 * Podem ser sobrescritas via menuconfig.
 */
#ifndef CONFIG_WIFI_SSID
#define CONFIG_WIFI_SSID "iot"
#endif

#ifndef CONFIG_WIFI_PASSWORD
#define CONFIG_WIFI_PASSWORD "123mudar"
#endif

#ifndef CONFIG_MQTT_BROKER_URI
// nao lembro a rota certa 
#define CONFIG_MQTT_BROKER_URI "mqtt://192.168.137.1:1883"
#endif

#ifndef CONFIG_MQTT_CLIENT_ID
#define CONFIG_MQTT_CLIENT_ID "esp32_device_001"
#endif

#ifndef CONFIG_MQTT_USERNAME
#define CONFIG_MQTT_USERNAME ""
#endif

#ifndef CONFIG_MQTT_PASSWORD
#define CONFIG_MQTT_PASSWORD ""
#endif

/* Comportamento do sistema */
#define MQTT_KEEPALIVE_SEC 60				 ///< Intervalo de keep-alive MQTT
#define MQTT_BUFFER_SIZE 2048				 ///< Tamanho do buffer MQTT
#define MQTT_TIMEOUT_MS 10000				 ///< Timeout de operações MQTT
#define WIFI_MAX_RETRY 5					 ///< Tentativas de reconexão WiFi
#define TELEMETRY_INTERVAL_MS 1000		 ///< Intervalo de telemetria
#define HEALTH_CHECK_INTERVAL_MS 60000	 ///< Intervalo de health check
#define WIFI_WATCHDOG_INTERVAL_MS 30000 ///< Intervalo de verificação WiFi

/* Tipos e estruturas */

/**
 * @brief Estatísticas de operação MQTT.
 */
typedef struct
{
	uint32_t total_publicadas;		  ///< Total de mensagens publicadas.
	uint32_t total_recebidas;		  ///< Total de mensagens recebidas.
	uint32_t falhas_publicacao;	  ///< Número de falhas na publicação.
	uint32_t desconexoes;			  ///< Contador de desconexões.
	uint32_t tempo_desconectado_ms; ///< Tempo total desconectado (ms).
	uint32_t ultima_mensagem_ts;	  ///< Timestamp da última mensagem (ms).
} mqtt_statistics_t;

/**
 * @brief Níveis de Qualidade de Serviço (QoS) MQTT.
 */
typedef enum
{
	MQTT_QOS_0 = 0, ///< Sem confirmação.
	MQTT_QOS_1 = 1, ///< Confirmação obrigatória.
	MQTT_QOS_2 = 2	 ///< Handshake completo.
} mqtt_qos_level_t;

/**
 * @brief Dados de telemetria de sensores.
 */
typedef struct
{
	float temperatura;  ///< Temperatura em °C.
	float umidade;		  ///< Umidade relativa (%).
	uint32_t contador;  ///< Contador de amostras.
	uint64_t timestamp; ///< Timestamp da leitura (ms).
} telemetry_data_t;

/**
 * @brief Métricas de saúde do sistema.
 */
typedef struct
{
	uint32_t free_heap;		///< Memória heap livre (bytes).
	uint32_t min_free_heap; ///< Menor heap livre desde o boot (bytes).
	int wifi_rssi;				///< Força do sinal WiFi (dBm).
	uint64_t uptime_sec;		///< Tempo de atividade (segundos).
	bool mqtt_connected;		///< Status da conexão MQTT.
} health_status_t;

/* Funções de Inicialização e Controle */

/**
 * @brief Inicializa o sistema IoT MQTT completo.
 * @return ESP_OK se sucesso, erro caso contrário.
 * @note Bloqueia até o WiFi conectar ou timeout. Deve ser chamada uma única vez.
 */
esp_err_t mqtt_system_init(void);

/**
 * @brief Desliga o sistema MQTT e libera recursos.
 * @return ESP_OK se sucesso, erro caso contrário.
 */
esp_err_t mqtt_system_shutdown(void);

/**
 * @brief Verifica o status da conexão MQTT.
 * @return true se conectado, false caso contrário.
 */
bool mqtt_system_is_connected(void);

/* Funções de Publicação MQTT */

/**
 * @brief Publica dados em um tópico MQTT.
 * @param topic Tópico de destino.
 * @param data Dados a publicar.
 * @param len Comprimento dos dados (0 para string).
 * @param qos Nível de QoS (0, 1, 2).
 * @param retain Se a mensagem deve ser retida pelo broker.
 * @return ID da mensagem ou -1 em caso de erro.
 * @note Retorna erro se o MQTT não estiver conectado.
 */
int mqtt_publish_data(const char *topic, const char *data,
							 int len, int qos, bool retain);

/**
 * @brief Publica dados de telemetria (temperatura, umidade, contador, timestamp).
 * @param data Estrutura com os dados de telemetria.
 * @return ID da mensagem ou -1 em caso de erro.
 */
int mqtt_publish_telemetry(const telemetry_data_t *data);

/**
 * @brief Publica as métricas de saúde do sistema (heap, RSSI, uptime, etc.).
 * @return ID da mensagem ou -1 em caso de erro.
 */
int mqtt_publish_health_check(void);

/**
 * @brief Publica o status online/offline do dispositivo.
 * @param online true para status "online", false para "offline".
 * @return ID da mensagem ou -1 em caso de erro.
 */
int mqtt_publish_status(bool online);

/* Funções de Subscrição MQTT */

/**
 * @brief Subscreve em um tópico MQTT.
 * @param topic Tópico para subscrever (suporta wildcards).
 * @param qos Nível de QoS desejado.
 * @return ID da mensagem ou -1 em caso de erro.
 */
int mqtt_subscribe_topic(const char *topic, int qos);

/**
 * @brief Cancela a subscrição de um tópico MQTT.
 * @param topic Tópico para cancelar a subscrição.
 * @return ID da mensagem ou -1 em caso de erro.
 */
int mqtt_unsubscribe_topic(const char *topic);

/* Funções de Estatísticas e Monitoramento */

/**
 * @brief Obtém as estatísticas atuais do sistema MQTT.
 * @param stats Ponteiro para a estrutura onde as estatísticas serão copiadas.
 * @return ESP_OK se sucesso, ESP_ERR_INVALID_ARG se `stats` for NULL.
 */
esp_err_t mqtt_get_statistics(mqtt_statistics_t *stats);

/**
 * @brief Reseta os contadores de estatísticas MQTT.
 * Zera todos os contadores, exceto desconexões e tempo desconectado.
 */
void mqtt_reset_statistics(void);

/**
 * @brief Obtém o status de saúde atual do sistema.
 * @param health Ponteiro para a estrutura onde o status será copiado.
 * @return ESP_OK se sucesso, ESP_ERR_INVALID_ARG se `health` for NULL.
 */
esp_err_t mqtt_get_health_status(health_status_t *health);

/**
 * @brief Imprime as estatísticas MQTT no log.
 * Útil para depuração e monitoramento.
 */
void mqtt_print_statistics(void);

/* Tópicos MQTT Padrão */

/** Tópico base do sistema */
#define MQTT_TOPIC_BASE "demo/central"

/** Tópico de status (online/offline) */
#define MQTT_TOPIC_STATUS MQTT_TOPIC_BASE "/status"

/** Tópico de telemetria */
#define MQTT_TOPIC_TELEMETRY MQTT_TOPIC_BASE "/telemetria"

/** Tópico de health check */
#define MQTT_TOPIC_HEALTH MQTT_TOPIC_BASE "/health"

/** Tópico de comandos recebidos */
#define MQTT_TOPIC_COMMANDS MQTT_TOPIC_BASE "/comandos"

/** Tópico de configuração */
#define MQTT_TOPIC_CONFIG MQTT_TOPIC_BASE "/config"

/** Tópico de boot/informações iniciais */
#define MQTT_TOPIC_BOOT MQTT_TOPIC_BASE "/boot"

/** Tópico de alertas/erros */
#define MQTT_TOPIC_ALERTS MQTT_TOPIC_BASE "/alertas"

#endif /* MQTT_SYSTEM_H */