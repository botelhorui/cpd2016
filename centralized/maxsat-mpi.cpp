#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>

#include <unistd.h>

#define MPI_DEBUG 1
enum MESSAGE_TYPE {WORK, NEED_WORK, BEST, BEST_ASSIGNMENT, STOPPED};
//
int DTASKS;

int id, nprocs;

MPI_Status status;
MPI_Request req;
MESSAGE_TYPE m;

int DIV = 8;
int length; //
int interval[2];
int *proc_stopped;

void one_worker(){
	//TODO call serial branch
	DTASKS = (int)log2(nprocs*4);
}

void receive_best_assignment(){
	//
	//TODO
	MPI_Send(&interval[1], 1, MPI_INT, 0, BEST_ASSIGNMENT, MPI_COMM_WORLD);
}

int all_workers_finished(int* proc_stopped){
	int sum = 0;
	for(int i = 0; i < nprocs; i++){
		sum += proc_stopped[i];
	}
	return sum == nprocs;
}

void centralized_load_balancer(){
	proc_stopped = (int*)malloc(sizeof(int)*nprocs);
	memset(proc_stopped,0,nprocs);
	proc_stopped[0] = 1;

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
			proc_stopped[status.MPI_SOURCE] = 1;			
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

		if(all_workers_finished(proc_stopped))
			break;			
	}
	free(proc_stopped);
}

void send_best_assignment(){
	//TODO
}
void centralized_worker(){
	while(1){
		// get job
		m = NEED_WORK;
		MPI_Send(&m, 1, MPI_INT, 0, NEED_WORK, MPI_COMM_WORLD);
		MPI_Recv(interval, 2, MPI_INT, 0, WORK, MPI_COMM_WORLD, &status);
		printf("Process %d: Received tasks: %d to task %d\n", id, interval[0], interval[1]);
		if(interval[0] == interval[1]){ // equals means no more work
			//send best assignment
			send_best_assignment();			
			break;
		}		
		// finish job
		for(int i = interval[0]; i < interval[1]; i++){
			// do one interation
			printf("P %d i %d\n",id,i);			
			//check for best
			//TODO use probe
		}		
	}
}

int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);	

	if(argc != 2){
		if(id == 0)
			printf("Error, usage: %s <input>\n", argv[0]);
		MPI_Finalize();
		exit(0);
	}	

	if(nprocs == 1){
		one_worker();
	}else if(id == 0){
		centralized_load_balancer();
	}else{
		centralized_worker();
	}
	printf("Process %d finished\n", id);
	
	MPI_Finalize();
}
