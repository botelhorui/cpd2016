#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>

#include <unistd.h>

//#define MPI_DEBUG 1

//
int DTASKS;

// Binary Tree distributed load balancers.
bool isLeaf;
int id, nprocs;
int parent, leftChild, rightChild;

MPI_Status status;
MPI_Request req;

enum MESSAGE_TYPE {WORK, BEST, NEED_WORK, STOPPED, LAST};

int DIV = 4;
int length; //
int needs_work[2];
int interval[2];
int child_stopped[2]; // send STOPPED to child
int stopped;
int i;
int requested_work;
int *proc_stopped;
//

int initConstants(){
	isLeaf = id >= (nprocs / 2);
	if(!isLeaf){
		leftChild = id * 2 + 1;
		rightChild = id * 2 + 2;
	}
	parent = (id-1) / 2;
	needs_work[0] = needs_work[1] = 0;
	child_stopped[0] = child_stopped[1] = 0;
	stopped = 0;
	requested_work = 0;
	DTASKS = (int)log2(nprocs*4);
}

/*
void int2arr(bool* vars, int n){
	memset(vars, 0, N);
	for(int j=1; n > 0; j++){
		vars[j] = (n & 1);
		n = n >> 1;
	}
}
*/

void sendWork(int &i, int child){
	// stop child if the work that can be given is zero
	if(length == 0){
		MESSAGE_TYPE m = STOPPED;
		MPI_Isend(&m, 1, MPI_INT, child, WORK, MPI_COMM_WORLD, &req);
		child_stopped[child == leftChild?0:1] = 1;
		return;
	}
	int newInterv[2];
	newInterv[0] = i;
	newInterv[1] = i + length;
	i += length;
	if(newInterv[1] >= interval[1]){
		newInterv[1] = interval[1];
		i = interval[1];		
		if(id != 0)
			MPI_Isend(&i, 1, MPI_INT, parent, NEED_WORK, MPI_COMM_WORLD, &req);
	}
	if(newInterv[1] > newInterv[0]){
		needs_work[child == leftChild ? 0 : 1] = 0;
		MESSAGE_TYPE m = WORK;
		MPI_Isend(&m, 1, MPI_INT, child, WORK, MPI_COMM_WORLD, &req);
		MPI_Isend(newInterv, 2, MPI_INT, child, WORK, MPI_COMM_WORLD, &req);
	}else{ // stop the child if the the work interval is void
		MESSAGE_TYPE m = STOPPED;
		MPI_Isend(&m, 1, MPI_INT, child, WORK, MPI_COMM_WORLD, &req);
		child_stopped[child == leftChild?0:1] = 1; 
	}	
}

