#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
rm ${DIR}/*.impact_of_associativity.log
cnt=0
k=0

for f in ${DIR}/../ext_traces/*.trace
	do
    (( cnt++ ))
	for ((i=1; i <= 64 ; i=2*i));
		do
		echo -e 'Executing ' $f 'with associativity ' $i
		${DIR}/../code/sim -is 8192 -ds 8192 -bs 128 -a $i -wb -wa $f >> "${DIR}/$(basename $f).impact_of_associativity.log"
	done
    grep -E "miss rate|cache size" "${DIR}/$(basename $f).impact_of_associativity.log" > "${DIR}/grep.$(basename $f).impact_of_associativity.log"
done