#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>

int main(int argc, char *argv[]){
	int MAX_ITS = 1;
	int size, rank, rank_up, rank_down, err;
	double starttime, stoptime, diff;
	MPI_Init(NULL, NULL);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &size);
	MPI_Comm_rank(comm, &rank);

	MPI_Status status;
	MPI_Request recv_request, send_request;
	

	//define custom structure
	struct rank_struct {
		int int_rank;
		double power_rank;
	};

	struct rank_struct send_r = {rank, pow(rank+1, 2)}; 
	struct rank_struct recv_r, local_r;
	local_r = send_r;

	MPI_Datatype r_struct;
	int count, blocklengths[2];
	MPI_Datatype types[2];
	MPI_Aint displacements[2], tdisp[2];
	count = 2;
	blocklengths[0] = 1;
	types[0] = MPI_INT; // just an MPI INT, no vectors so blocklength is 1
	blocklengths[1] = 1;
	types[1] = MPI_DOUBLE; // just an MPI DOUBLE, not a vector, again length 1

	MPI_Get_address(&send_r.int_rank, &displacements[0]); // make sure to reference the same instance
	MPI_Get_address(&send_r.power_rank, &displacements[1]);
	tdisp[0] = 0; // always zero
	tdisp[1] = displacements[1] - displacements[0];
	MPI_Type_create_struct(count, blocklengths, tdisp, types, &r_struct);
	MPI_Type_commit(&r_struct); //commits datatype


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

	for(int i = 0; i < size-1; i++){ // iterate unitl all have each
		// issue non blocking receive
		MPI_Irecv(&recv_r, 1, r_struct, rank_down, 0, comm_cart, &recv_request);
		// issue non blocking send 
		MPI_Issend(&send_r, 1, r_struct, rank_up, 0, comm_cart, &send_request);
	
		MPI_Wait(&recv_request, &status);
		local_r.int_rank += recv_r.int_rank;
		local_r.power_rank += recv_r.power_rank;
		send_r = recv_r;
		MPI_Wait(&send_request, &status);		
	}
	stoptime = MPI_Wtime();
	diff = stoptime-starttime;
	printf("%d, Time %f, per iteration: %f\n", rank, diff, diff/(double) MAX_ITS);
	printf("Process %d, up %d, down %d\nInt sum: %d, Power sum %f\n", rank, rank_up, rank_down, 
			local_r.int_rank, local_r.power_rank);
	MPI_Comm_free(&comm_cart);
	MPI_Finalize();
	return 0;
}
