#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>

#define DIM_NUMBER 2
int main(int argc, char *argv[]){
//	if (argc < 2) { exit(-1); }
//	int MAX_ITS = (int) strtol(argv[1], NULL, 10);
	int size, rank;

	// up-bottom -> y axis, left-right -> x axis
	MPI_Init(NULL, NULL);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &size);
	MPI_Comm_rank(comm, &rank);

	int to_send = rank;	

	// create the topology of the cyllinder
	MPI_Comm comm_cart;

	int dims[DIM_NUMBER], periods[DIM_NUMBER];
	// periodic in x, non-periodic in y
	periods[0] = 1;
	periods[1] = 0;

	// zero dimensions so Dims_create works properly
	dims[0] = 0;
	dims[1] = 3;
	// create dimensions
	if (size%3 != 0) { perror("Invalid dims, cyllinder has 3 layers\n"); exit(-1); }
	MPI_Dims_create(size, DIM_NUMBER, dims);
	// CAN ONLY CALL MODULO DIM NUMBER 
	printf("DIMS: %d, %d\n", dims[0], dims[1]);
	// create topology of 2d cyllinder
	MPI_Cart_create(comm, DIM_NUMBER, dims, periods, 0, &comm_cart);
	
	// cut out two dimensions to reduce them independently
	MPI_Comm vertical_slice, horizontal_slice;
	int rem_dims[DIM_NUMBER];
	// keep only x dimension
	rem_dims[0] = 1;
	rem_dims[1] = 0;
	MPI_Cart_sub(comm_cart, rem_dims, &horizontal_slice);
	//keep only y dimension
	rem_dims[0] = 0;
	rem_dims[1] = 1;
	MPI_Cart_sub(comm_cart, rem_dims, &vertical_slice);

	int up, down, left, right;
	MPI_Cart_shift(comm_cart, 0, 1, &left, &right);
	MPI_Cart_shift(comm_cart, 1, 1, &down, &up);
	printf("%d, up: %d, down: %d, left: %d, right %d\n", rank, up, down, left, right);
	int vertical_sum, horizontal_sum;
	MPI_Allreduce(&to_send, &vertical_sum, 1, MPI_INT, MPI_SUM, vertical_slice);
	MPI_Allreduce(&to_send, &horizontal_sum, 1, MPI_INT, MPI_SUM, horizontal_slice);

	printf("%d, horizontal %d, vertical %d\n", rank, horizontal_sum, vertical_sum);
	MPI_Comm_free(&comm_cart);
	MPI_Finalize();
	return 0;
}
