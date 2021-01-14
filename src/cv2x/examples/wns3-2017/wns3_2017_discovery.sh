#!/bin/bash
OVERWRITE=0
if [[ "$#" -gt 0 && $1 == "-f" ]];then
OVERWRITE=1
fi

if [[ ! -d "scratch" ]];then
echo "ERROR: $0 must be copied to ns-3 directory!" 
exit
fi

SCENARIO="wns3_2017_discovery"
STARTRUN=1
ENDRUN=10 #Number of runs
STARTSEED=1 #Run number to start serie
MAXSEEDS=500 #Number of trials
STEPSEED=10 #step
UE=10 #Number of UEs
stime=15 #Simulation time in s
version="wns3" #Version for logging run output
tx=100
recovery=false

container="wns3_2017_discovery_scenarios"
mkdir -p $container

main_discovery_log_file="${container}/discovery_log.txt"
echo "$version: $ENDRUN runs, $MAXSEEDS seeds - $SCENARIO, $UE UEs in simulation, time=$stime, recovery=$recovery" >> $main_discovery_log_file

./waf

for ((run=$STARTRUN; run<=$ENDRUN; run++))
do
  seed=$STARTSEED
  while [ $seed -le $MAXSEEDS ]
  do
    result=`ps aux | grep -i "wns3_2017_discovery*" | grep -i "build" | grep -v "grep" | wc -l`
    if [ $result -le $STEPSEED ]
    then
      max=`python -c "print $seed+$STEPSEED"`
      for ((i=$seed; i<$max; i++))
      do
        echo "RUN: ${version}_${run}_${i}"
        newdir="${container}/disc_${version}_${run}_${i}"
        if [[ -d $newdir && $OVERWRITE == "0" ]];then
          echo "$newdir exist! Use -f option to overwrite."
          continue
        fi
        mkdir -p $newdir
        OUTFILE="${newdir}/log_${version}_${run}_${i}.txt"
        rm -f $OUTFILE
        echo "UEs in simluation = $UE" >> $OUTFILE
        echo "Simulation time = $stime" >> $OUTFILE
        echo -e "-------------------------------\n" >> $OUTFILE
        ./waf --cwd=$newdir --run "$SCENARIO --RngRun=$run --RngSeed=$i --ue=$UE --time=$stime --recovery=$recovery" >> $OUTFILE 2>&1 &
        sleep 1 
      done
      seed=`python -c "print $seed+$STEPSEED"`
    else
      sleep 10
    fi
  done
done

result=`ps aux | grep -i "wns3_2017_discovery*"  | grep -i "build" | grep -v "grep" | wc -l`
while [ $result -ge 1 ]
do
  wait
  result=`ps aux | grep -i "wns3_2017_discovery*"  | grep -i "build" | grep -v "grep" | wc -l`
done

numues=$UE
numruns=$ENDRUN #runs
numseeds=$MAXSEEDS #trials
chosenue=1
periodlength=0.32
INFILE="discovery_out_monitoring.tr"
discoverystart=2

for j in $(seq 1 $numruns)
do
  outfile="${container}/discovery__cdf_${version}_ue${chosenue}_run${j}_${numseeds}trials.tr"
  rm -f $outfile
  
  for i in $(seq 1 $numseeds)
  do 
    #echo processing ${version}_${j}_${i} 
    dir="${container}/disc_${version}_${j}_${i}"
    cd $dir
    grep -v IMSI $INFILE | awk -v maxues=$numues '
      {
        if (! ($2, $5) in times) {
          times[$2, $5] = $1
        } 
      } 
      END{
        for (i=1; i<=maxues; i++) {
          maxtime=0
          count=0
          for (j=1; j<=maxues; j++) {
            if (i != j && (i, j) in times) {
              count++
              if (times[i, j] > maxtime) {
                maxtime = times[i, j]
              }
            }
          }
          print i, maxtime, count, count == (maxues - 1)?"OK":"ERROR"
        } 
      }' > discovery_first.txt
    cd ../..
  done
  errors=$(grep -c ERROR ${container}/disc_${version}_${j}_*/discovery_first.txt | grep -v 0$)
  if [ $? -eq 1 ]
  then
    head -qn$chosenue ${container}/disc_${version}_${j}_*/discovery_first.txt | sort -n | tail -n $numseeds | awk -v periodlength=$periodlength -v discoverystart=$discoverystart '
    {
      printf "%3.0f\n", (($2 - discoverystart)/ periodlength) + 0.5
    }' | awk -v numseeds=$numseeds '
    BEGIN{
      maxperiod=1
    }
    {
      count[$1]++; 
      if ($1 > maxperiod) {
        maxperiod = $1
      }
    }
    END{
      cdf=0
      for (i=1; i <= maxperiod; i++) {
        if (i in count) {
          #mean+=i*count[i]
          cdf+= count[i]
          print i, count[i], cdf, cdf*1.0/numseeds
        } else {
          print i, 0, cdf, cdf*1.0/numseeds
        }
      }
      #print ""
      #print "Mean=", mean*1.0/numseeds
    }'
  else 
    echo $errors
  fi > $outfile
done 

outfile="${container}/discovery__cdf_${version}_ue${chosenue}_average.tr"
cat ${container}/discovery__cdf_${version}_ue${chosenue}_run*_${numseeds}trials.tr | awk -v numseeds=$numseeds -v numruns=$numruns '
BEGIN{
  maxperiod=1
  totalsims=numseeds*numruns
}
{
  count[$1]+=$2
  if ($1 > maxperiod) {
    maxperiod = $1
  }
}
END{
  cdf=0
  for (i=1; i <= maxperiod; i++) {
    if (i in count) {
      cdf+= count[i]
      print i, count[i], cdf, cdf*1.0/totalsims
    } else {
      print i, 0, cdf, cdf*1.0/totalsims
    }
  }
}' >  $outfile

gnuplot -persist << EOF
  #set term x11 1
  set term png
  set out "wns3_2017_discovery_cdf.png"
  set style line 81 lt 0
  set style line 81 lt rgb "#808080"
  set grid back linestyle 81
  set border 3 back linestyle 80 
  set xtics nomirror
  set ytics nomirror
  unset surface
  set autoscale   
  unset log       
  unset label     
  set xtic auto   
  set ytic auto 
  set key on  
  set xlabel "Number of periods"
  set ylabel "CDF" 
  set key right bottom
  set title "CDF of UEs Discovered: 10 UEs and 50 Reources"
  plot "$outfile" using 1:4 title " ns-3 simulations" with steps
EOF

display "wns3_2017_discovery_cdf.png"
