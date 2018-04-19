#include "motor_control.h"
void initMotor() {
	GPIOC->MODER |= GPIO_MODER_MODER1_0;
	GPIOC->MODER |= GPIO_MODER_MODER2_0;
}

void turnOnMotorDown() {
	GPIOC->ODR |= GPIO_ODR_1;
	GPIOC->ODR &= ~GPIO_ODR_2;
}

void turnOnMotorUp() {
	GPIOC->ODR &= ~GPIO_ODR_1;
	GPIOC->ODR |= ~GPIO_ODR_2;
}

void turnoffMotor() {
	GPIOC->ODR &= ~GPIO_ODR_1;
	GPIOC->ODR &= ~GPIO_ODR_2;
}
