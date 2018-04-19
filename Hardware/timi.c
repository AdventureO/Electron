#include "timing.h"

void initRTC(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	RTC_TimeTypeDef RTC_TimeStruct;

	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);

	/* Allow access to RTC */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset RTC Domain */
	RCC_BackupResetCmd(ENABLE);
	RCC_BackupResetCmd(DISABLE);

	/* Enable the LSE OSC */
	RCC_LSEConfig(RCC_LSE_ON);

	/* Wait until LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);


	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	RCC->BDCR |= RCC_BDCR_RTCEN;

	/* Configure the RTC data register and RTC prescaler */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	RTC_InitStructure.RTC_SynchPrediv  = 0xFF;
	RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);

	/* Set the time to 00h 00mn 00s AM */
	RTC_TimeStruct.RTC_H12     = RTC_H12_AM;
	RTC_TimeStruct.RTC_Hours   = 0x00;
	RTC_TimeStruct.RTC_Minutes = 0x00;
	RTC_TimeStruct.RTC_Seconds = 0x00;
	RTC_SetTime(RTC_Format_BCD,&RTC_TimeStruct);
}



void Delay_ms(uint32_t ms) {
  volatile uint32_t nCount;
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq (&RCC_Clocks);
  nCount = (RCC_Clocks.HCLK_Frequency/10000)*ms;
  for (; nCount!=0; nCount--);
}

//
//	RTC_InitTypeDef RTC_InitStruct;
////	RCC->BDCR |= RCC_BDCR_RTCEN;
////	RCC->BDCR |= RCC_BDCR_RTCSEL_LSE;
//	initRTC();
//	RTC_Init(&RTC_InitStruct);
//	Delay_ms(500);

//		RTC_TimeTypeDef RTC_StampTimeStruct;
//		RTC_DateTypeDef RTC_StampDateStruct;
//		RTC_GetTimeStamp(RTC_Format_BIN, &RTC_StampTimeStruct, &RTC_StampDateStruct);
//		printf("%d ", RTC_StampTimeStruct.RTC_Hours);
//		printf("%d ",RTC_StampTimeStruct.RTC_Minutes);
//		printf("%d \n", RTC_StampTimeStruct.RTC_Seconds);
