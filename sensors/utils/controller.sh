#!/bin/bash

# Set variables
# Run as controller.sh <mode> <touchscreen>
mode=$1
# touchscreen="ELAN Touchscreen"
touchscreen=$2

case $mode in
    "laptop" )
		# touchpad on, keyboard on, orientation normal, onboard off
        synclient TouchpadOff=0
        xrandr -o normal
		xinput set-prop --type=int --format=8 $touchscreen "Evdev Axes Swap" "0"
        xinput set-prop $touchscreen "Evdev Axis Inversion" 0 0
        killall onboard
        ;;
	"stand" )
		# touchpad off, keyboard off, orientation normal, onboard on
		synclient TouchpadOff=1
        xrandr -o normal
		xinput set-prop --type=int --format=8 $touchscreen "Evdev Axes Swap" "0"
        xinput set-prop $touchscreen "Evdev Axis Inversion" 0 0
        onboard &
		;;
	"tent" )
	"tablet-inverted" )
		# touchpad off?, keyboard off?, orientation inverted, onboard on
	    xrandr -o inverted
        xinput set-prop --type=int --format=8 $touchscreen "Evdev Axes Swap" "0"
        xinput set-prop $touchscreen "Evdev Axis Inversion" 1 1
		onboard &
        ;;
	"tablet-normal" )
		xrandr -o normal
        synclient TouchpadOff=1
        xinput set-prop --type=int --format=8 $touchscreen "Evdev Axes Swap" "0"
        xinput set-prop $touchscreen "Evdev Axis Inversion" 0 0
		onboard &
        ;;
	"tablet-right" )
		xrandr -o right
		synclient TouchpadOff=1
        xinput set-prop --type=int --format=8 $touchscreen "Evdev Axes Swap" "1"
        xinput set-prop $touchscreen "Evdev Axis Inversion" 0 1
		onboard &
        ;;
	"tablet-left" )
		xrandr -o left
		synclient TouchpadOff=1
		xinput set-prop --type=int --format=8 $touchscreen "Evdev Axes Swap" "1"
        xinput set-prop $touchscreen "Evdev Axis Inversion" 1 0
		onboard &
		;;
    * )
        echo $orientation
        ;;
esac
