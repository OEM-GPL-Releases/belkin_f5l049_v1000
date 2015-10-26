#!/bin/sh

CNF=/etc/vsftpd.conf
CNTDATA=$1

if [ "${CNTDATA}" = "" ]; then
	echo "Usage: vsftpd.sh Control_DATA"
	exit 1
fi

# Truncate old config data.
echo -n "" > ${CNF}

TMP=`sxsysconf -f ${CNTDATA} NAS_ACTRL`
if [ $? = 0 ]; then
	if [ "${TMP}" = "ENABLE" ]; then
		echo "anonymous_enable=NO" >> ${CNF}
		echo "local_enable=YES" >> ${CNF}
	elif [ "${TMP}" = "DISABLE" ]; then
		echo "anonymous_enable=YES" >> ${CNF}
		echo "local_enable=NO" >> ${CNF}
	else
		echo "Illegal setting NAS_ACTRL=${CNF}"
		exit 1
	fi
fi

echo "anon_mkdir_write_enable=YES" >> ${CNF}
echo "anon_other_write_enable=YES" >> ${CNF}
echo "anon_upload_enable=YES" >> ${CNF}
echo "chroot_local_user=YES" >> ${CNF}
echo "chroot_list_enable=YES" >> ${CNF}
echo "chroot_list_file=/etc/vsftpd.chroot_list" >> ${CNF}
echo "write_enable=YES" >> ${CNF}
echo "local_umask=022" >> ${CNF}
echo "dirmessage_enable=YES" >> ${CNF}
echo "connect_from_port_20=YES" >> ${CNF}
echo "ls_recurse_enable=YES" >> ${CNF}
echo "syslog_enable=YES" >> ${CNF}
echo "userlist_enable=YES" >> ${CNF}
echo "listen=YES" >> ${CNF}

exit 0

