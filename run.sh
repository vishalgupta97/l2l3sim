trace_dir=../traces
traces=(bzip2.log_l1misstrace gcc.log_l1misstrace gromacs.log_l1misstrace h264ref.log_l1misstrace hmmer.log_l1misstrace sphinx3.log_l1misstrace)
num_traces=(2 2 1 1 1 2)

if [ ${1} = 6 ]; then
	for i in `seq 0 5`
	do
		echo ${traces[${i}]}
		./bin/l2l3sim ${trace_dir}/${traces[${i}]} ${num_traces[${i}]}
	done
else
	echo ${traces[${i}]}
	./bin/l2l3sim ${trace_dir}/${traces[${1}]} ${num_traces[${1}]}
fi
