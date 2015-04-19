#!/bin/sh
TEMPDIR=tmp
EXTFILEDIR=tools
mkdir $TEMPDIR
mkdir $TEMPDIR/etc
mkdir $TEMPDIR/etc/config
mkdir $TEMPDIR/etc/init.d
mkdir $TEMPDIR/etc/rc.d
cp $EXTFILEDIR/smart_home $TEMPDIR/etc/config/
cp $EXTFILEDIR/weather_home.sh $TEMPDIR/etc/init.d/
chmod 0755 $TEMPDIR/etc/init.d/weather_home.sh
ln -s /etc/init.d/weather_home.sh $TEMPDIR/etc/rc.d/S99WeatherHome 

mkdir $TEMPDIR/usr
mkdir $TEMPDIR/usr/bin

mkdir $TEMPDIR/usr/lib
mkdir $TEMPDIR/usr/share
mkdir $TEMPDIR/usr/share/weather_home
mkdir $TEMPDIR/usr/lib/lua
mkdir $TEMPDIR/usr/lib/lua/luci
mkdir $TEMPDIR/usr/lib/lua/luci/controller
mkdir $TEMPDIR/usr/lib/lua/luci/model
mkdir $TEMPDIR/usr/lib/lua/luci/controller/admin
mkdir $TEMPDIR/usr/lib/lua/luci/model/cbi
mkdir $TEMPDIR/usr/lib/lua/luci/model/cbi/smart_home


cp bin/weather_home $TEMPDIR/usr/bin/
chmod 0755 $TEMPDIR/usr/bin/weather_home
cp $EXTFILEDIR/update.lua $TEMPDIR/usr/share/weather_home/
cp $EXTFILEDIR/smart_home.lua $TEMPDIR/usr/lib/lua/luci/controller/admin/
cp $EXTFILEDIR/weather.lua $TEMPDIR/usr/lib/lua/luci/model/cbi/smart_home/

cd $TEMPDIR
tar cvzf ../pack.tar.gz *
cd ..
rm -rf $TEMPDIR


