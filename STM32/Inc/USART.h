/*
 * USART.h
 *
 *  Created on: Apr 19, 2018
 *      Author: dzvinka
 */
#include "stm32f0xx.h"
#ifndef USART_H_
#define USART_H_
void USART_Init();
void USART_Send(char ch);

void USART_SendString(char* string);

void USART_SendInt(int num);


#endif /* USART_H_ */
