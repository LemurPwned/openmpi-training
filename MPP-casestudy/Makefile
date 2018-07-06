all: pgmio.o img_reconstruction.o 	
	mpicc pgmio.o img_reconstruction.o -o img_rec
	mpicc pgmio.o img_reconstruction_ser.o -o ser_rec
pgmio.o: pgmio.c
	mpicc -c pgmio.c  
img_reconstruction.o: img_reconstruction.c img_reconstruction_ser.c 
	mpicc -c img_reconstruction.c
	mpicc -c img_reconstruction_ser.c
