#!/bin/sh
set -e

PACKAGE_NAME=weather_home
PACKAGE_DESC="BLE weather home server"
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:${PATH}

start() {
    echo "Starting ${PACKAGE_DESC}: "
    /usr/bin/weather_home /dev/ttyACM0
    echo "${PACKAGE_NAME}."
}

stop() {
    echo "Stopping ${PACKAGE_DESC}: "
    kill -9 `cat /tmp/weather_home.pid` || true
    echo "${PACKAGE_NAME}."
}

restart() {
    stop
    sleep 1
    start
}

usage() {
    N=$(basename "$0")
    echo "Usage: [sudo] $N {start|stop|restart}" >&2
    exit 1
}

if [ "$(id -u)" != "0" ]; then
    echo "please use sudo to run ${PACKAGE_NAME}"
    exit 0
fi

# `readlink -f` won't work on Mac, this hack should work on all systems.
cd $(python -c "import os; print os.path.dirname(os.path.realpath('$0'))")

case "$1" in
    # If no arg is given, start the goagent.
    # If arg `start` is given, also start goagent.
    '' | start)
        start
        ;;
    stop)
        stop
        ;;
    #reload)
    restart | force-reload)
        restart
        ;;
    *)
        usage
        ;;
esac

exit 0
