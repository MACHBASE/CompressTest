echo "===================================================================="
echo "stop bincompress2 "
echo "===================================================================="
append_pid_name=`ps -ef | grep -v grep|grep "bincompress2 "`
echo $append_pid_name

append_pids=(`ps -ef | grep -v grep|grep "bincompress2 "| awk '{print $2}'`)

for pid in ${append_pids[@]}
do
    echo "pid=$pid"
    kill -9 $pid
done

