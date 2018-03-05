#include "stm32f0xx.h"

void USART1Init();
void USART3Init();
void USARTSendString(char* string);
void USARTSend(char ch);
void sendToAndroid(int numbStrikes);
char* receiveFromArduino(char delimiter[]);
