#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
rm ${DIR}/*.working_set_characterization.log
cnt=0
k=0
for f in ${DIR}/../ext_traces/*.trace
	do
    (( cnt++ ))
	for ((i=1; i <= 268435456 ; i=2*i));
		do
		k=$(($i*4))
		echo -e 'Executing ' $f 'with cache size ' $k
		${DIR}/../code/sim -is $k -ds $k -bs 4 -a $i -wb -wa $f >> "${DIR}/$(basename $f).working_set_characterization.log"
        echo -e 'Executing ' $f 'with cache size ' $k 'done'
	done
    grep -E "miss rate|cache size" "${DIR}/$(basename $f).working_set_characterization.log" > "${DIR}/grep.$(basename $f).working_set_characterization.log"
done