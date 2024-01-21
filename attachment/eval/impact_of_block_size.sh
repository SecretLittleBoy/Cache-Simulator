#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
rm ${DIR}/*.impact_of_block_size.log

cnt=0
k=0
for f in ${DIR}/../ext_traces/*.trace
	do
    (( cnt++ ))
	for ((i=4; i <= 4096 ; i=2*i));
		do
		echo -e 'Executing ' $f 'with block size ' $i
		${DIR}/../code/sim -is 8192 -ds 8192 -bs $i -a 2 -wb -wa $f >> "${DIR}/$(basename $f).impact_of_block_size.log"
	done
    grep -E "miss rate|cache size" "${DIR}/$(basename $f).impact_of_block_size.log" > "${DIR}/grep.$(basename $f).impact_of_block_size.log"
done