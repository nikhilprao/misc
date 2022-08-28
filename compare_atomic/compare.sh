#!/bin/bash

for ((p = 1; p <= 32; p=p*2)); do
	t1=`time ( ./threads_pc  100 $p 50 0 ) 2>&1`;
	t2=`time ( ./threads_pc  100 $p 50 1 ) 2>&1`;
	echo -n $p $t1 $t2; echo "";
done
