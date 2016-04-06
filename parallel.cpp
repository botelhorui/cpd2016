#include <iostream>
#include <algorithm>
#include <string.h>
#include <omp.h>
#include <math.h>
#include <bitset>

using namespace std;

#define MAX_CLAUSES (2 << 16) + 1
#define MAX_VARS 128 +1
#define MAX_VARS_PER_CLAUSE 21
#define MAX_TASKS 24

int N, C, best = 0, nbest = 0;
bool* bestAssignment = (bool*) malloc(MAX_VARS);
int* clauses[MAX_CLAUSES];

int D_TASKS;
int *tasks;

double syncTime = 0;

int calcClauses(register int vi, bool* vars){
	register int sum = 0;
	for(register int i=0; i < C; i++){
		for(register int v=1; v <= clauses[i][0]; v++){
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
int calcClosedClauses(register int vi, bool* vars){
	register int sum = 0;
	for(register int i=0; i < C; i++){
		register int v;
		for(v=1; v <= clauses[i][0]; v++){
			register int c = clauses[i][v];
			register int a = abs(c);
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
void branch(register int vi, bool* vars){
	
	if(vi == N+1){
		int sum = calcClauses(vi, vars);

		bool isBest = false;
		//#pragma omp critical 
		//{
		//if(sum >= best){ // only enter critical region if best must be modified			
				if(sum > best){
					isBest = true;
					best = sum;
					nbest = 1;
				} else if(sum == best){
					nbest++;
				}
			//}
		//}

		bool* newBest = (bool*) malloc(MAX_VARS);
		memcpy(newBest, vars, MAX_VARS);

		if(isBest){
			double st = omp_get_wtime();
			#pragma omp critical
			{
				if(sum == best){
					//free(bestAssignment);
					bestAssignment = newBest;
				}
			}
			syncTime += omp_get_wtime() - st;
		}
		
		return;
	}
	
	if(C - calcClosedClauses(vi,vars) < best){
 		return;
	}

	vars[vi] = true;
	branch(vi+1, vars);
	vars[vi] = false;
	branch(vi+1, vars);	
}



int main(){
	double start = omp_get_wtime();

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
	
	D_TASKS = log2(omp_get_max_threads()) + 3; // NOT WORKING FIX THIS!
	//printf("D_TASKS: %d\n", D_TASKS);


	#pragma omp parallel
	{
		#pragma omp for nowait schedule(dynamic)
		for(int i = 0; i < 1 << D_TASKS; i++){
			// bits
			int n = i;
			bool vars[MAX_VARS];
			memset(vars, 0, sizeof(vars));
			for(int j=1; n > 0; j++){
				vars[j] = (n & 1);
				n = n >> 1;
			}
			branch(D_TASKS+1, vars);
		}
		printf("Thread %d: %lf seconds\n", omp_get_thread_num(), omp_get_wtime()-start);
	}
	printf("Total time: %lf seconds\n", omp_get_wtime()-start);
	printf("Sync time: %lf seconds\n", syncTime);
	
	cout << best << " " << nbest << endl;
	for(int i=1; i <= N; i++){
		cout << (bestAssignment[i] ? i: -i);
		if(i < N) cout << " ";
	}
	cout << endl;
}
