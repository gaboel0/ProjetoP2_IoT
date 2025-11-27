/**
 * @file main.c
 * @brief Ponto de entrada da aplicação IoT com FreeRTOS.
 *
 * Inicializa o sistema e cria as tasks principais da aplicação.
 * A lógica de negócio é modularizada em tasks independentes.
 *
 * @author Moacyr Francischetti Correa
 * @date 2025
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "services/mqtt_system.h"
#include "tasks/system_monitor_task.h"
#include "tasks/custom_publish_task.h"
#include "tasks/sensor_simulate_task.h"

/*
 * =============================================================================
 * CONFIGURAÇÕES DA APLICAÇÃO
 * =============================================================================
 */

static const char *TAG = "MAIN_APP";

/*
 * =============================================================================
 * FUNÇÃO PRINCIPAL
 * =============================================================================
 */

/**
 * @brief Ponto de entrada da aplicação.
 *
 * Inicializa o sistema (WiFi, MQTT) e em seguida cria as tasks da
 * aplicação, transferindo o controle para o scheduler do FreeRTOS.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔═════════════════════════════════╗");
    ESP_LOGI(TAG, "║   Sistema de Demonstracao IoT   ║");
    ESP_LOGI(TAG, "║     Baseado em ESP32 + MQTT     ║");
    ESP_LOGI(TAG, "║   Arquitetura: FreeRTOS Tasks   ║");
    ESP_LOGI(TAG, "╚═════════════════════════════════╝");
    ESP_LOGI(TAG, "");

    // PASSO 1: Inicializa os serviços de WiFi, MQTT e outras funcionalidades base
    esp_err_t ret = mqtt_system_init();

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao inicializar sistema MQTT, abortando.");
        return;
    }

    ESP_LOGI(TAG, "Sistema MQTT inicializado com sucesso");
    ESP_LOGI(TAG, "");

    // PASSO 2: Criar as tasks da aplicação que rodam em paralelo.
    ESP_LOGI(TAG, "Criando tasks da aplicacao...");

    // Task 1: Monitoramento do Sistema
    BaseType_t task_created = xTaskCreate(
        system_monitor_task,        // Função da task
        MONITOR_TASK_NAME,          // Nome (debug)
        MONITOR_TASK_STACK_SIZE,    // Tamanho da Stack
        NULL,                       // Parâmetros
        MONITOR_TASK_PRIORITY,      // Prioridade
        NULL                        // Handle
    );

    if (task_created != pdPASS)
    {
        ESP_LOGE(TAG, "Falha ao criar task de monitoramento");
        return;
    }

    ESP_LOGI(TAG, "   [OK] Task: %s (Prioridade: %d)",
             MONITOR_TASK_NAME, MONITOR_TASK_PRIORITY);

    // Task 2: Publicação de Dados Customizados
    task_created = xTaskCreate(
        custom_publish_task,            // Função da task
        CUSTOM_PUBLISH_TASK_NAME,       // Nome (debug)
        CUSTOM_PUBLISH_TASK_STACK_SIZE, // Tamanho da Stack
        NULL,                           // Parâmetros
        CUSTOM_PUBLISH_TASK_PRIORITY,   // Prioridade
        NULL                            // Handle
    );

    if (task_created != pdPASS)
    {
        ESP_LOGE(TAG, "Falha ao criar task de publicacao customizada");
        return;
    }

    // Task 3: Simulação de Sensores
    task_created = xTaskCreate(
        sensor_simulate_task,            // Função da task
        SENSOR_SIMULATE_TASK_NAME,       // Nome (debug)
        SENSOR_SIMULATE_TASK_STACK_SIZE, // Tamanho da Stack
        NULL,                           // Parâmetros
        SENSOR_SIMULATE_TASK_PRIORITY,   // Prioridade
        NULL                            // Handle
    );

    if (task_created != pdPASS)
    {
        ESP_LOGE(TAG, "Falha ao criar task de simulação de sensores");
        return;
    }

    ESP_LOGI(TAG, "Tasks da aplicacao criadas com sucesso!");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "════════════════════════════════════════");
    ESP_LOGI(TAG, "  Sistema Inicializado com Sucesso!");
    ESP_LOGI(TAG, "════════════════════════════════════════");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Funcionalidades ativas:");
    ESP_LOGI(TAG, "   - Telemetria automatica a cada %d segundos",
             TELEMETRY_INTERVAL_MS / 1000);
    ESP_LOGI(TAG, "   - Health check a cada %d segundos",
             HEALTH_CHECK_INTERVAL_MS / 1000);
    ESP_LOGI(TAG, "   - Watchdog WiFi monitorando conectividade");
    ESP_LOGI(TAG, "   - Monitoramento do sistema a cada %d segundos",
             MONITOR_INTERVAL_MS / 1000);
    ESP_LOGI(TAG, "   - Publicacao customizada a cada %d segundos",
             CUSTOM_PUBLISH_INTERVAL_MS / 1000);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Tasks criadas: 3");
    ESP_LOGI(TAG, "   1. %s (P%d)", MONITOR_TASK_NAME, MONITOR_TASK_PRIORITY);
    ESP_LOGI(TAG, "   2. %s (P%d)", CUSTOM_PUBLISH_TASK_NAME, CUSTOM_PUBLISH_TASK_PRIORITY);
    ESP_LOGI(TAG, "   3. %s (P%d)", SENSOR_SIMULATE_TASK_NAME, SENSOR_SIMULATE_TASK_PRIORITY);
    ESP_LOGI(TAG, "");

    // PASSO 3: Finaliza app_main. O scheduler do FreeRTOS assume o controle.
    ESP_LOGI(TAG, "app_main() finalizando...");
    ESP_LOGI(TAG, "FreeRTOS scheduler assumiu o controle");
    ESP_LOGI(TAG, "");

    // A função app_main retorna, mas as tasks continuam a ser executadas pelo scheduler.
}
