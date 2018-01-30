#include "data_processing.h"

//count how many real strikes were made based on distances data
int processData(int dataTest[]){
	int TDS = 0;
	int BDS = 1;
	int UP_M = 2;
	int DOWN_M = 3;




//	printf("Data size: %d",sizeof(data));

	//start

	int tempList[102];
	int resultList[100];

	tempList[0] = 0;

	int padding = 1;

	for(int i = 0; i < 100; i++){
		tempList[i + padding] = dataTest[i];
	}

	tempList[101] = 0;



	int u = 0;
	while (u != 100){
		int tempSum = 0;

		tempSum = ((tempList[u]*1.0/3.0) + (tempList[u+1]*1.0/3.0) + (tempList[u+2]*1.0/3.0));
		resultList[u] = tempSum;
		u += 1;
	}

	for (int i = 0; i < 100; i++){
		printf("%d\n", resultList[i]);
	}

	//finish


	int states[1000][2];
	int cur_state = TDS;
	int count = 1;
	int prev_dist = resultList[0];

	states[0][0] = cur_state;
	states[0][1] = prev_dist;


	for(int i = 0; i < 100; i++){
//		printf("%d\n", count);
		if (cur_state == TDS){
			if (prev_dist > resultList[i]) {
				cur_state = DOWN_M;
			}
		}
		else if(cur_state == BDS){
			if (prev_dist < resultList[i]){
				cur_state = UP_M;
			}
		}
		else if (cur_state == UP_M){
            if (prev_dist == resultList[i]){
            	cur_state = TDS;
            	states[count][0] = cur_state;
            	states[count][1] = prev_dist;
            	count += 1;
            }
            else if(prev_dist > resultList[i]){
            	states[count][0] = TDS;
            	states[count][1] = prev_dist;
//            	printf("TDS = %d\n", states[count][0]);
//            	printf("Distance = %d\n", states[count][1]);
            	cur_state = DOWN_M;
            	count += 1;
            }
		}
		else if(cur_state == DOWN_M){
			if (prev_dist == resultList[i]){
				cur_state = BDS;
				states[count][0] = cur_state;
				states[count][1] = prev_dist;

				count += 1;
			} else if(prev_dist < resultList[i]){
				states[count][0] = BDS;
				states[count][1] = prev_dist;
//				printf("BDS = %d\n", states[count][0]);
//				printf("Distance = %d\n", states[count][1]);
				cur_state = UP_M;
				count += 1;
			}
		}

	prev_dist = resultList[i];


	}

	int dlength = 0;
	for(int c = 0; c < count-1; c+=2){
		dlength+=1;
	}
//	printf("%d\n", dlength);


	int deltas[dlength];

	int countd = 0;
	for(int c = 0; c < count-1; c+=2){
		deltas[countd] = abs(states[c][1] - states[c + 1][1]);
		countd+=1;
	}

	int max = -1000;

	for(int d = 0; d < countd; d+=1){
		if(deltas[d] > max){
			max = deltas[d];
		}
	}
	printf("MAX: ");
	printf("%d\n", max);

	int number = 0;
	for(int e = 0; e < countd; e+=1){
		printf("%d\n", deltas[e]);
		if((deltas[e]>=max-5) && (deltas[e]<=max+5)){
			number ++;
		}
	}
	printf("Strikes: %d\n", number);
	return number;

}

void sendToAndroid(int numbStrikes) {
	printf("Number of real strikes: %d\n", numbStrikes);
	char temp[100] = "";
	sprintf(temp, "%u;", numbStrikes);
	USARTSendString(temp);
}
