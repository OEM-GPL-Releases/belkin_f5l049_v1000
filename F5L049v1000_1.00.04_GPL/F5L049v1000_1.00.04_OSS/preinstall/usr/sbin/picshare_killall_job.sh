#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin

PICT_RUN_FILE="/var/lock/picshare/runnig"

    echo "PICT_RUN_FILE $PICT_RUN_FILE"
    if [ -f $PICT_RUN_FILE ]; then
       PICT_PID=`cat $PICT_RUN_FILE`
       echo "PICT_PID $PICT_PID"
       if [ -n "$PICT_PID" ]; then
          kill -15 $PICT_PID
       fi
    fi

return 0
