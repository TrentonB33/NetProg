echo '10 seconds'
timeout -s 10 10 mpirun -np $1 ./a.out
echo '20 seconds'
timeout -s 10 20 mpirun -np $1 ./a.out
echo '40 seconds'
timeout -s 10 40 mpirun -np $1 ./a.out
echo '60 seconds'
timeout -s 10 60 mpirun -np $1 ./a.out
echo '80 seconds'
timeout -s 10 80 mpirun -np $1 ./a.out
echo '100 seconds'
timeout -s 10 100 mpirun -np $1 ./a.out