#!/bin/bash
C=$(pwd)
exec_dir=`pwd`
cd ../tl2-x86-0.9.6
tl2_src=`pwd`
cd $C
cd ../stamp-0.9.10
stamp_src=`pwd`
cd $C

function make_tl2  {

    option=$1
    cd $tl2_src
    make clean
    echo "option set to = $option"
    if [ "$option" == "model" ]
    then
	    echo -e " Option set to modelling"
	    make NTHREADS=-DNUMTHREADS=$3 MODEL=-Dmodelling MU=-Dmodelusage FREQ=-DFREQX=$2 RANKS=-DMAX_RANKS=6
    elif [ "$option" == "ND_mcmc" ]
    then
	    echo -e " Option set to monitoring Non-determinism"
	    make NTHREADS=-DNUMTHREADS=$3 MODEL=-Dmodelling MU=-Dmodelusage FREQ=-DFREQX=$2 RANKS=-DMAX_RANKS=6 ND=-Dmonitor_ND
    elif [ "$option" == "ND_only" ]
    then
	    echo -e " Option set to monitoring Non-determinism"
	    make NTHREADS=-DNUMTHREADS=$3 MODEL=-Dmodelling ND=-Dmonitor_ND
    elif [ "$option" == "mcmc_data" ]
    then
	    echo -e " Option set to MCMC data"
	    make NTHREADS=-DNUMTHREADS=$3 MODEL=-Dmodelling MCMC=-Dmcmcdata
    else
	    echo -e " Option set to original"
	    make NTHREADS=-DNUMTHREADS=$3 
    fi

}

declare -a benches
#benches=( genome intruder kmeans labyrinth ssca2 vacation yada )
benches=( genome kmeans labyrinth ssca2 vacation yada )
#benches=( genome )
#benches=( ssca2 )
#benches=( labyrinth vacation )

function make_benches {
    cd $stamp_src
    for bench in "${benches[@]}"
    do
	cd $bench
	./rem.sh
	make -f Makefile.stm clean
	make -f Makefile.stm
	mkdir "$exec_dir"/$bench
	cp $bench "$exec_dir"/$bench
	cp -r input* "$exec_dir"/$bench
	cp result.sh "$exec_dir"/$bench
	cd $stamp_src
    done
}

function cleanAndCount {

    cur_dir=`pwd`
    mkdir "$5"_"$3""$4"_"$1"_"$2"
    mv rdmp mcmc_data timeAna* abortsThread* mcmc_monitor_ND "$5"_"$3""$4"_"$1"_"$2"
    cd "$5"_"$3""$4"_"$1"_"$2"
    echo $6 >> "$exec_dir"/AvgSummary_"$5"_"$3""$4"_"$1"_"$2"
    "$exec_dir"/process_result.py $1 $2 $3 $4 >>"$exec_dir"/AvgSummary_"$5"_"$3""$4"_"$1"_"$2"
    cp state_data ../.
    cd $cur_dir
}

function run_benches {
    cd $exec_dir
    for bench in "${benches[@]}"
    do
	cd $bench
	#sudo "$exec_dir"/CleanCache.sh
	{ time ./result.sh $bench $1 $2 $3 $6 2>&1 >/dev/null ;} >/dev/null 2>>time"$1".tout
	#{ time ./result.sh $bench $1 $2 $3 $6 2>&1 ;} 2>>time"$1".tout
	cleanAndCount $1 $2 $3 $4 $5 $bench
	cd $exec_dir
    done
}

function exec {
make_tl2 $3 $4 $1
make_benches 
run_benches $1 $2 $3 $4 $5 $6
}

#different options are:
# model  - for using the model in guided execution (gstm)
# ND_mcmc - for generating the Non-deterministic data (secondary data) for guided stm
# ND_only - for generating the non-deterministic data for default stm
# mcmc_data - for generating the state model
# default or orig - for default stm run

#different sizes-of-data are:
# large
# medium
# small

ARG6=${6:-"default"}
#./exec.sh numthreads numruns option(lookabove) freq directoryName(RunName) size-of-data(optional)
exec $1 $2 $3 $4 $5 $ARG6
