#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
rm ${DIR}/*.memory_bandwidth.log
cnt=0
k=0
wa_nw=("-wa" "-nw")
wt_wb=("-wt" "-wb")


for f in ${DIR}/../ext_traces/*.trace
	do
	for ((cs = 8192; cs <= 16384 ; cs = 2*cs)); # cache size
		do
		for ((bs = 64; bs <= 128 ; bs = 2*bs)); # block size
			do
			for ((as = 2; as <= 4 ; as = 2*as)); # associativity
				do
				for an in "${wa_nw[@]}" # write policy when miss
					do
					for bt in "${wt_wb[@]}" # write policy when hit
						do
						echo -e 'Executing' $f 'with\n\tassociativity' $as '\n\tcache size' $cs '\n\tblock size' $bs '\n\twrite pollicies' $an $bt
						${DIR}/../code/sim -is $cs -ds $cs -bs $bs -a $as $bt $an $f >> ${DIR}/$(basename $f).memory_bandwidth.log"
					done
				done
			done
		done
	done
done