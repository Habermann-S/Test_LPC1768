#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define LED1_OUTPUT (1<<0)
#define LED2_OUTPUT (1<<1)
#define Button1_IN (0<<30)
#define Button2_IN (0<<31)

static void prvSetupHardware( void );
void vTask_Button( void *pvParametres );
void vTask_LED1( void *pvParametres );
void vTask_LED2( void *pvParametres );
SemaphoreHandle_t xSemaphoreLED1;
SemaphoreHandle_t xSemaphoreLED2;

void vTask_Button( void *pvParametres){

    for( ;; ) {
      if(!(GPIO1->FIOPIN & (Button1_IN))) xSemaphoreGive(xSemaphoreLED1); 
      if(!(GPIO1->FIOPIN & (Button2_IN))) xSemaphoreGive(xSemaphoreLED2); 
    }  
}

void vTask_LED1( void *pvParametres){

    for( ;; ) {
      xSemaphoreTake( xSemaphoreLED1, portMAX_DELAY );
      GPIO2->FIOSET |= LED1_OUTPUT;
      vTaskDelay(10);
      GPIO2->FIOCLR |= LED1_OUTPUT;
      vTaskDelay(90);
    }
}

void vTask_LED2( void *pvParametres){

    for( ;; ) {
      xSemaphoreTake( xSemaphoreLED2, portMAX_DELAY );
      GPIO2->FIOSET |= LED2_OUTPUT;
      vTaskDelay(100);
      GPIO2->FIOCLR |= LED2_OUTPUT;
      vTaskDelay(900);
    }
}

void prvSetupHardware( void ){
  SC->PCONP = 0;
  SC->PCONP = PCONP_PCGPIO;
  GPIO1->FIODIR |= Button1_IN | Button2_IN;
  GPIO2->FIODIR |= LED1_OUTPUT | LED2_OUTPUT;
  GPIO2->FIOCLR |= LED1_OUTPUT | LED2_OUTPUT;
}

void vApplicationTickHook( void ){
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName ){
	/* This function will get called if a task overflows its stack. */

	( void ) pxTask;
	( void ) pcTaskName;

	for( ;; );
}

void vConfigureTimerForRunTimeStats( void ){
const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00, TCR_COUNT_ENABLE = 0x01;

	/* This function configures a timer that is used as the time base when
	collecting run time statistical information - basically the percentage
	of CPU time that each task is utilising.  It is called automatically when
	the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
	to 1). */

	/* Power up and feed the timer. */
	SC->PCONP |= 0x02UL;
	SC->PCLKSEL0 = (SC->PCLKSEL0 & (~(0x3<<2))) | (0x01 << 2);

	/* Reset Timer 0 */
	TIM0->TCR = TCR_COUNT_RESET;

	/* Just count up. */
	TIM0->CTCR = CTCR_CTM_TIMER;

	/* Prescale to a frequency that is good enough to get a decent resolution,
	but not too fast so as to overflow all the time. */
	TIM0->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;

	/* Start the counter. */
	TIM0->TCR = TCR_COUNT_ENABLE;
}

int main(void){
  prvSetupHardware();
  xSemaphoreLED1 = xSemaphoreCreateBinary();
  xSemaphoreLED2 = xSemaphoreCreateBinary();
  xTaskCreate( &vTask_Button, "vTask_Button", configMINIMAL_STACK_SIZE,  NULL, 1, NULL );
  xTaskCreate( &vTask_LED1, "vTask_LED1", configMINIMAL_STACK_SIZE,  NULL, 1, NULL );
  xTaskCreate( &vTask_LED1, "vTask_LED2", configMINIMAL_STACK_SIZE,  NULL, 1, NULL );
  
  vTaskStartScheduler();
  for( ;; );
}