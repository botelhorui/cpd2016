echo "Ex3 Serial"
time ./serial < input/ex3.in
echo "Ex3 Parallel 2"
time OMP_NUM_THREADS=2 ./parallel < input/ex3.in
echo "Ex3 Parallel 3"
time OMP_NUM_THREADS=3 ./parallel < input/ex3.in
echo "Ex3 Parallel 4"
time OMP_NUM_THREADS=4 ./parallel < input/ex3.in
echo "Ex3 Parallel 5"
time OMP_NUM_THREADS=6 ./parallel < input/ex3.in
echo "Ex3 Parallel 6"
time OMP_NUM_THREADS=8 ./parallel < input/ex3.in

echo "Ex4 Serial"
time ./serial < input/ex4.in
echo "Ex4 Parallel 2"
time OMP_NUM_THREADS=2 ./parallel < input/ex4.in
echo "Ex4 Parallel 3"
time OMP_NUM_THREADS=3 ./parallel < input/ex4.in
echo "Ex4 Parallel 4"
time OMP_NUM_THREADS=4 ./parallel < input/ex4.in
echo "Ex4 Parallel 6"
time OMP_NUM_THREADS=6 ./parallel < input/ex4.in
echo "Ex4 Parallel 8"
time OMP_NUM_THREADS=8 ./parallel < input/ex4.in

echo "Ex5 Serial"
time ./serial < input/ex5.in
echo "Ex5 Parallel 2"
time OMP_NUM_THREADS=2 ./parallel < input/ex5.in
echo "Ex5 Parallel 3"
time OMP_NUM_THREADS=3 ./parallel < input/ex5.in
echo "Ex5 Parallel 4"
time OMP_NUM_THREADS=4 ./parallel < input/ex5.in
echo "Ex5 Parallel 6"
time OMP_NUM_THREADS=6 ./parallel < input/ex5.in
echo "Ex5 Parallel 8"
time OMP_NUM_THREADS=8 ./parallel < input/ex5.in
