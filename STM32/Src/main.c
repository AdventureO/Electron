
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx.h"
#include "stm32f0xx_hal.h"
#include "gpio.h"
#include "usbd_cdc_if.h"
#include "string.h"
#include "data_processing.h"
#include "USART.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */
#define SystemCoreClockInMHz (SystemCoreClock/1000000

volatile uint8_t catcher_status = 0;     //status of echo interuption : 0 - rising edge, 1 - falling edge
volatile uint16_t duration = 0;
uint8_t MODE = 0; // 0 - raw data, 1 - strikes

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/


/* USER CODE END PFP */
//int DWT_Init(void) {
//
//    /* Enable TRC */
//    CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;
//
//    /* Enable counter */
//    DWT->CTRL |=  SysTick_CTRL_ENABLE_Msk;
//
//    /* Reset counter */
//    DWT->CYCCNT = 0; // Для STM32F3 -- 32-бітний
//
// /* Check if DWT has started */
// uint32_t before = DWT->CYCCNT;
// __NOP();
// __NOP();
//
// /* Return difference, if result is zero, DWT has not started */
// return (DWT->CYCCNT - before);
//}

//inline void udelay_DWT (uint32_t useconds) {
// // DWT->CYCCNT = 0; // Максимізуємо можливий інтервал
// // Але тоді udelay_DWT i get_DWT_us не можна буде змішувати.
//
// useconds *= SystemCoreClockInMHz;
// while( DWT->CYCCNT < useconds){}
//}

void Ports_Init() {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN |RCC_AHBENR_GPIOBEN;
	GPIOC->MODER |= GPIO_MODER_MODER3_0;
}

void ADC_Init() {

}
//
void Interrupts_Init() {
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

void USART1_IRQHandler() {}




void USB_SendString(char str[]) {
	CDC_Transmit_FS(str, strlen(str));
	HAL_Delay(500);
}

void USB_SendInt(int num) {
	char str[100] = "";
	sprintf(str, "%u;", num);
	USB_SendString(str);
}






int Infred_Read() {

}

int Laser_Read() {

}


/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  Ports_Init();

  Interrupts_Init();
  USART_Init();
//  DWT_Init();

  int TDS_Ultrasonic = duration/29;
  int numberOfElements_Ultrasionic = 0;
  int numberOfTDS_Ultrasonic  = 0;
  int distancesArray_Ultrasonic[100];

  int TDS_Lazer = duration/29;
  int numberOfElements_Lazer = 0;
  int numberOfTDS_Lazer  = 0;
  int distancesArray_Lazer[100];




// Set timer to count first 10us
  TIM7->DIER |= TIM_DIER_UIE;          // allow TIM7 interrupts
  GPIOC->ODR |= GPIO_ODR_3;            // turn on TRIG
  TIM7->CR1 |= TIM_CR1_CEN;            // enable TIM7


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */
	  if (MODE == 0) {
		  USART_SendInt(duration/29);
		  USB_SendInt(duration/29);
		  USART_SendInt(Infred_Read);
		  USB_SendInt(duration/29);
		  USART_SendInt(Laser_Read);
		  USB_SendInt(duration/29);
	  } else {
		HAL_Delay(300);
		int distance = duration/29;
//		printf("U: %d\n", distance);
		HAL_Delay(300);
		getNumberOfStrikes(distance, &TDS_Ultrasonic, &numberOfElements_Ultrasionic, &numberOfTDS_Ultrasonic, distancesArray_Ultrasonic, 0, 'U');
  //		Delay_ms(300);

		HAL_Delay(300);
		int distance_Lazer = duration/29;
//		printf("L: %d\n", distance_Lazer);
		HAL_Delay(300);
		getNumberOfStrikes(distance_Lazer, &TDS_Lazer, &numberOfElements_Lazer, &numberOfTDS_Lazer, distancesArray_Lazer, 0, 'L');
		HAL_Delay(300);

	  }

	}





  /* USER CODE BEGIN 3 */
}
  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
