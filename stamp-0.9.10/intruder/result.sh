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
	./$1 -a10 -l128 -n262144 -s1 -t$2 #large
    elif [ $size == "medium" ]; then
	./$1 -a10 -l32 -n65536 -s1 -t$2 #medium
    else
	./$1 -a10 -l4 -n2038 -s1 -t$2 #small
    fi
    
done
