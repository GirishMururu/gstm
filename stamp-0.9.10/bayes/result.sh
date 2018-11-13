MAX_LOOP=$3
for (( i = 0; i < $MAX_LOOP; i++))
do

    ./$1 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$2 #large
 # echo"./$1 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t$2" #large
 #   ./$1 -v32 -r2048 -n5 -p30 -i2 -e4 -s1 -t$2 #medium
#    ./$1 -v32 -r1024 -n2 -p20 -s0 -i2 -e2 -t$2  #small
  # echo " ./$1 -v32 -r1024 -n2 -p20 -s0 -i2 -e2 -t$2"  #small
done
