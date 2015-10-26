#!/bin/sh


REQUEST_METHOD=GET
SCRIPT_NAME="/en/conf/wless_main.htm"
HTTP_USER_AGENT="dummy-browser"
HTTP_ACCEPT_LANGUAGE="en"

export REQUEST_METHOD
export SCRIPT_NAME
export HTTP_USER_AGENT
export HTTP_ACCEPT_LANGUAGE

/usr/sbin/sxcgi
