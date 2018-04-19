#include "USART.h"

void USARTInit() {
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN; //enable clock for USART1
	USART1->BRR = 0x341;  //set baud rate
	USART1->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;  //enable USART1, USART1 receive and transmit
	while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC); //wait until transmisson complete flag isn't set
	USART1->ICR |= USART_ICR_TCCF;  //clear the transmission complete flag
	USART1->CR1 |= USART_CR1_RXNEIE;  //enable interrupts from USART1 receive
}
void USARTSendString(char* string){

	uint8_t str_size = strlen(string);
	for(int i = 0; i<str_size; i++ ){
		USARTSend(string[i]);
	}
}


void USARTSend(char ch){
	USART1->TDR = ch;
	while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC);
}
