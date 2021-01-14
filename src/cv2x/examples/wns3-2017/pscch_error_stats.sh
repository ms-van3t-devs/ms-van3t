#!/bin/bash

OVERWRITE=1;
if [ $# -lt "3" ];then
  echo "Missing arguments!"
  echo "Usage: $0 version StartRun EndRun"
  exit 1
fi

ver=$1
basedir=d2d
PERIOD_LENGTH=`echo $ver | sed s/.*period// | sed s/_.*//`
SIMU_TIME_SEC=`echo $ver | sed 's/.*sim_\([[:digit:]]*\)s_.*/\1/'`
SIMU_TIME=$(($SIMU_TIME_SEC*1000))
echo -e "Simulation Time: $SIMU_TIME ms, Period Length: $PERIOD_LENGTH ms"

OUTFILE_PSCCH_2="ProsePSCCH_errors_all_${basedir}_${ver}.txt"
if [[ -f $OUTFILE_PSCCH_2 && $OVERWRITE == "0" ]];then
  echo "NO OVERWRITE PERMISSION!"
  exit
fi
echo -e "Periods\tP_with_Coll\tTot_Collisions\tP_with_SO\tTot_SO\tP_with_DO\tTot_DO" > $OUTFILE_PSCCH_2 

for ((run=$2; run<=$3; run++))
do
  newdir="${basedir}_${ver}${run}"
  echo "RUN: $newdir"
  if [ ! -d $newdir ];then
    echo "$newdir does not exist!"
    continue
  fi
  
  OUTFILE_PSCCH_1="${newdir}/ProsePSCCH_errors_${ver}${run}.txt"
  
  if [[ -f $OUTFILE_PSCCH_1 && $OVERWRITE == "0" ]];then
    echo "SKIPPING RUN, NO OVERWRITE PERMISSION!"
    continue
  fi

  echo -e "Period_time\tCollisions\tSingle_Overlap\tDouble_Overlap" > $OUTFILE_PSCCH_1

  #echo "Processing Tx"
  INFILE_UeScheduleTrace="$newdir/SlUeMacStats.txt"
  
  gawk -v simu_time=$SIMU_TIME 'FNR>1{
          
            if($1 >= simu_time)
              { #Avoid counting schedulings out of simulation time because they would not be processed.
                #print
                exit;
              }
            curr_period_t = $1;
            if(FNR == 2)
              {
                prev_period_t = curr_period_t;
                p_num = 1;
              }
            
            # Collect statistics
            RI = $7; # PSCCH resource index
            RI_access[curr_period_t][RI]++;
            PSCCH_1tx = $8$9;   # Time of first PSCCH transmission
            PSCCH_2tx = $10$11; # Time of second PSCCH transmission
            PSCCH_txs[curr_period_t][PSCCH_1tx]++;
            PSCCH_txs[curr_period_t][PSCCH_2tx]++;
            PSCCH_combined_txs[curr_period_t][PSCCH_1tx "" PSCCH_2tx]++;
            
            if (prev_period_t != curr_period_t) # New period!
              {
                p_num++;
                p_collisions = 0;
                p_single_o = 0;
                p_double_o = 0;
                for(i in RI_access[prev_period_t])
                  {
                    if(RI_access[prev_period_t][i]>1)
                      {
                        p_collisions++;
                      }
                  }
                delete RI_access[prev_period_t];
                
                for(i in PSCCH_combined_txs[prev_period_t])
                  {
                    if(PSCCH_combined_txs[prev_period_t][i]>1)
                      {
                        p_double_o += PSCCH_combined_txs[prev_period_t][i] - 1;
                      }
                  }
                delete PSCCH_combined_txs[prev_period_t];
                
                for(i in PSCCH_txs[prev_period_t])
                  {
                    if(PSCCH_txs[prev_period_t][i]>1)
                      {
                        p_single_o += PSCCH_txs[prev_period_t][i] - 1;
                      }
                  }
                p_single_o -= p_double_o * 2; # Substract double overlaps counted as single overlaps.
                delete PSCCH_txs[prev_period_t];
                
                printf "%d\t%d\t%d\t%d\n", prev_period_t, p_collisions, p_single_o, p_double_o  >> "'"$OUTFILE_PSCCH_1"'";
                collisions += p_collisions;
                if(p_collisions > 0)
                  {
                    collided_periods += 1;
                  }
                single_o += p_single_o;
                if(p_single_o > 0)
                  {
                    so_periods += 1;
                  }
                double_o += p_double_o;
                if(p_double_o > 0)
                  {
                    do_periods += 1;
                  }

                prev_period_t = curr_period_t;
              }
          
          }
       END{#print "Periods: " p_num
            #Proccess last period
            p_collisions = 0;
            p_single_o = 0;
            p_double_o = 0;
            for(i in RI_access[prev_period_t])
              {
                if(RI_access[prev_period_t][i]>1)
                  {
                    p_collisions++;
                  }
              }
            delete RI_access[prev_period_t];
            
            for(i in PSCCH_combined_txs[prev_period_t])
              {
                if(PSCCH_combined_txs[prev_period_t][i]>1)
                  {
                    p_double_o += PSCCH_combined_txs[prev_period_t][i] - 1;
                  }
              }
            delete PSCCH_combined_txs[prev_period_t];
            
            for(i in PSCCH_txs[prev_period_t])
              {
                if(PSCCH_txs[prev_period_t][i]>1)
                  {
                    p_single_o += PSCCH_txs[prev_period_t][i] - 1;
                  }
              }
            p_single_o -= p_double_o * 2; # Substract double overlaps counted as single overlaps.
            delete PSCCH_txs[prev_period_t];
            
            printf "%d\t%d\t%d\t%d\n", prev_period_t, p_collisions, p_single_o, p_double_o  >> "'"$OUTFILE_PSCCH_1"'";
            collisions += p_collisions;
            if(p_collisions > 0)
              {
                collided_periods += 1;
              }
            single_o += p_single_o;
            if(p_single_o > 0)
              {
                so_periods += 1;
              }
            double_o += p_double_o;
            if(p_double_o > 0)
              {
                do_periods += 1;
              }

            # All run results
            c_ratio = collisions/p_num;
            so_ratio = single_o/p_num;
            do_ratio = double_o/p_num;
            printf "%d\t%d\t%d\t%d\t%d\t%d\t%d\n", p_num, collided_periods, collisions, so_periods, single_o, do_periods, double_o  >> "'"${OUTFILE_PSCCH_2}"'";
          }
       ' $INFILE_UeScheduleTrace 
