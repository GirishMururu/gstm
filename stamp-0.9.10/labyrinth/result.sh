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
    ./$1 -i inputs/random-x512-y512-z7-n512.txt -t$2 #large
    elif [ $size == "medium" ]; then
	./$1 -i inputs/random-x128-y128-z3-n128.txt -t$2 #medium
    else
	./$1 -i inputs/random-x32-y32-z3-n96.txt -t$2 #small
    fi
    
done
