#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define MAX_MSG_IT 100

int main(int argc, char *argv[]){
	FILE *fp;
	fp = fopen("timing_results.txt", "w");
	if (fp == NULL) exit(-1);
	if (argc < 2) { perror("No argument... Insert iterations"); exit(-1); }
	long iterations = strtol(argv[1], NULL, 10);
	printf("Performing %ld iterations\n", iterations);
	int size, rank, err;
	double starttime, stoptime, diff;
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Status status;
	long int msg_size = 10;
	int *ball;

	for (int k = 1; k < MAX_MSG_IT; k++){
		msg_size *= 2;
	//	printf("ITERTATION %d\n", k);
       		ball = (int *) malloc(msg_size*sizeof(int));
	if (!rank){
		for(int i = 0; i < msg_size; i++){
			ball[i] = i;	
		}
		for (long int j=0; j<iterations; j++){
			err = MPI_Ssend(ball, msg_size, MPI_INT, 1, 0, comm);
			if (err < 0) perror("Send err");
			err = MPI_Recv(ball, msg_size, MPI_INT, 1, MPI_ANY_TAG, comm, &status);
			if (err < 0) perror("Recv err");
		}
	}
	if (rank == 1){
		starttime = MPI_Wtime();
		for (long int j=0; j<iterations; j++){
			err = MPI_Recv(ball, msg_size, MPI_INT, 0, MPI_ANY_TAG, comm, &status);
			if (err < 0) perror("Recv err");
			err = MPI_Ssend(ball, msg_size, MPI_INT, 0, 0, comm);
			if (err < 0) perror("Send err");
		}
		stoptime = MPI_Wtime();
		diff = stoptime-starttime;
		//printf("Elapsed time: %f\nAvg time: %f\n", diff, (double) diff/iterations);
		fprintf(fp, "%d, %f, %f", 1, 0.3, 0.2);

	}
		free(ball);
	}
	fclose(fp);
	MPI_Finalize();
	return 0;
}
