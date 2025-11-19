#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"

/*
 * Tag para logging - facilita filtrar mensagens deste módulo
 * nos logs do sistema
 */
static const char *TAG = "MQTT_EXEMPLO";

/*
 * Handle do cliente MQTT - este ponteiro será nossa referência
 * para todas as operações MQTT após inicialização
 */
esp_mqtt_client_handle_t mqtt_client = NULL;

/*
 * Declaração forward da função de processamento de mensagens
 */
static void processar_mensagem_mqtt(const char *topic,
												int topic_len,
												const char *data,
												int data_len);

/*
 * Esta função é o coração da sua aplicação MQTT.
 * Ela será chamada automaticamente pelo ESP-IDF sempre que:
 * - A conexão com o *broker* for estabelecida
 * - A conexão for perdida
 * - Uma mensagem chegar em um tópico assinado
 * - Ocorrer um erro
 * - E vários outros eventos
 *
 * O event_id indica qual tipo de evento ocorreu.
 */
static void mqtt_event_handler(void *handler_args,
										 esp_event_base_t base,
										 int32_t event_id,
										 void *event_data)
{
	/*
	 * Cast do event_data para o tipo correto.
	 * Todos os eventos MQTT usam esta estrutura que contém
	 * informações específicas sobre o evento.
	 */
	esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

	/*
	 * Extrair o handle do cliente para facilitar acesso
	 */
	esp_mqtt_client_handle_t client = event->client;

	/*
	 * Switch baseado no tipo de evento.
	 * Cada case trata um evento específico do ciclo de vida MQTT.
	 */
	switch ((esp_mqtt_event_id_t)event_id)
	{

	case MQTT_EVENT_CONNECTED:
		/*
		 * Evento disparado quando conexão com *broker* é estabelecida.
		 * Este é o momento ideal para fazer subscrições em tópicos.
		 */
		ESP_LOGI(TAG, "Conectado ao *broker* MQTT!");

		/*
		 * Subscrever em tópicos de interesse.
		 * Cada chamada retorna um msg_id que pode ser usado
		 * para rastrear o sucesso da subscrição.
		 */
		int msg_id;

		// QoS 0 - mensagens de monitoramento não críticas
		msg_id = esp_mqtt_client_subscribe(client,
													  "jardim/+/temperatura",
													  0);
		ESP_LOGI(TAG, "Subscrito em jardim/+/temperatura, msg_id=%d",
					msg_id);

		// QoS 1 - comandos importantes que não podem ser perdidos
		msg_id = esp_mqtt_client_subscribe(client,
													  "demo/comandos/#",
													  1);
		ESP_LOGI(TAG, "Subscrito em demo/comandos/#, msg_id=%d",
					msg_id);

		break;

	case MQTT_EVENT_DISCONNECTED:
		/*
		 * Conexão perdida com o *broker*.
		 * O cliente MQTT automaticamente tentará reconectar,
		 * mas você pode querer implementar lógica adicional aqui.
		 */
		ESP_LOGW(TAG, "Desconectado do *broker* MQTT");

		/*
		 * Aqui você poderia:
		 * - Armazenar dados localmente até reconectar
		 * - Ativar um LED indicando perda de conexão
		 * - Ajustar comportamento de sensores/atuadores
		 */

		break;

	case MQTT_EVENT_SUBSCRIBED:
		/*
		 * Confirmação de que subscrição foi bem sucedida.
		 * O msg_id corresponde ao retornado por subscribe().
		 */
		ESP_LOGI(TAG, "Subscricao confirmada, msg_id=%d",
					event->msg_id);
		break;

	case MQTT_EVENT_UNSUBSCRIBED:
		/*
		 * Confirmação de cancelamento de subscrição.
		 */
		ESP_LOGI(TAG, "Cancelamento de subscricao confirmado, msg_id=%d",
					event->msg_id);
		break;

	case MQTT_EVENT_PUBLISHED:
		/*
		 * Confirmação de que publicação foi bem sucedida.
		 * Só ocorre para QoS 1 e 2.
		 */
		ESP_LOGI(TAG, "Mensagem publicada com sucesso, msg_id=%d",
					event->msg_id);
		break;

	case MQTT_EVENT_DATA:
		/*
		 * Este é o evento mais importante: chegada de nova mensagem!
		 * Aqui você processa os dados recebidos.
		 */
		ESP_LOGI(TAG, "Mensagem recebida!");

		/*
		 * Imprimir informações sobre o tópico.
		 * Note que usamos printf com especificador %.*s que permite
		 * imprimir string sem null-terminator usando comprimento.
		 */
		ESP_LOGI(TAG, "Topico: %.*s",
					event->topic_len,
					event->topic);

		/*
		 * Imprimir o payload (dados) da mensagem
		 */
		ESP_LOGI(TAG, "Dados: %.*s",
					event->data_len,
					event->data);

		/*
		 * Aqui você implementaria a lógica de negócio:
		 * - Parse do payload (pode ser JSON, string, binário, etc)
		 * - Validação dos dados
		 * - Ações baseadas no conteúdo (controlar atuadores, etc)
		 */
		processar_mensagem_mqtt(event->topic,
										event->topic_len,
										event->data,
										event->data_len);

		break;

	case MQTT_EVENT_ERROR:
		/*
		 * Algo deu errado na comunicação MQTT.
		 * O campo error contém detalhes específicos.
		 */
		ESP_LOGE(TAG, "Erro MQTT!");

		if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
		{
			ESP_LOGE(TAG, "Erro na camada de transporte");
			ESP_LOGE(TAG, "Codigo de erro: 0x%x",
						event->error_handle->esp_transport_sock_errno);
		}
		break;

	default:
		/*
		 * Outros eventos que podem ocorrer mas não precisamos
		 * tratar especificamente neste exemplo.
		 */
		ESP_LOGI(TAG, "Evento MQTT nao tratado, id: %d", event_id);
		break;
	}
}

