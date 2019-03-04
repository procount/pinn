#!/bin/sh
#PINN_INIT.SH
#Script to identify which of (hyp3, hyp4, dsi or hdmi) screens are attached.
#Then to switch config.txt to enabel the most appropriate one, rebooting if necessary

#set -x

mount /dev/mmcblk0p5 /settings

cd /tmp/mnt

pref=""
config=""
dsi=""
dpi=""
hdmi=""

#read the currently selected configuration
config=`cat config.txt | grep "^#select \(.*\)" | sed -e "s|#select \(.*\)|\1|"`
echo "config=$config"

#read the user's preference
if [ -e "screenpref" ] ; then
        #Process user preferences
        read pref <screenpref
        #echo "screenpref found"
fi
echo "read pref=$pref"

#Set pref to current config if not known
if [[ -z $pref ]]; then
	pref=$config
fi
#echo "pref=$pref"


#Discover what is fitted
#Check if we have an LCD screen attached
if ( dmesg | grep "Detected: pimhyp3" >/dev/null); then
    #echo "found hyp3"
    dpi="hyp3"
fi
if ( dmesg | grep "Detected: pimhyp4" >/dev/null ); then
    #echo "found hyp4"
    dpi="hyp4"
fi
echo "dpi=$dpi"
#Check if  DSI touchscreen is attached
if [ -e /sys/devices/platform/rpi_ft5406 ]; then
    dsi="dsi"
fi
echo "dsi=$dsi"
#Check if HDMI is attached
hdminame=$(tvservice -n 2>&1)
#remove any line feed
echo -n $hdminame>/tmp/hdmi
read hdminame</tmp/hdmi
rm /tmp/hdmi
if [[ "$hdminame" != "[E] No device present" ]]; then
    #echo "HDMI attached!"
    hdmi="hdmi"
fi
echo "hmdi=$hdmi"

default=$config #Choose this for a quicker boot if no display connected (keeps existing)
#default="hdmi" #Choose this to force HDMI if no display connected.


case $pref in
hyp3)
	echo "case hyp3"
	if [[ "$dpi" = "hyp3" ]]; then
		wanted="hyp3"
	elif [[ "$dpi"  = "hyp4" ]]; then
		wanted="hyp4"
	elif [[ "$dsi" = "dsi" ]]; then
		wanted = "dsi"
	else
		#echo "No display connected"
		wanted=$default
	fi
        ;;
hyp4)
	echo "case hyp4"
        if [[ "$dpi" = "hyp4" ]];then
                wanted="hyp4"
        elif [[ "$dpi"  = "hyp3" ]]; then
                wanted="hyp3"
        elif [[ "$dsi" = "dsi" ]]; then
                wanted = "dsi"
        else
		#echo "No display connected"
                wanted=$default
        fi
        ;;
rpf)
	echo "case rpf"
        if [[ "$dsi" = "dsi" ]]; then
                wanted = "dsi"
        elif [[ "$dpi" = "hyp3" ]];then
                wanted="hyp3"
        elif [[ "$dpi"  = "hyp4" ]]; then
                wanted="hyp4"
        else
		#echo "No display connected"
                wanted=$default
        fi
        ;;
hdmi)
	echo "case hdmi"
	if [[ "$hdmi" = "hdmi" ]];then
            wanted="hdmi"
        elif [[ "$dpi" = "hyp3" ]];then
            wanted="hyp3"
        elif [[ "$dpi"  = "hyp4" ]]; then
            wanted="hyp4"
        elif [[ "$dsi" = "dsi" ]]; then
            wanted = "dsi"
	    else
            #echo "No display connected"
            wanted=$default
        fi
        ;;
*)
	#echo "Shouldn't get here!">>$logfile
	#echo "default"
	wanted=$default
        ;;
esac

#echo "comparing $config with $wanted"

if [[ "$config" != "$wanted" ]]; then
	#echo "Switching to $wanted..."
	mount -o remount,rw /tmp/mnt
	cd /tmp/mnt
	cp config.$wanted config.txt
	sync
	rebootp
else
	#echo "Keeping original config of $config"
	#echo "=================="
    ;
fi

umount /settings

