trace_dir=./traces
traces=(bzip2.log_l1misstrace gcc.log_l1misstrace gromacs.log_l1misstrace h264ref.log_l1misstrace hmmer.log_l1misstrace sphinx3.log_l1misstrace)
#config=(inclusive noninclusive exclusive)
config=(inclusive)
num_traces=(2 2 1 1 1 2)

if [ ${1} = 6 ]; then
	for i in `seq 0 5`
	do
		for cf in ${config[@]}
		do
			echo ${traces[${i}]} $cf
			./bin/l2l3sim-$cf ${trace_dir}/${traces[${i}]} ${num_traces[${i}]}
		done
	done
else
	for cf in ${config[@]}
	do
		echo ${traces[${1}]} $cf
		./bin/l2l3sim-$cf ${trace_dir}/${traces[${1}]} ${num_traces[${1}]}
	done
fi
