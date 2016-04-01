#include <iostream>
#include <algorithm>
#include <string.h>
#include <omp.h>
#include <math.h>

using namespace std;

#define MAX_CLAUSES (2 << 16) + 1
#define MAX_VARS 128
#define MAX_VARS_PER_CLAUSE 21

int N, C, best = 0, nbest = 0;
bool bestAssignment[MAX_VARS];
int* clauses[MAX_CLAUSES];

int D_TASKS;
int *tasks;	

int calcClauses(int vi, bool* vars){
	int sum = 0;
	for(int i=0; i < C; i++){
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
	for(int i=0; i < C; i++){
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


// vars saves variables assignments
void branch(int vi, bool* vars, int task){
	
	if(vi == N+1){
		int sum = calcClauses(vi, vars);
		if(sum >= best){ // only enter critical region if best must be modified
			#pragma omp critical 
			{
				if(sum > best){
					best = sum;
					memcpy(bestAssignment, vars, (N+1) * sizeof(bool));
					nbest = 1;
					/*for(int i=1; i <= N; i++){
						bestAssignment[i] = vars[i];
					}*/
				} else if(sum == best){
					nbest++;
				}
			}
		}
		
		return;
	}
	int b;
	#pragma omp critical
	b = best;
	if(C - calcClosedClauses(vi,vars) < b){
 		return;
	}

	if(vi != D_TASKS){
		vars[vi] = true;
		branch(vi+1, vars, (task << 1) + 1);
		vars[vi] = false;
		branch(vi+1, vars, task << 1);
	}else{
		
		if(!tasks[task]){
			tasks[task] = 1;
			/*
			int thread_id = omp_get_thread_num();
			if(thread_id << (vi-1) % 2){
				vars[vi] = false;
				branch(vi+1, vars, task << 1);
				
				vars[vi] = true;
				branch(vi+1, vars, (task << 1) + 1);
			}else{
				vars[vi] = true;
				branch(vi+1, vars, (task << 1) + 1);
				
				vars[vi] = false;
				branch(vi+1, vars, task << 1);
			}*/
			if(rand() % 2){
				vars[vi] = false;
				branch(vi+1, vars, task << 1);
				
				vars[vi] = true;
				branch(vi+1, vars, (task << 1) + 1);
			}else{
				vars[vi] = true;
				branch(vi+1, vars, (task << 1) + 1);
				
				vars[vi] = false;
				branch(vi+1, vars, task << 1);
			}
		}
	}
}

int main(){
	int start = omp_get_wtime();

	cin >> N >> C;
	for(int i = 0; i < C; i++){
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
	
	D_TASKS = log2(omp_get_max_threads()) + 4; // NOT WORKING FIX THIS!
	printf("%d\n", D_TASKS);
	//D_TASKS = 1 + 4;
	tasks = (int*) malloc(sizeof(int) * (1 << D_TASKS) );
	memset(tasks, 0, sizeof(int) * (1 << D_TASKS));

	bool v[MAX_VARS+1];
	#pragma omp parallel for
	for(int th=0; th < omp_get_num_threads(); th++){
		bool v2[MAX_VARS+1];
		memcpy(v2, v, sizeof(v));
		branch(1, v2, 0);
		cout << "Thread " << omp_get_thread_num() << ": " << omp_get_wtime() - start << endl;
		printf("%d\n", omp_get_num_threads());
	}
	cout << best << " " << nbest << endl;
	for(int i=1; i <= N; i++){
		cout << (bestAssignment[i] ? i: -i);
		if(i < N) cout << " ";
	}
	cout << endl;
}
