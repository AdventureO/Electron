#include "data_processing.h"

int processData(int dataTest[], int size){
	int TDS = 0;
	int BDS = 1;
	int UP_M = 2;
	int DOWN_M = 3;
	int states[1000][2];
	int cur_state = TDS;
	int count = 1;
	int prev_dist = dataTest[0];

	states[0][0] = cur_state;
	states[0][1] = prev_dist;


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
            	states[count][0] = cur_state;
            	states[count][1] = prev_dist;
            	count += 1;
            }
            else if(prev_dist < dataTest[i]) {
            	states[count][0] = TDS;
            	states[count][1] = prev_dist;
            	cur_state = DOWN_M;
            	count += 1;
            }
		}
		else if(cur_state == DOWN_M){
			if (prev_dist == dataTest[i]){
				cur_state = BDS;
				states[count][0] = cur_state;
				states[count][1] = prev_dist;
				printf("Distance = %d\n", states[count][1]);

				count += 1;

			} else if(prev_dist > dataTest[i]){
				states[count][0] = BDS;
				states[count][1] = prev_dist;

				printf("Distance = %d\n", states[count][1]);
				cur_state = UP_M;

			}

		}

	prev_dist = dataTest[i];


	}
	int max = -1000;
	for(int c = 0; c < count; c+=1) {
		if(c%2 != 0){
			if(states[c][1] > max) {
				max=states[c][1];

			}
		}
	}
	printf("MAX: ");
	printf("%d\n", max);

	int number = 0;
	int delta = 3;
	for(int c = 0; c < count; c+=1) {
		if(c%2 != 0){
			if((states[c][1] >= max-delta)||(states[c][1] >= max+delta)) {
				number++;
			}
		}
	}
	printf("Strikes: %d\n", number);
	return max;

}
