#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "pgmio.h"




int main(int argc, char *argv[]){
	if (argc < 3) { perror("Invalid number of args\n"); exit(-1); }
	char *filename = argv[1];
	int MAX_ITER = (int) strtol(argv[2], NULL, 10);
	if (MAX_ITER <= 0) { perror("Invalid number of iterations\n"); exit(-1);}
	int size_x, size_y;

	pgmsize(filename, &size_x, &size_y);

	printf("Img size: %d, %d\n", size_x, size_y);

	

	// initialize MPI 
	MPI_Init(NULL, NULL);
	MPI_Comm comm = MPI_COMM_WORLD;

	int rank, size;
	
	MPI_Comm_size(comm, &size);
	MPI_Comm_rank(comm, &rank);

	// t is img array, malloc this
	// divide image array into parts 
	// each column ie. i in arr[i][j] has a separate chunk
	// C is column major so stores columns contigunously
	
	int process_slice = size_x/size;
	
	double master_buffer[size_x][size_y],
	       slave_buffer[process_slice][size_y],
	       reconstructed_array[process_slice][size_y],
	       halo_array[process_slice+2][size_y+2];


	// define new Cartesian topology, 1D rank up, rank down 0 | 1 | 2 | 3 ... | P (in C
	// column major order), P is a process rank
	MPI_Comm comm_cart;
	MPI_Status status;
	int dims[1], periods[1];
	periods[0] = 0;
	dims[0] = 0; // set to 0 in order to allow degrees of freedom
	MPI_Dims_create(size, 1, dims);
	MPI_Cart_create(comm, 1, dims, periods, 0, &comm_cart);
	// calculate the neighbours
	int rank_left, rank_right;
	MPI_Cart_shift(comm_cart, 0, 1, &rank_left, &rank_right);
	printf("%d, left: %d, right: %d\n", rank, rank_left, rank_right);

	// set halo array to 255 
	bzero(halo_array, sizeof(halo_array));
	//pgmwrite("halo", halo_array, process_slice+2, size_y+2);	

	// read only on root -> 0 process	
	if (!rank) pgmread(filename, master_buffer, size_x, size_y);
//	printf("%d, %d\n", rank, process_slice);
	MPI_Scatter(master_buffer, process_slice*size_y, MPI_DOUBLE, slave_buffer,\
		       	process_slice*size_y, MPI_DOUBLE, 0, comm);

	for (int k = 0; k < MAX_ITER; k++){
		for (int i = 1; i < process_slice+1; i++){
			for (int j=1; j < size_y+1; j++){
				reconstructed_array[i-1][j-1] =
					0.25*(halo_array[i-1][j]+
					      halo_array[i+1][j]+
					      halo_array[i][j-1]+
					      halo_array[i][j+1]-
					      slave_buffer[i-1][j-1]);
			}
		}
		for (int i = 1; i < process_slice+1; i++){
			for (int j=1; j < size_y+1; j++){
				halo_array[i][j] = reconstructed_array[i-1][j-1];
			}
		}
		MPI_Ssend(halo_array[process_slice], size_y, MPI_DOUBLE, rank_right, 0, comm_cart);
		MPI_Recv(halo_array[0], size_y, MPI_DOUBLE, rank_left, 0, comm_cart, &status);
	        MPI_Ssend(halo_array[1], size_y, MPI_DOUBLE, rank_left, 0, comm_cart);
		MPI_Recv(halo_array[process_slice+1], size_y, MPI_DOUBLE, rank_right, 0, comm_cart, &status);	
	}

	MPI_Gather(reconstructed_array, process_slice*size_y, MPI_DOUBLE, master_buffer,\
		       	process_slice*size_y, MPI_DOUBLE, 0, comm);
	if (!rank) pgmwrite("rec2", master_buffer, size_x, size_y);	
	
	MPI_Finalize();
	return 0;
}
