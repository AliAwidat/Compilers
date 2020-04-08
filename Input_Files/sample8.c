#include <stdio.h>

void main() {
	double i;
	double j;
	double k;
	
	i = 10;
	j = 5;
	k = 3;
	while (i >= 0) {
		k = k + 2;
		if (k > i) {
			while (j <= 10) {
				j = j + 1;
				printf("%.2f\n", j);
			}
		}
		else {
			k = k - 1;
		};
		i = i - 1;
		printf("%.2f\n", i);
	}
	return;
}