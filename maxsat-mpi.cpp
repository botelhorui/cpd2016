#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include "sat.h"

bool isLeaf;
int id, nprocs;
int parent, leftChild, rightChild;

MPI_Status status;
MPI_Request req;

enum MESSAGE_TYPE {SIZE, WORK, BEST, NEED_WORK, STOPPED};

const int DIV = 8;
int length;
int needs_work[2];
int interval[2];

int initConstants(){
	isLeaf = id >= (nprocs / 2);
	if(!isLeaf){
		leftChild = id * 2 + 1;
		rightChild = id * 2 + 2;
	}
	parent = (id-1) / 2;
	needs_work[0] = needs_work[1] = 0;
}

void int2arr(bool* vars, int n){
	memset(vars, 0, N);
	for(int j=1; n > 0; j++){
		vars[j] = (n & 1);
		n = n >> 1;
	}
}

void sendWork(int &i, int child){
	int newInterv[2];
	newInterv[0] = i;
	newInterv[1] = i + length;
	i += length;
	if(newInterv[1] >= interval[1]){
		newInterv[1] = interval[1];
		i = interval[1];
		
		if(id != 0)
			MPI_ISend(&i, 1, MPI_INT, parent, NEED_WORK, MPI_COMM_WORLD, &status);
	}
	needs_work[child == leftChild ? 0 : 1] = 1;

	MESSAGE_TYPE m = WORK;
	MPI_ISend(&m, 1, MPI_INT, child, WORK, MPI_COMM_WORLD, &status);
	MPI_ISend(newInterv, 2, MPI_INT, child, WORK, MPI_COMM_WORLD, &status);
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

	printf("nprocs: %d, me: %d\n", nprocs, id);

	initConstants();

	if(id == 0){
		printf("I AM THE ROOT\n");
		interval[0] = 0;
		interval[1] = 1 << (int)log2(nprocs);
	}else{
		//MESSAGE_TYPE m;
		//MPI_Recv(&m, 1, MPI_INT, parent, MPI_COMM_WORLD);
		MPI_Recv(interval, 2, MPI_INT, parent, WORK, MPI_COMM_WORLD, &status);
		printf("Process %d: Received work from %d: task %d to task %d\n", id, parent, interval[0], interval[1]);
	}
	length = (interval[1] - interval[0]) / DIV;
	
	if(!isLeaf){
		int m = interval[0] + length;
		
		int interval2[2];
		interval2[0] = interval[0];
		interval2[1] = interval[0] + m;
		MPI_Send(interval2, 2, MPI_INT, leftChild, WORK, MPI_COMM_WORLD);

		interval2[0] += m;
		interval2[1] += m;
		MPI_Send(interval2, 2, MPI_INT, rightChild, WORK, MPI_COMM_WORLD);

		interval[0] += m * 2;
	}

	input(argv[1]);

	// TODO: Send work when remaining interval is smaller that task size
	//		 Use nodes != 2^k - 1
	// 		 Smaller inputs corner case

	for(int i=interval[0]; i<interval[1]; ){ 
		bool vars[MAX_VARS];
		int2arr(vars, i);
		branch(1, vars);
		i++;

		MESSAGE_TYPE m;
		int f;
		MPI_IRecv(&m, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
		MPI_Test(&req, &f, &status);
		if(f){
			MPI_Wait(&req, &status);
			if(m == BEST){

				int nbest;
				MPI_Recv(&nbest, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
				if(nbest > best)
					best = nbest;
			}else if(m == NEED_WORK){
				int child = status.MPI_SOURCE;
				sendWork(i, child);
			}else if(m == STOPPED){

			}else if(m == WORK){
				int newInterv[2];
				MPI_Recv(&newInterv, 2, MPI_INT, parent, WORK, MPI_COMM_WORLD, &req);

				interval[0] = newInterv[0];
				interval[1] = newInterv[1];
				i = interval[0];
				if(needs_work[0])
					sendWork(i, leftChild);
				if(needs_work[1])
					sendWork(i, rightChild);
			}
		}

		while(i >= interval[1] - 1){

		}

	}

	MPI_Finalize();
}
