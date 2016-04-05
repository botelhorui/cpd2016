#include <iostream>
#include <algorithm>
#include <string.h>
#include <omp.h>
#include <math.h>
#include <bitset>

using namespace std;

#define MAX_CLAUSES (2 << 16) + 1
#define MAX_VARS 128
#define MAX_VARS_PER_CLAUSE 21
#define MAX_TASKS 24

int N, C, best = 0, nbest = 0;
bitset<MAX_VARS> bestAssignment;
int* clauses[MAX_CLAUSES];

int D_TASKS;
int *tasks;	

int calcClauses(int vi, bitset<MAX_VARS>& vars){
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
int calcClosedClauses(int vi, bitset<MAX_VARS>& vars){
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
void branch(int vi, std::bitset<MAX_VARS>& vars){
	
	if(vi == N+1){
		int sum = calcClauses(vi, vars);
		#pragma omp critical 
		{	
		if(sum >= best){ // only enter critical region if best must be modified			
				if(sum > best){
					best = sum;
					bestAssignment = vars;					
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

	vars[vi] = true;
	branch(vi+1, vars);
	vars[vi] = false;
	branch(vi+1, vars);	
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

	#pragma omp parallel for schedule(dynamic)
		for(int i = 0; i < 1 << D_TASKS; i++){
			// bits
			cout << i << endl;
			std::bitset<MAX_VARS> vars(i<<1);
			branch(D_TASKS+1, vars);
		}
		//cout << "Thread " << omp_get_thread_num() << ": " << omp_get_wtime() - start << endl;
		//#pragma omp single
		//printf("%d\n", omp_get_num_threads());
	//}
	cout << best << " " << nbest << endl;
	
	for(int i=1; i <= N; i++){
		cout << (bestAssignment[i] ? i: -i);
		if(i < N) cout << " ";
	}
	cout << endl;
}