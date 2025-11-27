/**
 * @file sensor_simulate_task.h
 * @brief Task para simular e publicar dados de sensores.
 *
 * Simula leituras de luminosidade e temperatura, publicando-as em
 * tópicos MQTT em intervalos regulares.
 *
 * @author GitHub Copilot
 * @date 2025
 */

#ifndef SENSOR_SIMULATE_TASK_H
#define SENSOR_SIMULATE_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*
 * =============================================================================
 * CONFIGURAÇÕES DA TASK
 * =============================================================================
 */

/** @brief Intervalo de publicação em milissegundos (1 segundo) */
#define SENSOR_SIMULATE_INTERVAL_MS 1000

/** @brief Tamanho da stack da task em bytes */
#define SENSOR_SIMULATE_TASK_STACK_SIZE 3072

/** @brief Prioridade da task de simulação de sensor */
#define SENSOR_SIMULATE_TASK_PRIORITY 2

/** @brief Nome da task para debug */
#define SENSOR_SIMULATE_TASK_NAME "SensorSimulate"

/*
 * =============================================================================
 * PROTÓTIPOS DE FUNÇÕES
 * =============================================================================
 */

/**
 * @brief Função da task de simulação de sensores.
 *
 * Gera valores aleatórios para luminosidade e temperatura e os publica
 * em tópicos MQTT a cada segundo.
 *
 * @param pvParameters Parâmetros da task (não utilizado).
 */
void sensor_simulate_task(void *pvParameters);

#endif // SENSOR_SIMULATE_TASK_H