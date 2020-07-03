set -x

DATE=`date '+%Y%m%d'`
DIR=test_$DATE
mkdir -p $DIR

#nohup  dstat -tcmndf -D vda1 -N eth0,lo -C total --disk-util -o ${DIR}/dstat_${DATE}.csv &> ${DIR}/dstat_run_${DATE}.log &
#sleep 10


#nohup ./bincompress1 -s 20200701 -a 1  -n 10 -P 9656 --disable-delay &> ${DIR}/c1  &
nohup ./bincompress1 -s 20200701 -a 1  -n 10 -P 5656 --disable-delay &> ${DIR}/c1  &
