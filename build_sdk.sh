#!/bin/bash

BASEDIR=`pwd`
LOGFILEHOST=$BASEDIR/tools.log
LOGFILE=$BASEDIR/software.log

echo "================================================================================"
echo "Building Milkymist SDK"
echo ""
echo "Log file (host):   $LOGFILEHOST"
echo "Log file (target): $LOGFILE"
echo "================================================================================"
echo ""

BASEDIR=`pwd`

echo -n "Building host utilities..."
cd $BASEDIR/tools
echo >> $LOGFILEHOST
date >> $LOGFILEHOST
make >> $LOGFILEHOST 2>&1
if [ "$?" != 0 ] ; then
        echo "FAILED"
	exit 1
else
        echo "OK"
fi

echo -n "Building base library..."
echo >> $LOGFILE
date >> $LOGFILE
cd $BASEDIR/software/libbase && make >> $LOGFILE 2>&1
if [ "$?" != 0 ] ; then
        echo "FAILED"
	exit 1
else
        echo "OK"
fi
echo -n "Building math library..."
cd $BASEDIR/software/libmath && make >> $LOGFILE 2>&1
if [ "$?" != 0 ] ; then
        echo "FAILED"
	exit 1
else
        echo "OK"
fi
echo -n "Building HAL..."
cd $BASEDIR/software/libhal && make >> $LOGFILE 2>&1
if [ "$?" != 0 ] ; then
        echo "FAILED"
	exit 1
else
        echo "OK"
fi
echo -n "Building network library..."
cd $BASEDIR/software/libnet && make >> $LOGFILE 2>&1
if [ "$?" != 0 ] ; then
        echo "FAILED"
	exit 1
else
        echo "OK"
fi
cd $BASEDIR

echo "Build complete!"
