#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "data_processing.h"


//PA0 echo PC3 trigg `

// ���������� ����������
uint8_t catcher_status = 0;     // ��������� �������: 0 - ����������� �����, 1 - ���������
uint16_t duration = 0;          // ������������ ���������� ���������� ��������




void Delay_ms(uint32_t ms)
{
  volatile uint32_t nCount;
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq (&RCC_Clocks);

  nCount = (RCC_Clocks.HCLK_Frequency/10000)*ms;
  for (; nCount!=0; nCount--);
}

void init_ports()
{
//GPIO_InitTypeDef PORT;
//  // ���� �, ��� 3 - �����. ���������� � ����� Trig ������ HC-SR04
//  // ���� A, ��� 0 - ����. ���������� � ����� Echo ������ HC-SR04
////  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
//  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN;
//  PORT.GPIO_Pin = GPIO_Pin_3;
//  PORT.GPIO_Mode = GPIO_Mode_Out_PP;
//  PORT.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIOC, &PORT);
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN |RCC_AHBENR_GPIOBEN;
	GPIOC->MODER |= GPIO_MODER_MODER3_0;

}

void init_interrupts()
{
  //========================================================================
  //                          ��������� ������� 6
  // ������������ ��� 2-� �����:
  // 1) ������� ������������ Echo �������� (150 ��� - 25 ��)
  // 2) ������� � ����������� ��� ������ ������� ����� - �������,
  // ������������ ��� ��������� ���������� ��������� � ����� Echo
  //========================================================================
  // �������� ������������ �������
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

  // ���������� ������������ �� ������������ ��� � ���
  TIM6->PSC = 24 - 1;
  // ������� ������������ - 50 �� = 50 000 ���
  TIM6->ARR = 50000;
  //���������� TIM6_IRQn ���������� - ���������� ��� ������� ������� �����
  NVIC_SetPriority(TIM6_DAC_IRQn, 3);
  NVIC_EnableIRQ(TIM6_DAC_IRQn);
  //========================================================================
  //                          ��������� ������� 7
  //========================================================================
  // �������� ������������ �������
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
  RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
  // ���������� ������������ �� ������������ ��� � ���
  TIM7->PSC = 24 - 1;
  // ������� ������������ - 10 ���
  TIM7->ARR = 10;
  //���������� TIM7_IRQn ���������� - ���������� ��� ������� ����������� ��������
  NVIC_SetPriority(TIM7_IRQn, 2);
  NVIC_EnableIRQ(TIM7_IRQn);
  //========================================================================
  //                          ��������� �������� ����������
  // ������� ���������� �� ��������� �������� ��� ����� ����� A.
  // ����� ������������ 0-� ��� ����� �.
  //========================================================================
  // �������� Alternate function I/O clock
//  RCC_APB2PeriphClockCmd(RCC_APB2ENR_AFIOEN , ENABLE);
  SYSCFG->EXTICR[0] &= (uint16_t)~SYSCFG_EXTICR1_EXTI0_PA;
  // ���������� �� ������� ���� ���������
  EXTI->IMR |= EXTI_IMR_MR0;
  // ���������� �� ������������ ������
  EXTI->RTSR |= EXTI_RTSR_TR0;
  //��������� ����������
  NVIC_SetPriority(EXTI0_1_IRQn, 1);
  NVIC_EnableIRQ (EXTI0_1_IRQn);
}

int main(void)
{
              // ��������� USART ��� ������ printf � COM
  init_ports();
  init_interrupts();
  USARTInit();
  Array distances;
  initArray(&distances, 150);



  // ��������� ������ 7 ������ ��� �� ������ 10 ��
  TIM7->DIER |= TIM_DIER_UIE;          // ��������� ���������� �� ������� 7
  GPIOC->ODR |= GPIO_Pin_3;            // �������� ���������� �������
  TIM7->CR1 |= TIM_CR1_CEN;
  // ��������� ������
  int i = 0;
  while(i != 60)
    {
      Delay_ms(500);
      printf("S = %d mm, %d\n", duration/29, duration);
      insertArray(&distances, duration / 29);
      i++;
    }
    for (int i = 0; i < 6; i++) {
    	printf("%d \n", distances.array[i]);
    }

    sendToAndroid(processData(distances));
}

// ���������� ���������� EXTI0: ��������� ������ �������
void EXTI0_1_IRQHandler(void)
{
  // ���� ������� ����������� �����
  if (!catcher_status)
  {
    // ��������� ������ ������������ ��������
    TIM6->CR1 |= TIM_CR1_CEN;
    // ������������� �� ����� ���������� ������
    catcher_status = 1;
    EXTI->RTSR &= ~EXTI_RTSR_TR0;
    EXTI->FTSR |= EXTI_FTSR_TR0;
  }
  // ���� ������� ��������� �����
  else
  {
    TIM6->CR1 &= ~TIM_CR1_CEN;         // ������������� ������
    duration = TIM6->CNT;              // ��������� �������� ������������ � ���
    TIM6->CNT = 0;                     // �������� �������-�������
    // ������������� �� ����� ������������ ������
    catcher_status = 0;
    EXTI->FTSR &= ~EXTI_FTSR_TR0;
    EXTI->RTSR |= EXTI_RTSR_TR0;
    // ��������� ������ 6 �� ������ 50 ��
    TIM6->DIER |= TIM_DIER_UIE;        // ��������� ���������� �� �������
    TIM6->CR1 |= TIM_CR1_CEN;          // ��������� ������
  }
  EXTI->PR |= 0x01;                    //������� ����
}

// ���������� ���������� TIM7_DAC
// ���������� ����� ����, ��� ������ 7 �������� 10 ��� ��� ����������� ��������
void TIM7_IRQHandler(void)
{
  TIM7->SR &= ~TIM_SR_UIF;             // ���������� ���� UIF
  GPIOC->ODR &= ~GPIO_Pin_3;           // ������������� ���������� �������
  TIM7->DIER &= ~TIM_DIER_UIE;         // ��������� ���������� �� ������� 7
}

// ���������� ���������� TIM6_DAC
// ���������� ����� ����, ��� ������ 6 �������� 50 ��� ��� ������� �����
void TIM6_DAC_IRQHandler(void)
{
  TIM6->SR &= ~TIM_SR_UIF;             // ���������� ���� UIF
  GPIOC->ODR |= GPIO_Pin_3;            // �������� ���������� �������
  // ��������� ������ 7 �� ������ 10 ��
  TIM7->DIER |= TIM_DIER_UIE;          // ��������� ���������� �� ������� 7
  TIM7->CR1 |= TIM_CR1_CEN;            // ��������� ������
}