/*
 * Esta função é chamada sempre que uma mensagem chega.
 * Aqui você implementa a lógica específica da sua aplicação.
 *
 * Parâmetros:
 *   topic: ponteiro para string do tópico (NÃO é null-terminated!)
 *   topic_len: comprimento da string do tópico
 *   data: ponteiro para os dados da mensagem (NÃO é null-terminated!)
 *   data_len: comprimento dos dados
 */
static void processar_mensagem_mqtt(const char *topic,
												int topic_len,
												const char *data,
												int data_len)
{
	/*
	 * Precisamos trabalhar com strings null-terminated.
	 * Vamos criar cópias locais com terminação nula.
	 */
	char topic_str[128];
	char data_str[256];

	/*
	 * Copiar topic e adicionar null terminator.
	 * Sempre verificar limites para evitar buffer overflow!
	 */
	int copy_len = topic_len < (sizeof(topic_str) - 1) ? topic_len : (sizeof(topic_str) - 1);
	memcpy(topic_str, topic, copy_len);
	topic_str[copy_len] = '\0';

	/*
	 * Mesmo processo para os dados
	 */
	copy_len = data_len < (sizeof(data_str) - 1) ? data_len : (sizeof(data_str) - 1);
	memcpy(data_str, data, copy_len);
	data_str[copy_len] = '\0';

	ESP_LOGI(TAG, "Processando: topico='%s', dados='%s'",
				topic_str, data_str);

	/*
	 * Implementar lógica baseada no tópico.
	 * Aqui usamos comparação de strings para decidir a ação.
	 */

	if (strncmp(topic_str, "demo/comandos/bomba", 24) == 0)
	{
		/*
		 * Comando para controlar bomba de irrigação
		 */
		if (strcmp(data_str, "LIGAR") == 0)
		{
			ESP_LOGI(TAG, "Acionando bomba de irrigacao");
			// Aqui você chamaria função para ligar GPIO da bomba
			// gpio_set_level(GPIO_BOMBA, 1);
		}
		else if (strcmp(data_str, "DESLIGAR") == 0)
		{
			ESP_LOGI(TAG, "Desligando bomba de irrigacao");
			// gpio_set_level(GPIO_BOMBA, 0);
		}
		else
		{
			ESP_LOGW(TAG, "Comando desconhecido para bomba: %s", data_str);
		}
	}
	else if (strncmp(topic_str, "demo/comandos/valvula/", 27) == 0)
	{
		/*
		 * Comando para válvula específica.
		 * Extrair número da válvula do tópico.
		 */
		int valvula_num = atoi(&topic_str[27]);

		ESP_LOGI(TAG, "Comando para valvula %d: %s",
					valvula_num, data_str);

		/*
		 * Aqui você implementaria controle da válvula específica
		 */
	}
	else if (strstr(topic_str, "temperatura") != NULL)
	{
		/*
		 * Mensagem de sensor de temperatura.
		 * Converter string para float para processamento.
		 */
		float temperatura = atof(data_str);

		ESP_LOGI(TAG, "Temperatura recebida: %.2f °C", temperatura);

		/*
		 * Implementar lógica baseada na temperatura:
		 * - Acionar sistema de refrigeração
		 * - Ajustar intensidade de irrigação
		 * - Enviar alertas se temperatura crítica
		 */

		if (temperatura > 35.0)
		{
			ESP_LOGW(TAG, "Temperatura alta! Aumentando irrigacao");
			// Implementar ação apropriada
		}
	}
	else
	{
		/*
		 * Tópico não reconhecido - apenas log para debug
		 */
		ESP_LOGD(TAG, "Topico não tratado: %s", topic_str);
	}
}

