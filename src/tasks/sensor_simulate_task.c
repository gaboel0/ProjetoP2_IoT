/**
 * @file sensor_simulate_task.c
 * @brief Task que simula e publica dados de sensores.
 *
 * Implementa a task que simula leituras de sensores e as publica via MQTT.
 *
 * @author GitHub Copilot
 * @date 2025
 */

#include "tasks/sensor_simulate_task.h"
#include "services/mqtt_system.h"
#include "esp_log.h"
#include "esp_random.h"
#include <stdio.h>

static const char *TAG = "SENSOR_SIMULATE";

void sensor_simulate_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Task de simulação de sensores iniciada");

    char buffer[16];

    while (1)
    {
        if (mqtt_system_is_connected())
        {
            // Simula e publica luminosidade (0 a 10)
            int luminosity = esp_random() % 11;
            snprintf(buffer, sizeof(buffer), "%d", luminosity);
            mqtt_publish_data("/casa/externo/luminosidade", buffer, 0, 1, false);

            // Simula e publica temperatura (-3 a 45)
            int temperature = (esp_random() % 49) - 3;
            snprintf(buffer, sizeof(buffer), "%d", temperature);
            mqtt_publish_data("/casa/sala/temperatura", buffer, 0, 1, false);

            ESP_LOGI(TAG, "Sensores simulados: Luminosidade=%d, Temperatura=%d°C", luminosity, temperature);
        }

        vTaskDelay(pdMS_TO_TICKS(SENSOR_SIMULATE_INTERVAL_MS));
    }
}