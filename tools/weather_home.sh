#!/bin/sh /etc/rc.common
START=99

PACKAGE_NAME=weather_home
PACKAGE_DESC="BLE weather home server"

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