int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	proc_stopped = (int*)malloc(sizeof(int)*nprocs);
	memset(proc_stopped,0,nprocs);
	proc_stopped[0] = 1;

	if(argc != 2){
		if(id == 0)
			printf("Error, usage: %s <input>\n", argv[0]);
		MPI_Finalize();
		exit(0);
	}

	//printf("nprocs: %d, me: %d\n", nprocs, id);

	initConstants();

	if(id == 0){		
		interval[0] = 0;
		interval[1] = 1 << DTASKS;
		#ifdef MPI_DEBUG
		printf("Root: interval[1]:%d\n",interval[1]);
		#endif
	}else{
		//MESSAGE_TYPE m;
		//MPI_Recv(&m, 1, MPI_INT, parent, MPI_COMM_WORLD);
		MPI_Recv(interval, 2, MPI_INT, parent, WORK, MPI_COMM_WORLD, &status);
		#ifdef MPI_DEBUG
		printf("Process %d: Received work from %d: task %d to task %d\n", id, status.MPI_SOURCE, interval[0], interval[1]);
		#endif
	}
	length = (interval[1] - interval[0]) / DIV;
	// if length equals 0 then interval differente is less than DIV
	if(!isLeaf){
		int interval2[2];
		interval2[0] = interval[0];
		interval2[1] = interval[0] + length;
		MPI_Send(interval2, 2, MPI_INT, leftChild, WORK, MPI_COMM_WORLD);
		interval[0] += length;

		interval2[0] = interval[0];
		interval2[1] = interval[0] + length;
		MPI_Send(interval2, 2, MPI_INT, rightChild, WORK, MPI_COMM_WORLD);
		interval[0] += length;
	}

	/// Read INPUT
	//input(argv[1]);

	// TODO: Send work when remaining interval is smaller that task size
	//		 Use nodes != 2^k - 1
	// 		 Smaller inputs corner case	
	i = interval[0];
	for(;;){ 
		// Work loop
		if(i < interval[1]){
			//bool vars[MAX_VARS];
			//int2arr(vars, i);
			//branch(1, vars);
			printf("Process %d i:%d\n",id,i);
			i++;
			//if(id == 0)
				sleep(1);
		}else{ // no more tasks
			if(id==0){
				stopped = true;				
			}else{
				if(stopped){
					if(isLeaf){
						#ifdef MPI_DEBUG
						printf("Process %d (leaf) stopped\n",id);
						#endif
						MESSAGE_TYPE m = LAST;
						MPI_Isend(&m, 1, MPI_INT, 0, NEED_WORK, MPI_COMM_WORLD,&req);
						break;
					}else{
						if(child_stopped[0] && child_stopped[1]){
							#ifdef MPI_DEBUG
							printf("Process %d (not leaf) stopped\n",id);
							#endif
							break;
							MESSAGE_TYPE m = LAST;
							MPI_Isend(&m, 1, MPI_INT, 0, NEED_WORK, MPI_COMM_WORLD,&req);
						}
					}
				}
				
				if(!requested_work){
					// request work
					MESSAGE_TYPE m = NEED_WORK;
					#ifdef MPI_DEBUG
					printf("Process %d requesting work to parent %d\n",id, parent);
					#endif
					MPI_Isend(&m, 1, MPI_INT, parent, NEED_WORK, MPI_COMM_WORLD,&req);
					//MPI_Request_free(&req);
					requested_work = 1;
				}				
			}			
		}
		// Root waits for all proccesses to terminate
		if(stopped){
			int sum = 0;
			for(int i = 0; i < nprocs; i++){
				sum += proc_stopped[i];
			}
			if(sum == nprocs){
				if(child_stopped[0] && child_stopped[1]){
					//#ifdef MPI_DEBUG
					printf("Root: all processes stopped. Exiting\n");
					//#endif
					break;
				}
				// print best value
			}
		}
			


		// Generic loop that receives any message
		MESSAGE_TYPE m;			
		int hasReceived;
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &hasReceived, &status);
		if(hasReceived){
			MPI_Recv(&m, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,&status);		
			#ifdef MPI_DEBUG
			printf("Process %d received message %d from %d\n",id,m,status.MPI_SOURCE);
			#endif

			if(m == LAST){
				proc_stopped[status.MPI_SOURCE] = 1;				
			}
			else if(m == BEST){
				/*
				int nbest;
				MPI_Recv(&nbest, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
				if(nbest > best)
					best = nbest;
					*/
			}else if(m == NEED_WORK){
				int child = status.MPI_SOURCE;	
				needs_work[child == leftChild?0:1] = 1; 
				if(stopped){
					m = STOPPED;
					MPI_Isend(&m, 1, MPI_INT, child, STOPPED, MPI_COMM_WORLD, &req);
					child_stopped[child == leftChild?0:1] = 1; 
				}else if(length > 0 ){								
					sendWork(i, child);
				}			
			}else if(m == STOPPED){
				stopped = 1;
			}else if(m == WORK){
				requested_work = 0;
				int newInterv[2];
				MPI_Recv(&newInterv, 2, MPI_INT, parent, WORK, MPI_COMM_WORLD, &status);
				interval[0] = newInterv[0];
				interval[1] = newInterv[1];
				#ifdef MPI_DEBUG
				printf("Process %d: Received work from %d: task %d to task %d\n", id, status.MPI_SOURCE, interval[0], interval[1]);
				#endif
				i = interval[0];
				
				if(!child_stopped[0] && needs_work[0])
					sendWork(i, leftChild);
				if(!child_stopped[1] && needs_work[1])
					sendWork(i, rightChild);					
			}
		}
	}
	
	free(proc_stopped);
	MPI_Finalize();
}
