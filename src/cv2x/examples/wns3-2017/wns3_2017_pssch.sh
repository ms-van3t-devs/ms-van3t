#! /bin/bash


scenario="wns3_2017_pssch"
RB_START=2
RB_END=50
RB_STEP=2
TIME=50 #simulation length

function run_config ()
{    
    period=$1
    pscch=$2
    mcs=$3
    ktrp=$4    

    output=""

    for ((rb=$RB_START; rb<=$RB_END;rb=$((rb+RB_STEP))))
    do
	#echo "Running RB=$rb"
	./waf --run "$scenario --period=${period} --pscchLength=${pscch} --mcs=${mcs} --ktrp=${ktrp} --rbSize=${rb}" >> /dev/null 2>&1
	datarate=`cat SlRxPhyStats.txt  | awk 'BEGIN{bytes=0;start=0;stop=0}{if ($9==1 && $10==1) { if (bytes==0) start=$1; bytes+=$7; stop=$1}}END{print 8*bytes/(stop-start)}'`
	output="$output $datarate"       
	
    done
    echo $output
}

echo "Config 1"
run_config "sf40" 8 10 2
echo "Config 2"
run_config "sf80" 8 12 4
echo "Config 3"
run_config "sf320" 40 15 8
