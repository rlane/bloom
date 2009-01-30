for i in `seq 1 20`
do
	echo $i
	gcc -O2 -DNUM_IDXS=$i test-big.c bloom.c -lssl -o bench
	time ./bench 2>/dev/null
done
