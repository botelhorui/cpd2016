#include <iostream>
#include <algorithm>
#include <string.h>
#include <omp.h>
#include <fstream>

using namespace std;

#define MAX_CLAUSES (2 << 16) + 1
#define MAX_VARS 128
#define MAX_VARS_PER_CLAUSE 21

int NUM_VARS, NUM_CLAUSES, best = 0, nbest = 0;
bool bestAssignment[MAX_VARS];
int* clauses[MAX_CLAUSES];

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


// vars saves variables assignments
void branch(int vi, bool* vars){
	
	if(vi == NUM_VARS+1){
		int sum = calcClauses(vi, vars);
		if(sum > best){
			best = sum;
			nbest = 1;
			memcpy(bestAssignment, vars, (NUM_VARS+1) * sizeof(bool));
			/*for(int i=1; i <= N; i++){
				bestAssignment[i] = vars[i];
			}*/
		} else if(sum == best){
			nbest++;
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
}


void print_result(){
	cout << best << " " << nbest << endl;
	for(int i=1; i <= NUM_VARS; i++){
		cout << (bestAssignment[i] ? i: -i);
		if(i < NUM_VARS) cout << " ";
	}
	cout << endl;
}