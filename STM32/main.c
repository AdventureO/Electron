#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "data_processing.h"


//PA0 echo PC3 trig
//PC10 PC11 motor control pins`


volatile uint8_t catcher_status = 0;     //status of echo interuption : 0 - rising edge, 1 - falling edge
volatile uint16_t duration = 0;

void Delay_ms(uint32_t ms) {
  volatile uint32_t nCount;
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq (&RCC_Clocks);
  nCount = (RCC_Clocks.HCLK_Frequency/10000)*ms;
  for (; nCount!=0; nCount--);
}

void initPorts() {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN |RCC_AHBENR_GPIOBEN;
	GPIOC->MODER |= GPIO_MODER_MODER3_0;
}
//
void init_interrupts() {
  //use TIM6 for counting ECHO length
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	TIM6->PSC = 24 - 1; //us prescaler
	TIM6->ARR = 50000;  //count till 50 mc - time required for echo impulse to fade down
	NVIC_SetPriority(TIM6_DAC_IRQn, 3); //interrupt required to count whole cycle length
	NVIC_EnableIRQ(TIM6_DAC_IRQn);

  //use TIM 7 for counting TRIG time
	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
	TIM7->PSC = 24 - 1; //us prescaler
    TIM7->ARR = 10;//count till 10 uc - time required for trigging ECHO impulse
	NVIC_SetPriority(TIM7_IRQn, 2); //interrupt required for counting TRIG duration
	NVIC_EnableIRQ(TIM7_IRQn);

	//configure PA0 interrupts
	SYSCFG->EXTICR[0] &= (uint16_t)~SYSCFG_EXTICR1_EXTI0_PA;
	EXTI->IMR |= EXTI_IMR_MR0;
	EXTI->RTSR |= EXTI_RTSR_TR0;
	NVIC_SetPriority(EXTI0_1_IRQn, 1);
	NVIC_EnableIRQ (EXTI0_1_IRQn);
}

int main(void) {

	initPorts();
//	initMotor();
	init_interrupts();
	USARTInit();
	int distances[100];
	initArray(&distances, 10);
	uint8_t dir = 1;  //motor direction


  // Set timer to count first 10us
	TIM7->DIER |= TIM_DIER_UIE;          // allow TIM7 interrupts
	GPIOC->ODR |= GPIO_ODR_3;            // turn on TRIG
	TIM7->CR1 |= TIM_CR1_CEN;            // enable TIM7

	int i = 0;
	while(i != 50)
    {
		Delay_ms(500);
		printf("S = %d mm, %d\n", duration/29, duration);
		insertArray(&distances, duration / 29);
		i++;
		if (100 % 20) {
			if (dir){
				turnOnMotorDown();
				dir = 0;
			} else {
				turnOnMotorUp();
				dir = 1;
			}
    	}
    }
	TIM7->DIER &= ~TIM_DIER_UIE;
	TIM6->DIER &= ~TIM_DIER_UIE;

//    turnoffMotor();
	sendToAndroid(processData(distances));
//	while (1){
////		sendToAndroid(15);
//		USARTSendString("15");
//	}
	freeArray(&distances);
}


void EXTI0_1_IRQHandler(void)	{

	if ((EXTI->PR &= EXTI_PR_PR0) !=0 ) {
		if (!catcher_status) {
			TIM6->CR1 |= TIM_CR1_CEN; //start ECHO length counting
			catcher_status = 1;
			//switch to catching falling edge interrupts
			EXTI->RTSR &= ~EXTI_RTSR_TR0;
			EXTI->FTSR |= EXTI_FTSR_TR0;
		} else {
			TIM6->CR1 &= ~TIM_CR1_CEN;         // stop TIM6
			duration = TIM6->CNT;              // get length of ECHO in us
			TIM6->CNT = 0;
			catcher_status = 0;
			//switch to catching rising edge interrupts
			EXTI->FTSR &= ~EXTI_FTSR_TR0;
			EXTI->RTSR |= EXTI_RTSR_TR0;
			// set timer to count 50 ms of whole cycle
			TIM6->DIER |= TIM_DIER_UIE;        // enable TIM 6 interrupts
			TIM6->CR1 |= TIM_CR1_CEN;		  // start TIM6
		}
	}
	EXTI->PR |= 0x01;
}


// called after TIM7 counted 10us for TRIG
void TIM7_IRQHandler(void) {
	TIM7->SR &= ~TIM_SR_UIF;
	GPIOC->ODR &= ~GPIO_ODR_3;            // stop TRIG
	TIM7->DIER &= ~TIM_DIER_UIE;		//forbid interrupts from TIM7
}


// called after TIM6 counted 50ms for th whole cycle
void TIM6_DAC_IRQHandler(void) {
	  TIM6->SR &= ~TIM_SR_UIF;             //clear updtate interrupt
	  GPIOC->ODR |= GPIO_ODR_3;            // start TRIG
	  // start TIM7 for counting 10 us for TRIG
	  TIM7->DIER |= TIM_DIER_UIE;
	  TIM7->CR1 |= TIM_CR1_CEN;            // start TIM7
}


