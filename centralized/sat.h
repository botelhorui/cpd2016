#include <iostream>
#include <algorithm>
#include <string.h>
#include <omp.h>
#include <fstream>

using namespace std;

#define MAX_CLAUSES (2 << 16) + 1
#define MAX_VARS 128
#define MAX_VARS_PER_CLAUSE 21


// Load balancing
enum MESSAGE_TYPE {WORK, NEED_WORK, BEST, BEST_ASSIGNMENT, STOPPED};

int NUM_VARS, NUM_CLAUSES, best = 0, count_best = 0;
bool bestAssignment[MAX_VARS];
int* clauses[MAX_CLAUSES];
int set_size;

int calcClauses(int vi, bool* vars){
	int sum = 0;
	for(int i=0; i < NUM_CLAUSES; i++){
		for(int v=1; v <= clauses[i][0]; v++){
			int c = clauses[i][v];
			int a = abs(c);
			if(c < 0){
				if(!vars[a]){
					sum++;
					break;
				}	
			}else{
				if(vars[a]){
					sum++;
					break;
				}
			}
			
		}
	}
	return sum;
}

// unsatisfiable closed clauses
int calcClosedClauses(int vi, bool* vars){
	int sum = 0;
	for(int i=0; i < NUM_CLAUSES; i++){
		int v;
		for(v=1; v <= clauses[i][0]; v++){
			int c = clauses[i][v];
			int a = abs(c);
			if(a >= vi){
				break;
			}else if(c < 0){
				if(!vars[a]) break;
			}else{
				if( vars[a]) break;
			}
			
		}
		if(v > clauses[i][0]) sum++;
	}
	return sum; 
}

void int2arr(bool* vars, int n){
	memset(vars, 0, NUM_VARS+1);
	for(int j=1; n > 0; j++){
		vars[j] = (n & 1);
		n = n >> 1;
	}
}

MPI_Request req, req2;

void checkForBest(){ // receive broadcast from root
	int n_best, flag=0;
	MPI_Status status;
	MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
	if(flag){
		MPI_Recv(&n_best, 1, MPI_INT, 0, BEST, MPI_COMM_WORLD, &status);
		if(n_best > best){
			best = n_best;
			count_best = 0;
		}
	}
	
}


void sendBest(){ // send best to master
	//MPI_IBcast(&best, 1, MPI_INT, id, MPI_COMM_WORLD);
	MPI_Isend(&best, 1, MPI_INT, 0, BEST, MPI_COMM_WORLD, &req);
}

// vars saves variables assignments
void branch(int vi, bool* vars){
	checkForBest();
	if(vi >= NUM_VARS+1){
		int sum = calcClauses(vi, vars);
		if(sum > best){
			best = sum;
			count_best = 1;
			memcpy(bestAssignment, vars, (NUM_VARS+1) * sizeof(bool));
			sendBest();
			/*for(int i=1; i <= N; i++){
				bestAssignment[i] = vars[i];
			}*/
		} else if(sum == best){
			count_best++;
		}
		return;
	}
	if(NUM_CLAUSES - calcClosedClauses(vi,vars) < best){
 		return;
	}

	vars[vi] = true;
	branch(vi+1, vars);
	vars[vi] = false;
	branch(vi+1, vars);
}


void read_input(char *file){
	ifstream in(file);
	cin.rdbuf(in.rdbuf());

	cin >> NUM_VARS >> NUM_CLAUSES;
	
	for(int i = 0; i < NUM_CLAUSES; i++){
		// 1 for size, 1 for last zero
		clauses[i] = new int[MAX_VARS_PER_CLAUSE];
		for(int v = 1;;v++){
			cin >> clauses[i][v];
			if(clauses[i][v] == 0){
				clauses[i][0]= v - 1;
				break;
			}
		}
	}

	set_size = (NUM_VARS+64) / 64 + 1;
}

void print_result(){
	cout << best << " " << count_best << endl;
	for(int i=1; i <= NUM_VARS; i++){
		cout << (bestAssignment[i] ? i: -i);
		if(i < NUM_VARS) cout << " ";
	}
	cout << endl;
}