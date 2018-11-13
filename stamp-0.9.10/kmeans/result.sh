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
	    ./$1 -m40 -n40 -p$2 -t0.00001 -i inputs/random-n131072-d64-c32.txt #large
    elif [ $size == "medium" ]; then
	    ./$1 -m15 -n15 -t0.005 -p$2 -i inputs/random-n16384-d24-c16.txt #medium
    else
	    ./$1 -m15 -n15 -t0.05 -p$2 -i inputs/random-n2048-d16-c16.txt #small
     fi
done
