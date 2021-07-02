/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Version: v1.0
 *===========================================================================*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "queue.h"
#include "keys.h"
#include "app.h"
#include "string.h"

#define USED_UART UART_USB
#define UART_RATE 9600
#define LED LED1
#define LED_RATE 1000
#define PERIODO pdMS_TO_TICKS(LED_RATE)
#define nQueue 4
#define Nsem 2
#define PERIODO pdMS_TO_TICKS(LED_RATE)
#define MESSAGE_LED "LED ON\r\n"
#define MESSAGE_BUTTON_TIME "TEC%i T:%lu\r\n"

BaseType_t res;
QueueHandle_t Queue;
extern QueueHandle_t QueueAux;

void TareaA(void * pvParameters);
void TareaB(void * pvParameters);
void TareaC(void * pvParameters);

/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE
;

int main(void) {
	/* Inicializar la placa */
	boardConfig();

	debugPrintConfigUart( USED_UART , UART_RATE );	// UART for debug messages

	/* inicializo driver de teclas */
	keys_Init();

	res = xTaskCreate(TareaA, (const char *) "TareaA", configMINIMAL_STACK_SIZE * 2, 0, tskIDLE_PRIORITY + 1, 0);

	configASSERT(res == pdPASS);

	res = xTaskCreate(TareaB, (const char *) "TareaB", configMINIMAL_STACK_SIZE * 2, 0, tskIDLE_PRIORITY + 1, 0);

	configASSERT(res == pdPASS);

	res = xTaskCreate(TareaC, (const char *) "TareaC", configMINIMAL_STACK_SIZE * 2, 0, tskIDLE_PRIORITY + 1, 0);

	configASSERT(res == pdPASS);

	Queue = xQueueCreate(nQueue, sizeof(char *));

	configASSERT(Queue != NULL);

	/* arranco el scheduler */
	vTaskStartScheduler();

	// ---------- REPETIR POR SIEMPRE --------------------------
	configASSERT(0);

	// NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
	// directamenteno sobre un microcontroladore y no es llamado por ningun
	// Sistema Operativo, como en el caso de un programa para PC.

	return TRUE;
}

void TareaA(void * pvParameters) {

	TickType_t time = xTaskGetTickCount();
	TickType_t Periodity = PERIODO;
	char *data;
	data = (char *) pvPortMalloc(sizeof(char));
	configASSERT(data != NULL);

	while (1) {

		gpioWrite(LED, ON);
		data = MESSAGE_LED;
		xQueueSend(Queue, &data, 0);
		vTaskDelay(Periodity / 2);
		gpioWrite(LED, OFF);
		vTaskDelayUntil(&time, Periodity);

	}

}

void TareaB(void * pvParameters) {

	TickType_t TimeDiff;
	gpioMap_t Button;
	char *data;
	data = (char *) pvPortMalloc(sizeof(char));
	configASSERT(data != NULL);

	while (1) {

		xQueueReceive(QueueAux, &Button, portMAX_DELAY);
		taskENTER_CRITICAL();
		TimeDiff = get_diff(Button);
		sprintf(data, MESSAGE_BUTTON_TIME, Button + 1, TimeDiff);
		taskEXIT_CRITICAL();
		xQueueSend(Queue, &data, 0);

	}

}

void TareaC(void * pvParameters) {

	char *data;

	while (1) {

		xQueueReceive(Queue, &data, portMAX_DELAY);
		taskENTER_CRITICAL();
		printf(data);
		taskEXIT_CRITICAL();

	}

}