/*
 * Esta função configura e inicializa o cliente MQTT.
 * Deve ser chamada após WiFi estar conectado.
 *
 * Retorna: handle do cliente MQTT ou NULL em caso de erro
 */
esp_mqtt_client_handle_t mqtt_app_start(void)
{
	/*
	 * Estrutura de configuração do cliente MQTT.
	 * Todos os parâmetros de conexão são definidos aqui.
	 */
	esp_mqtt_client_config_t mqtt_cfg = {
		 /*
		  * URI do *broker* no formato mqtt://host:porta
		  * Pode usar também mqtts:// para conexão criptografada
		  */
		 .broker.address.uri = "mqtt://192.168.1.100:1883",

		 /*
		  * Credenciais de autenticação (se o *broker* requer)
		  * Comente estas linhas se *broker* permite acesso anônimo
		  */
		 // .credentials.username = "esp32_irrigacao",
		 // .credentials.authentication.password = "senha_segura",

		 /*
		  * Client ID - identificador único deste cliente.
		  * O *broker* usa isto para gerenciar sessão persistente.
		  * Se dois clientes com mesmo ID conectarem, o primeiro
		  * será desconectado!
		  */
		 .credentials.client_id = "esp32_jardim_central",

		 /*
		  * Last Will and Testament (LWT) - "testamento"
		  * Se conexão for perdida inesperadamente, o *broker*
		  * publicará automaticamente esta mensagem.
		  * Muito útil para detectar dispositivos offline!
		  */
		 .session.last_will.topic = "demo/status/central",
		 .session.last_will.msg = "OFFLINE",
		 .session.last_will.msg_len = 7,
		 .session.last_will.qos = 1,
		 .session.last_will.retain = true, // Manter mensagem para novos *subscriber*s

		 /*
		  * Keep alive - intervalo em segundos para enviar ping
		  * ao *broker* mantendo conexão ativa.
		  * Valores típicos: 30-120 segundos.
		  */
		 .session.keepalive = 60,

		 /*
		  * Desabilitar auto-reconnect se quiser controlar
		  * manualmente tentativas de reconexão.
		  * Por padrão é habilitado, o que é geralmente desejado.
		  */
		 .network.disable_auto_reconnect = false,
	};

	/*
	 * Criar instância do cliente com a configuração
	 */
	mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

	if (mqtt_client == NULL)
	{
		ESP_LOGE(TAG, "Falha ao criar cliente MQTT!");
		return NULL;
	}

	/*
	 * Registrar handler de eventos.
	 * Este callback será chamado para todos os eventos MQTT.
	 */
	esp_mqtt_client_register_event(mqtt_client,
											 ESP_EVENT_ANY_ID,
											 mqtt_event_handler,
											 NULL);

	/*
	 * Iniciar o cliente.
	 * Isto dispara tentativa de conexão com o *broker*.
	 */
	esp_err_t err = esp_mqtt_client_start(mqtt_client);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Falha ao iniciar cliente MQTT: %s",
					esp_err_to_name(err));
		return NULL;
	}

	ESP_LOGI(TAG, "Cliente MQTT iniciado com sucesso");

	/*
	 * Publicar mensagem inicial indicando que dispositivo está online.
	 * Note que fazemos isto aqui ao invés de no evento CONNECTED
	 * porque queremos enviar com retain=true.
	 */

	return mqtt_client;
}

