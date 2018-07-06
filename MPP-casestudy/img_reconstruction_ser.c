#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pgmio.h"

int main(int argc, char *argv[]){
	if (argc < 3) { perror("Invalid number of args\n"); exit(-1); }
	char *filename = argv[1];
	int MAX_ITER = (int) strtol(argv[2], NULL, 10);
	if (MAX_ITER <= 0) { perror("Invalid number of iterations\n"); exit(-1);}
	int size_x, size_y;

	pgmsize(filename, &size_x, &size_y);

	printf("Img size: %d, %d\n", size_x, size_y);

	// t is img array, malloc this
	double t[size_x][size_y],
	       reconstructed_array[size_x][size_y],
	       halo_array[size_x+2][size_y+2];
	pgmread(filename, t, size_x, size_y);
	
	// set halo array to 255 
	bzero(halo_array, sizeof(halo_array));
	pgmwrite("halo", halo_array, size_x+2, size_y+2);	
	
	for (int k = 0; k < MAX_ITER; k++){
		for (int i = 2; i < size_x+2; i++){
			for (int j=2; j < size_y+2; j++){
				reconstructed_array[i-2][j-2] =
					0.25*(halo_array[i-1][j]+
					      halo_array[i+1][j]+
					      halo_array[i][j-1]+
					      halo_array[i][j+1]-
					      t[i-2][j-2]);
			}
		}
		for (int i = 2; i < size_x+2; i++){
			for (int j=2; j < size_y+2; j++){
				halo_array[i][j] = reconstructed_array[i-2][j-2];
			}
		}
	}
//	strcat(filename, "_rec");
	pgmwrite("rec", t, size_x, size_y);	

	return 0;
}