done

OUTFILE_PSCCH_ERRORS_MEAN="ProsePSCCH_errors_mean_${basedir}_${ver}.txt"
echo -e "Tot_Runs\tMean_Collided_P_ratio\tStd_Coll_P_ratio\tMean_SO_P_ratio\tStd_SO_P_ratio\tMean_DO_P_ratio\tStd_DO_P_ratio" > "${OUTFILE_PSCCH_ERRORS_MEAN}"

awk 'FNR>1{
    coll_p_ratio[FNR-1]=$2/$1;
    sum_coll_p_ratio+=coll_p_ratio[FNR-1];
    so_p_ratio[FNR-1]=$4/$1;
    sum_so_p_ratio+=so_p_ratio[FNR-1];
    do_p_ratio[FNR-1]=$6/$1;
    sum_do_p_ratio+=do_p_ratio[FNR-1];
   }
END{
    runs=FNR-1;
    mean_coll_p_ratio=sum_coll_p_ratio/runs;
    mean_so_p_ratio=sum_so_p_ratio/runs;
    mean_do_p_ratio=sum_do_p_ratio/runs;
    for(i=1;i<FNR;i++)
      {
        sum_deviation_coll+=(coll_p_ratio[i]-mean_coll_p_ratio)^2;
        sum_deviation_so  +=(so_p_ratio[i]-mean_so_p_ratio)^2;
        sum_deviation_do  +=(do_p_ratio[i]-mean_do_p_ratio)^2;
      }
     if(runs == 1)
      {
        std_coll = 0;
        std_so   = 0;
        std_do   = 0;  
      }
     else
      {
        std_coll = sqrt(sum_deviation_coll/(runs-1));
        std_so   = sqrt(sum_deviation_so/(runs-1));
        std_do   = sqrt(sum_deviation_do/(runs-1));
      }
      printf "%d\t%.8f\t%.8f\t%.8f\t%.8f\t%.8f\t%.8f\n", runs, mean_coll_p_ratio, std_coll, mean_so_p_ratio, std_so, mean_do_p_ratio, std_do;
   }' "${OUTFILE_PSCCH_2}" >> "${OUTFILE_PSCCH_ERRORS_MEAN}"


