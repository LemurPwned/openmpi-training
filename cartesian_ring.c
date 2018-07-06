#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>

int main(int argc, char *argv[]){
//	if (argc < 2) { exit(-1); }
//	int MAX_ITS = (int) strtol(argv[1], NULL, 10);
//	int
	int MAX_ITS = 1;
	int size, rank, rank_up, rank_down, err;
	double starttime, stoptime, diff;
	MPI_Init(NULL, NULL);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &size);
	MPI_Comm_rank(comm, &rank);

	MPI_Status status;
	MPI_Request recv_request, send_request;
	
	int value_to_send = pow(rank+1, 2);		
	int received_value, coords[1];
	int local_sum = value_to_send;

	
	// create the topology
	MPI_Comm comm_cart;
	int dims[1], periods[1];
	periods[0] = 1;
	dims[0] = 0;
	// create dimensions
	MPI_Dims_create(size, 1, dims);
	// create topology
	MPI_Cart_create(comm, 1, dims, periods, 0,  &comm_cart);
	// MPI_Cart_shift provides both sink and dest for send/receive
	MPI_Cart_shift(comm_cart, 0, 1, &rank_down, &rank_up); 
	printf("Rank up %d, rank down %d, rank %d\n", rank_up, rank_down, rank);
	starttime = MPI_Wtime();
	//for (int j = 0; j < MAX_ITS; j++){
		for(int i = 0; i < size-1; i++){ // iterate unitl all have each
			// issue non blocking receive
			MPI_Irecv(&received_value, 1, MPI_INT, rank_down, 0, comm_cart, &recv_request);
			// issue non blocking send 
			MPI_Issend(&value_to_send, 1, MPI_INT, rank_up, 0, comm_cart, &send_request);
		
			MPI_Wait(&recv_request, &status);
			local_sum += received_value;
			value_to_send = received_value;
			MPI_Wait(&send_request, &status);		
		}
	//}
	stoptime = MPI_Wtime();
	diff = stoptime-starttime;
	printf("%d, Time %f, per iteration: %f\n", rank, diff, diff/(double) MAX_ITS);
	printf("Process %d, up %d, down %d\nSum: %d\n", rank, rank_up, rank_down, local_sum);
	MPI_Comm_free(&comm_cart);
	MPI_Finalize();
	return 0;
}
