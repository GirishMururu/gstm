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
	./$1 -a15 -i inputs/ttimeu1000000.2 -t$2 #large
    elif [ $size == "medium" ]; then
	./$1 -a17 -i inputs/ttimeu100000.2 -t$2 #medium
    else
	./$1 -a20 -i inputs/ladder.2 -t$2 #small
    fi
    
done
