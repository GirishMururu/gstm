C=`pwd`
exec_dir=`pwd`
for dir in */;do
	cd $dir
	app="${dir::-1}"
	echo $app
	#cd $5
	#echo $app >>../../AvgSummary_"$5"
	#"$exec_dir"/process_result.py $1 $2 $3 $4 >>../../AvgSummary_"$5"
	#"$exec_dir"/guidanceMetric.py state_data $1
	wc -l state_data
	cd $C
done
