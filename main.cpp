#include <iostream>
#include <algorithm>
#include <string.h>

using namespace std;

#define MAX_CLAUSES (2 << 16) + 1
#define MAX_VARS 128

int N, C, best = 0, nbest = 0;
bool bestAssignment[MAX_VARS];
int* clauses[MAX_CLAUSES];

bool comp(int* a, int* b) {
	// a[0] returns number of clauses in clause
	return abs(a[a[0]]) < abs(b[b[0]]); 
}


bool intComp(int a, int b) {
	return abs(a) < abs(b); 
}

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
	bool passed = false;
	for(int i=0; i < C; i++){
		passed = false;
		for(int v=1; v <= clauses[i][0]; v++){
			int c = clauses[i][v];
			int a = abs(c);
			if(a >= vi){
				passed = true;
				break;
			}else if(c < 0){
				if(!vars[a]){					
					passed = true;
					break;
				}				
			}else{
				if(vars[a]){
					passed = true;					
					break;
				}
			} 
			
		}
		if(!passed) sum++;
	}
	return sum; 
}


// vars saves variables assignments
void branch(int vi, bool* vars){
	
	if(vi == N+1){
		int sum = calcClauses(vi, vars);
		if(sum > best){
			best = sum;
			nbest = 1;
			for(int i=1; i <= N; i++){
				bestAssignment[i] = vars[i];
			}
		} else if(sum == best){
			nbest++;			
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
	cin >> N >> C;
	int c;
	for(int i = 0; i < C; i++){
		// 1 for size, 1 for last zero
		clauses[i] = new int[MAX_CLAUSES+2];
		for(int v = 1;;v++){
			cin >> clauses[i][v];
			if(clauses[i][v] == 0){
				clauses[i][0]= v - 1;
				sort(clauses[i]+1, clauses[i]+ v, intComp);
				break;
			}

		}
	}
	sort(clauses, clauses + C, comp);
	bool v[MAX_VARS+1];
	branch(1, v);
	cout << best << " " << nbest << endl;
	for(int i=1; i <= N; i++){
		cout << (bestAssignment[i] ? i: -i);
		if(i < N) cout << " ";
	}
	cout << endl;
}