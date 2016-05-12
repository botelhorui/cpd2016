#include <iostream>
#include <fstream>
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <math.h>
#include <bitset>
#include <unistd.h>
#include "sat.h"

//////////////////////

#define MPI_DEBUG 1

// Load balancing
enum MESSAGE_TYPE {WORK, NEED_WORK, BEST, BEST_ASSIGNMENT, STOPPED};
//
int DTASKS;

int id, nprocs;

MPI_Status status;
MPI_Request req;
MESSAGE_TYPE m;

int DIV = 8; // division of the main interval
int length; //
int interval[2];
int proc_stopped;
////////////////////////


void one_worker(){
	//TODO call serial branch
	DTASKS = (int)log2(nprocs*4);
}

void receive_best_assignment(){
	//
	//TODO
	MPI_Send(&interval[1], 1, MPI_INT, 0, BEST_ASSIGNMENT, MPI_COMM_WORLD);
}
void send_best_assignment(){
	//TODO
}


void centralized_load_balancer(){
	proc_stopped = 1;

	DTASKS = (int)log2(nprocs*4);
	interval[0] = 0;
	interval[1] = 1 << DTASKS;
	length = (interval[1] - interval[0]) / DIV;
	if(length == 0)
		length == 1;
	printf("length %d\n",length);

	printf("Root: interval[1]:%d\n",interval[1]);
	int sender;
	// centralized load balancer		
	while(1){
		// recv message
		MPI_Recv(&m, 1, MPI_INT, MPI_ANY_SOURCE, NEED_WORK, MPI_COMM_WORLD, &status);
		sender = status.MPI_SOURCE;		
		if(interval[0] >= interval[1]){
			interval[0] = interval[1];
			MPI_Send(interval, 2, MPI_INT, sender, WORK, MPI_COMM_WORLD);
			receive_best_assignment();
			proc_stopped++;			
		}else{
			int interval2[2];
			interval2[0] = interval[0];
			interval2[1] = interval[0]+length;
			interval[0] = interval2[1];
			if(interval2[1] >= interval[1]){
				interval2[1] = interval[1];
			}
			MPI_Send(interval2, 2, MPI_INT, sender, WORK, MPI_COMM_WORLD);
		}		

		if(proc_stopped == nprocs)
			break;			
	}
}


void centralized_worker(){
	while(1){
		// get job
		m = NEED_WORK;
		MPI_Send(&m, 1, MPI_INT, 0, NEED_WORK, MPI_COMM_WORLD);
		MPI_Recv(interval, 2, MPI_INT, 0, WORK, MPI_COMM_WORLD, &status);
		//printf("Process %d: Received tasks: %d to task %d\n", id, interval[0], interval[1]);
		if(interval[0] == interval[1]){ // equals means no more work
			//send best assignment
			send_best_assignment();			
			break;
		}		
		// finish job
		for(int i = interval[0]; i < interval[1]; i++){
			// do one interation
			//printf("P %d i %d\n",id,i);			
			//check for best
			//TODO use probe
		}		
	}
}

int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);	
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	if(argc != 2){
		if(id == 0)
			printf("Error, usage: %s <input>\n", argv[0]);
		MPI_Finalize();
		exit(0);
	}	
	read_input(argv[1]);
	printf("id %d %d %d\n",id, NUM_VARS, NUM_CLAUSES);
	if(nprocs == 1){
		one_worker();
	}else if(id == 0){
		centralized_load_balancer();
	}else{
		centralized_worker();
	}

	print_result();
	printf("Process %d finished\n", id);
	
	MPI_Finalize();
}
