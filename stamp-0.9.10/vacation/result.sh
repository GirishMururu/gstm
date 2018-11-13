MAX_LOOP=$3
size=$5
if [ $5 == "default" ]; then
    if [ $4 == "mcmc_data" ]; then
        size="medium"
    else
        size="small"
    fi
fi

for (( i = 0; i < $MAX_LOOP; i++))
do
   
    if [ $size == "large" ]; then
	./$1 -n4 -q60 -u90 -r1048576 -t4194304 -c$2 #large
    elif [ $size == "medium" ]; then
	    ./$1 -n4 -q60 -u90 -r524288 -t1048576 -c$2 #medium
    else
	./$1 -n4 -q60 -u90 -r16384 -t4096 -c$2  #small
    fi
    
done
