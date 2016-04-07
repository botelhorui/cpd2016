#include <iostream>
#include <algorithm>
#include <string.h>
#include <omp.h>
#include <math.h>

using namespace std;

#define GET(bs, ind) (ind < 129) ? ((bs->a >> (ind-1) ) & 1) : ((bs->b >> (ind-129)) & 1)
#define SET(bs, ind, v) (ind < 129) ? (bs->a ^= (-(v) ^ bs->a) & (1LL << (ind-1) )) : (bs->b ^= (-(v) ^ bs->b) & (1LL << (ind-129)) )

#define MAX_CLAUSES (2 << 16) + 1
#define MAX_VARS 128 +1
#define MAX_VARS_PER_CLAUSE 21
#define MAX_TASKS 24

class Bitset{
	public:
		int64_t a, b;
	Bitset(int64_t a, int64_t b){
		this->a = a;
		this->b = b;
	}
	Bitset(){
		this->a = 0;
		this->b = 0;
	}
};

int N, C, best = 0, nbest = 0;
Bitset* bestAssignment = NULL;
int clauses[MAX_CLAUSES][MAX_VARS_PER_CLAUSE];

int D_TASKS;
int *tasks;

double syncTime = 0;

Bitset* localAssignment;

int calcClauses(register int vi, Bitset* vars){
	register int sum = 0;
	for(register int i=0; i < C; i++){
		for(register int v=1; v <= clauses[i][0]; v++){
			int c = clauses[i][v];
			int a = abs(c);
			if(c < 0){
				if(!(GET(vars, a)) ){
					sum++;
					break;
				}
			}else{
				if((GET(vars, a))){
					sum++;
					break;
				}
			}
			
		}
	}
	return sum;
}

// unsatisfiable closed clauses
int calcClosedClauses(register int vi, Bitset* vars){
	register int sum = 0;
	for(register int i=0; i < C; i++){
		register int v;
		for(v=1; v <= clauses[i][0]; v++){
			register int c = clauses[i][v];
			register int a = abs(c);
			if(a >= vi){
				break;
			}else if(c < 0){
				if(!(GET(vars, a))) break;
			}else{
				if( (GET(vars, a))) break;
			}
			
		}
		if(v > clauses[i][0]) sum++;
	}
	return sum; 
}


// vars saves variables assignments
void branch(register int vi, Bitset* vars){
	
	if(vi == N+1){
		int sum = calcClauses(vi, vars);

		bool isBest = false;

		if(sum > best){
			isBest = true;
			best = sum;
			nbest = 1;
		} else if(sum == best){
			nbest++;
		}
		
		
		if(isBest){
			localAssignment[omp_get_thread_num()].a = vars->a;
			localAssignment[omp_get_thread_num()].b = vars->b;

			double st = omp_get_wtime();
			#pragma omp critical
			{
				if(sum >= best){
					bestAssignment = &localAssignment[omp_get_thread_num()];
				}
			}
			syncTime += omp_get_wtime() - st;
		}
		
		return;
	}
	
	if(C - calcClosedClauses(vi,vars) < best){
 		return;
	}

	SET(vars, vi, true);
	branch(vi+1, vars);
	SET(vars, vi, false);
	branch(vi+1, vars);
}



int main(){
	
	double start = omp_get_wtime();

	cin >> N >> C;
	for(int i = 0; i < C; i++){
		// 1 for size, 1 for last zero
		for(int v = 1;;v++){
			cin >> clauses[i][v];
			if(clauses[i][v] == 0){
				clauses[i][0]= v - 1;
				break;
			}

		}
	}
	
	D_TASKS = log2(omp_get_max_threads()) + 3; // NOT WORKING FIX THIS!
	if(D_TASKS > 31)
		D_TASKS = 31;
	//printf("D_TASKS: %d\n", D_TASKS);
	localAssignment = (Bitset *) malloc(sizeof(Bitset) * omp_get_max_threads());

	#pragma omp parallel
	{
		printf("Thread %d\n", omp_get_thread_num());
		#pragma omp for nowait schedule(guided)
		for(int i = 0; i < 1 << D_TASKS; i++){
			// bits
			int n = i;
			Bitset vars(i, 0);
			for(int j=0; n > 0; j++){
				SET((&vars), j, (n & 1));
				n = n >> 1;
			}

			branch(D_TASKS+1, &vars);
		}
		printf("Thread %d: %lf seconds\n", omp_get_thread_num(), omp_get_wtime()-start);
	}
	printf("Total time: %lf seconds\n", omp_get_wtime()-start);
	printf("Sync time: %lf seconds\n", syncTime);
	
	cout << best << " " << nbest << endl;
	cout << best << "[" << bestAssignment->a << ", " << bestAssignment->b << "]" << endl;
	for(int i=1; i <= N; i++){
		cout << ( (GET(bestAssignment, i)) == 1 ? i : -i);
		if(i < N) cout << " ";
	}
	cout << endl;
}
