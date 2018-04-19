#include "communication.h"

void USART3Init() {
	GPIOC->MODER |= GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1;  //set C10 C11 pins in alternate funct mode
	GPIOC->AFR[1] = (GPIOC->AFR[1] &~(GPIO_AFRH_AFRH2 |
			GPIO_AFRH_AFRH3)) | (1 << (2*4)) | (1 << (3 * 4));//choose alternate funct for C10 - TX, C11 - RX

	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;  //enable clock for USART3
	USART3->BRR = 0x341;//set baud rate
	USART3->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE; //enable USART3, USART3 receive and transmit and
	while((USART3->ISR & USART_ISR_TC) != USART_ISR_TC);//wait until transmisson complete flag isn't set
	USART3->ICR |= USART_ICR_TCCF; //clear the transmission complete flag
	USART3->CR1 |= USART_CR1_RXNEIE;//enable interrupts from USART3 receive
}

void USART1Init() {
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN; //enable clock for USART1
	GPIOA->MODER = (GPIOA->MODER &
				~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10))\
				| (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);  //set A9 A10 pins in alternate funct mode
	GPIOA->AFR[1] = (GPIOA->AFR[1] &~(GPIO_AFRH_AFRH1 |
				GPIO_AFRH_AFRH2)) | (1 << (1*4)) | (1 << (2 * 4));  //choose alternate funct for A9 - TX, A10 - RX

	USART1->BRR = 0x341;//set baud rate
	USART1->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE; //enable USART3, USART3 receive and transmit and
	while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC);//wait until transmisson complete flag isn't set
	USART1->ICR |= USART_ICR_TCCF; //clear the transmission complete flag
	USART1->CR1 |= USART_CR1_RXNEIE;//enable interrupts from USART3 receive
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

void sendToAndroid(int numbStrikes) {
	printf("Number of real strikes: %d\n", numbStrikes);
	char temp[100] = "";
	sprintf(temp, "%u;", numbStrikes);
	USARTSendString(temp);
}


char* receiveFromArduino(char delimiter[]){
	char ch;
	char dist[4];
	int i = 0;

	while (ch != delimiter[0]) {
		if((USART3->ISR & USART_ISR_RXNE) == USART_ISR_RXNE) { //check whether thre is new data in the receive register
			ch = (USART3->RDR);////read data from receive register
		}
	}
	while (ch != delimiter[1]) {
		if((USART3->ISR & USART_ISR_RXNE) == USART_ISR_RXNE) { //check whether thre is new data in the receive register
			dist[i] = (USART3->RDR);////read data from receive register
			ch = dist[i];
			i++;
		}
	}
	dist[i-1] = '\0';
	USARTSendString(dist);
	return dist;
}
