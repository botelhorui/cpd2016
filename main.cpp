#include <iostream>
#include <algorithm>
#include <string.h>
#include <stdio.h>

using namespace std;

#define MAX_CLAUSES (2 << 16) + 1
#define MAX_VARS 128

int N, C, best = 0, nbest = 0;
bool bestAssignment[MAX_VARS];
int* clauses[MAX_CLAUSES];

int clausePointers[MAX_VARS+1];

bool comp(int* a, int* b) {
	// a[0] returns number of clauses in clause
	return abs(a[a[0]]) < abs(b[b[0]]); 
}


bool intComp(int a, int b) {
	return abs(a) < abs(b); 
}

int calcClauses(bool* vars){
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
int calcClosedClauses(bool* vars, int st_var){
	int sum = 0;
	for(int i=clausePointers[st_var]; i < C; i++){
		int v;
		for(v=1; v <= clauses[i][0]; v++){
			int c = clauses[i][v];
			int a = abs(c);
			if(a >= st_var)
				break;
			else if(c < 0)
				if(!vars[a])
					break;
			else
				if(vars[a])
					break;
		}
		if(v > clauses[i][0]) sum++;
	}
	return sum;
}

int calcInterval(bool* vars, int st_var, int end_var){
	int sum = 0;
	for(int i=clausePointers[st_var]; i < clausePointers[end_var]; i++){
		int v;
		for(v=1; v <= clauses[i][0]; v++){
			int c = clauses[i][v];
			int a = abs(c);
			if(c < 0)
				if(!vars[a])
					break;
			else
				if(vars[a])
					break;
		}
		if(v > clauses[i][0]) sum++;
	}
	return sum; 
}

// vars saves variables assignments
void branch(bool* vars, int vi, int closed_vi){
	
	if(vi == N+1){
		int sum = calcClauses(vars);
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
	/*if(C - calcClosedClauses(vars, vi, clausePointers[vi]) < best){
 		return;
	}*/
 	if(C - closed_vi - calcClosedClauses(vars, vi) < best){
 		return;
	}

	int c = calcInterval(vars, vi, vi+1);
	//printf("vi: %d, %d %d\n", vi, c, c + closed_vi);
	vars[vi] = true;
	branch(vars, vi+1, closed_vi+c);
	vars[vi] = false;
	branch(vars, vi+1, closed_vi+c);
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
	memset(clausePointers, -1, sizeof(clausePointers));
	for(int i = 0; i < C; i++){
		int s = clauses[i][0];
		int v = abs(clauses[i][s]);
		if(clausePointers[v] == -1)
			clausePointers[v] = i;
	}

	for(int i = 0; i < C; i++){
		for(int v=1; v<=clauses[i][0]; v++){
			printf("%d ", clauses[i][v]); fflush(stdout);
		}
		printf("\n");
	}

	clausePointers[N+1] = C;
	for(int i=N; i > 0; i--){
		if(clausePointers[i] == -1)
			clausePointers[i] = clausePointers[i+1];
		printf("%d %d\n", i, clausePointers[i]);
	}


	bool v[MAX_VARS+1];
	branch(v, 1, 0);
	cout << best << " " << nbest << endl;
	for(int i=1; i <= N; i++){
		cout << (bestAssignment[i] ? i: -i);
		if(i < N) cout << " ";
	}
	cout << endl;
}