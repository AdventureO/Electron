#include "data_processing.h"

int getNumberOfStrikes(int distance, int *TDS, int *numberOfElements, int *numberOfTDS,int *distancesArray, int lambda, char sensor){
	if((distance <= *TDS + lambda)&&(distance >= *TDS - lambda)){
    		*numberOfTDS += 1;
    	}
    	if(distance < *TDS) {
    	      *TDS = distance;
    	 }



    	if(distance != *TDS) {
    		distancesArray[*numberOfElements] = distance;
    		*numberOfElements += 1;

    	}
    	if(*numberOfTDS == 5) {
    		int result = processData1(distancesArray, *numberOfElements);
    		if(result != -1000){
    			if(sensor == 'U'){
//    				printf("MAX U: %d\n", result);
    				sendToResultAndroid(result, sensor);
    			} else if(sensor == 'L'){
    				printf("MAX L: %d\n", result);
//    				sendToResultAndroid(result, sensor);
    			} else if(sensor =='I'){
    				sendToResultAndroid(result, sensor);
    			}
    		}
    		*numberOfTDS = 0;
    		*numberOfElements = 0;
    	}
    }

void sendToResultAndroid(int numbStrikes, char sensor) {
	char temp[100] = "";
	if(sensor == 'U'){
		sprintf(temp, "U:%u;", numbStrikes);
		USART_SendString(temp);
	} else if (sensor == 'L') {
		sprintf(temp, "L:%u;", numbStrikes);
		USART_SendString(temp);
	} else if (sensor == 'I') {
		sprintf(temp, "I:%u;", numbStrikes);
		USART_SendString(temp);
	}
}


int processData1(int dataTest[], int size){
    int TDS = 0;
    int BDS = 1;
    int UP_M = 2;
    int DOWN_M = 3;


    int BottomStates[1000];
    int BottomStatesCount = 0;

    int TopStates[1000];
    int TopStatesCount= 0;


    int cur_state = TDS;

    int prev_dist = dataTest[0];


    for(int i = 1; i < size; i++) {
        if (cur_state == TDS){
            if (prev_dist < dataTest[i]) {
                cur_state = DOWN_M;
            }
        }
        else if(cur_state == BDS) {
            if (prev_dist > dataTest[i]) {
                cur_state = UP_M;
            }
        }
        else if (cur_state == UP_M) {
            if (prev_dist == dataTest[i]) {
                cur_state = TDS;
                TopStates[TopStatesCount] = prev_dist;
                TopStatesCount +=1;
            }
            else if(prev_dist < dataTest[i]) {
                cur_state = DOWN_M;
                TopStates[TopStatesCount] = prev_dist;
                TopStatesCount +=1;
            }
        }
        else if(cur_state == DOWN_M){
            if (prev_dist == dataTest[i]){
                cur_state = BDS;

                BottomStates[BottomStatesCount] = prev_dist;
                BottomStatesCount +=1;


            } else if(prev_dist > dataTest[i]){

                BottomStates[BottomStatesCount] = prev_dist;
                BottomStatesCount +=1;

                cur_state = UP_M;

            }

        }

        prev_dist = dataTest[i];


    }

//    for(int i = 0; i<BottomStatesCount; i++){
//        printf("Bottom States: %d \n", BottomStates[i]);
//    }


//    for(int i = 0; i<TopStatesCount; i++){
//        printf("Top States: %d \n", TopStates[i]);
//    }

    int max = -1000;
    for(int c = 0; c < BottomStatesCount; c+=1) {
        if(BottomStates[c] > max) {
            max=BottomStates[c];
        }
    }

    return max;
}



