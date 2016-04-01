time ./serial < input/ex3.in
time OMP_NUM_THREADS=2 ./parallel < input/ex3.in
time OMP_NUM_THREADS=3 ./parallel < input/ex3.in
time OMP_NUM_THREADS=4 ./parallel < input/ex3.in
time OMP_NUM_THREADS=6 ./parallel < input/ex3.in
time OMP_NUM_THREADS=8 ./parallel < input/ex3.in

time ./serial < input/ex4.in
time OMP_NUM_THREADS=2 ./parallel < input/ex4.in
time OMP_NUM_THREADS=3 ./parallel < input/ex4.in
time OMP_NUM_THREADS=4 ./parallel < input/ex4.in
time OMP_NUM_THREADS=6 ./parallel < input/ex4.in
time OMP_NUM_THREADS=8 ./parallel < input/ex4.in

time ./serial < input/ex5.in
time OMP_NUM_THREADS=2 ./parallel < input/ex5.in
time OMP_NUM_THREADS=3 ./parallel < input/ex5.in
time OMP_NUM_THREADS=4 ./parallel < input/ex5.in
time OMP_NUM_THREADS=6 ./parallel < input/ex5.in
time OMP_NUM_THREADS=8 ./parallel < input/ex5.in