/*
 * Função wrapper para simplificar publicações MQTT.
 * Encapsula lógica comum e tratamento de erros.
 *
 * Parâmetros:
 *   topic: tópico onde publicar
 *   data: dados a publicar (pode ser string, JSON, dados binários)
 *   len: comprimento dos dados (use 0 para calcular automaticamente se string)
 *   qos: nível de qualidade de serviço (0, 1, ou 2)
 *   retain: se true, *broker* manterá última mensagem para novos *subscriber*s
 *
 * Retorna: msg_id da publicação ou -1 em caso de erro
 */
int mqtt_publish_data(const char *topic,
							 const char *data,
							 int len,
							 int qos,
							 bool retain)
{
	/*
	 * Verificar se cliente está inicializado
	 */
	if (mqtt_client == NULL)
	{
		ESP_LOGE(TAG, "Cliente MQTT nao inicializado!");
		return -1;
	}

	/*
	 * Se len é 0, assumir que data é string e calcular comprimento
	 */
	if (len == 0)
	{
		len = strlen(data);
	}

	/*
	 * Publicar mensagem
	 */
	int msg_id = esp_mqtt_client_publish(mqtt_client,
													 topic,
													 data,
													 len,
													 qos,
													 retain ? 1 : 0);

	if (msg_id < 0)
	{
		ESP_LOGE(TAG, "Falha ao publicar em %s", topic);
		return -1;
	}

	ESP_LOGD(TAG, "Publicado em %s: %.*s (msg_id=%d, qos=%d)",
				topic, len, data, msg_id, qos);

	return msg_id;
}

/*
 * Task que simula leitura de sensores e publica via MQTT.
 * Em aplicação real, você leria sensores reais aqui.
 */
void mqtt_publisher_task(void *pvParameters)
{
	/*
	 * Buffer para construir mensagens
	 */
	char msg_buffer[128];

	/*
	 * Contadores para simular dados variáveis
	 */
	uint32_t contador = 0;

	/*
	 * Aguardar conexão MQTT antes de começar publicações.
	 * Em implementação robusta, você usaria event group ou semáforo
	 * para sincronização precisa.
	 */
	ESP_LOGI(TAG, "Task de publicacao aguardando conexao MQTT...");
	vTaskDelay(pdMS_TO_TICKS(5000));

	ESP_LOGI(TAG, "Iniciando publicacoes periodicas");

	while (1)
	{
		/*
		 * Publicar status online periodicamente.
		 * Isto ajuda sistema de monitoramento detectar dispositivo ativo.
		 */
		mqtt_publish_data("demo/status/central",
								"ONLINE",
								0,		 // calcular comprimento automaticamente
								1,		 // QoS 1 - importante garantir entrega
								true); // retain - novos *subscriber*s saberão status

		/*
		 * Simular leitura de temperatura
		 */
		float temperatura = 20.0 + (rand() % 100) / 10.0;
		snprintf(msg_buffer, sizeof(msg_buffer), "%.2f", temperatura);

		mqtt_publish_data("jardim/central/temperatura",
								msg_buffer,
								0,
								0, // QoS 0 - dados frequentes, perda tolerável
								false);

		/*
		 * Simular leitura de umidade do solo
		 */
		float umidade = 30.0 + (rand() % 500) / 10.0;
		snprintf(msg_buffer, sizeof(msg_buffer), "%.2f", umidade);

		mqtt_publish_data("jardim/sensor1/umidade",
								msg_buffer,
								0,
								0,
								false);

		/*
		 * Publicar dados em formato JSON estruturado.
		 * JSON facilita parsing no backend e permite
		 * enviar múltiplos valores em uma mensagem.
		 */
		snprintf(msg_buffer, sizeof(msg_buffer),
					"{\"temp\":%.2f,\"umid\":%.2f,\"cnt\":%lu}",
					temperatura, umidade, contador++);

		mqtt_publish_data("jardim/central/telemetria",
								msg_buffer,
								0,
								1, // QoS 1 - dados agregados são importantes
								false);

		/*
		 * Aguardar intervalo antes da próxima publicação.
		 * Ajuste baseado nos requisitos da aplicação:
		 * - Sensores de temperatura: 30-60 segundos
		 * - Sensores de movimento: imediato quando detecta
		 * - Dados de debug: 5-10 segundos
		 */
		vTaskDelay(pdMS_TO_TICKS(10000)); // 10 segundos
	}
}
