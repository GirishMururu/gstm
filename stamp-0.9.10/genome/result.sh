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
	    ./$1 -g16384 -s64 -n16777216 -t$2 #large
    elif [ $size == "medium" ]; then
	    ./$1 -g4096 -s32 -n2097152 -t$2 #medium
    else
	    ./$1 -g256 -s16 -n16384 -t$2 #small
    fi
done
