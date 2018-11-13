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
	./$1 -s20 -i1.0 -u1.0 -l3 -p3 -t$2 #large
    elif [ $size == "medium" ]; then
	    ./$1 -s17 -i1.0 -u1.0 -l3 -p3 -t$2 #medium
    else
	./$1 -s13 -i1.0 -u1.0 -l3 -p3 -t$2 #small
    fi
    
done
