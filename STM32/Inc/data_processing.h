/*
 * data_processing.h
 *
 *  Created on: Apr 19, 2018
 *      Author: dzvinka
 */
#include "USART.h"

#ifndef DATA_PROCESSING_H_
#define DATA_PROCESSING_H_

int processData1(int dataTest[], int size);
int getNumberOfStrikes(int distance, int *TDS, int *numberOfElements, int *numberOfTDS,int *distancesArray, int lambda, char sensor);
void sendToResultAndroid(int numbStrikes, char sensor);

#endif /* DATA_PROCESSING_H_ */
