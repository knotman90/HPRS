#!/bin/bash

#libraries that are necessary for execution data and query layer
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/exports/applications/apps/community/VELaSSCo/lib/thrift/lib/cpp/lib:/exports/applications/apps/community/VELaSSCo/lib/boost_1_53_0/stage/lib

#CHANGE ACCORDING TO THE HADOOP AND HBASE INSTALLATION FOLDER
HADOOP_BASE=/exports/applications/apps/community/VELaSSCo/hadoop-2.7.0
HBASE_BASE=/exports/applications/apps/community/VELaSSCo/hbase-1.1.2


VELASSCO_NAME="VELaSSCO"


start() {
	echo "STARTING $VELASSCO_NAME"
	# (start the hadoop cluster, this step can take lot of time)
	echo -e "\tStarting DFS"
	$HADOOP_BASE/sbin/start-dfs.sh
	 # (Start Yarn and MapReduce)
	echo -e "\tSTARTING YARN"
	$HADOOP_BASE/sbin/start-yarn.sh

	echo -e "\tSTARTING JOBHISTORY SERVER"
	$HADOOP_BASE/sbin/mr-jobhistory-daemon.sh start historyserver
# (start HBase)
	echo -e "\tSTARTING HBASE"
	$HBASE_BASE/bin/start-hbase.sh

	# (start HBase thrift server)
	echo -e "\tSTARTING THRIFT SERVER"	
	$HBASE_BASE/bin/hbase-daemon.sh start thrift -p 9090
	# (start Rest Server for HBase)
	echo -e "\tSTARTING REST SERVER" 
	$HBASE_BASE/bin/hbase-daemon.sh start rest -p 8880


# (start Hive)

#(start Flume)


	echo "VELaSSCO BOOTSTRAP DONE"
}



stop() {
	echo "STOPPING $VELASSCO_NAME"

        # (stop Rest Server for HBase)
        echo -e "\tSTOPPING REST SERVER" 
        $HBASE_BASE/bin/hbase-daemon.sh stop rest -p 8880      
  
	# (stop HBase thrift server)
        echo -e "\tSTOPPING THRIFT SERVER"      
         $HBASE_BASE/bin/hbase-daemon.sh stop thrift -p 9090

 	echo -e "\tSTOPPING HBASE"
        source $HBASE_BASE/bin/stop-hbase.sh
       
	echo -e "\tSTOPPING JOBHISTORY SERVER"
        $HADOOP_BASE/sbin/mr-jobhistory-daemon.sh stop historyserver

        echo -e "\tSTOPPING YARN"
        source $HADOOP_BASE/sbin/stop-yarn.sh

	echo -e "\tSTOPPING DFS"
        source $HADOOP_BASE/sbin/stop-dfs.sh
}

status() {
echo "STATUS"
	HOSTS="$(cat $HBASE_BASE/conf/regionservers)"

	for i in $HOSTS  ; do echo "Node $i:" && ssh $i 'jps'; done
}

v_status_help() {
	echo $"Usage: $0 {start|stop|restart|status}"
}



# See how we were called.
case "$1" in
  start)
	start
	;;
  stop)
	stop
	;;
  restart)
	stop
	sleep 3
	start
	;;
  status)
	status
	;;
  *)
#	echo $"Usage: $0 {start|stop|restart|status}"
	v_status_help
	exit 2
esac


