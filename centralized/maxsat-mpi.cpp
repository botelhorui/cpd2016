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
typedef long long int ll;

//
int DTASKS;

int id, nprocs;

MPI_Status status;
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

void receive_best_assignment(int t){
	//
	//TODO
	//MPI_Send(&interval[1], 1, MPI_INT, 0, BEST_ASSIGNMENT, MPI_COMM_WORLD);
	ll* set = (ll*) malloc(set_size * sizeof(ll));

	MPI_Recv(set, set_size, MPI_LONG_LONG_INT, t, BEST_ASSIGNMENT, MPI_COMM_WORLD, &status);
	count_best += set[0];
	for(int i=1; i<NUM_VARS; i++)
		if( (set[i/64+1] >> (i%64)) & 1)
			bestAssignment[i] = true;
	
}

void send_best_assignment(){
	//TODO
	ll* set = (ll*) malloc(set_size * sizeof(ll));
	memset(set, 0, set_size * sizeof(ll));
	set[0] = count_best;
	if(count_best > 0){
		for(int i=1; i<NUM_VARS; i++)
			if(bestAssignment[i])
				set[i/64+1] |= 1 << (i%64);
	}
	MPI_Send(set, set_size, MPI_LONG_LONG_INT, 0, BEST_ASSIGNMENT, MPI_COMM_WORLD);
	
}

inline double min(double a, double b){
	return a < b ? a : b;
}

inline double max(double a, double b){
	return a > b ? a : b;
}

void centralized_load_balancer(){
	proc_stopped = 1;

	interval[0] = 0;
	interval[1] = 1 << DTASKS;
	double n_tasks = interval[1];
	printf("length %d\n",length);

	printf("Root: interval[1]:%d\n",interval[1]);
	int sender;
	// centralized load balancer
	MPI_Request req;
	while(1){
		// recv message
		MPI_Recv(&m, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		int tag = status.MPI_TAG;
		sender = status.MPI_SOURCE;		
		if(tag == NEED_WORK){		
			if(interval[0] >= interval[1]){
				interval[0] = interval[1];
				MPI_Send(interval, 2, MPI_INT, sender, WORK, MPI_COMM_WORLD);
				receive_best_assignment(sender);
				proc_stopped++;
			}else{
				int interval2[2];
				int length = max( (int)(n_tasks/nprocs * 0.5 * pow(2, -interval[0] / n_tasks)), 1);
				interval2[0] = interval[0];
				interval2[1] = interval[0] + length;
				interval[0] = interval2[1];
				if(interval2[1] >= interval[1]){
					interval2[1] = interval[1];
				}
				MPI_Send(interval2, 2, MPI_INT, sender, WORK, MPI_COMM_WORLD);
			}

			if(proc_stopped == nprocs)
				break;	
		}else if(tag == BEST){
			if(m > best){
				best = m;
				for(int i=1; i<nprocs; i++)
					MPI_Isend(&best, 1, MPI_INT, i, BEST, MPI_COMM_WORLD, &req);
			}
		}else{
			printf("Wrong data received: %d\n", tag);
		}
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
			bool* vars = new bool[NUM_VARS+1];
			int2arr(vars, i);
			branch(DTASKS+1, vars);
			// do one iteration
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
	
	double start = MPI_Wtime();
	//DTASKS = (int)log2(nprocs*4);
	DTASKS = (int)(0.25 * NUM_VARS);

	if(nprocs == 1){
		one_worker();
	}else if(id == 0){
		centralized_load_balancer();
	}else{
		centralized_worker();
	}

	double end = MPI_Wtime();
	
	if(id == 0){
		printf("Elapsed time: %lf\n", end-start);
		print_result();
	}

	
	MPI_Finalize();

}
