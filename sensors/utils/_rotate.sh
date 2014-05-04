#!/bin/bash

# Script rotates screen 90deg on every run, and also rotates touchscreen and wacom input.
# In modes other than normal, touchpad is deactivated.

current_orientation(){
    xrandr|grep " connected" |awk '{print $4}'
}
#orientation=`current_orientation`
case $( current_orientation ) in
    "(normal" )
        xrandr -o left
        xsetwacom set "Wacom ISDv4 EC Pen stylus" rotate ccw
        xsetwacom set "Wacom ISDv4 EC Pen eraser" rotate ccw
        synclient TouchpadOff=1
        xinput set-prop --type=int --format=8 "ELAN Touchscreen" "Evdev Axes Swap" "1"
        xinput set-prop "ELAN Touchscreen" "Evdev Axis Inversion" 1 0
        onboard &
        ;;
    "inverted" )
        xrandr -o right

        xsetwacom set "Wacom ISDv4 EC Pen stylus" rotate cw
        xsetwacom set "Wacom ISDv4 EC Pen eraser" rotate cw
        xinput set-prop --type=int --format=8 "ELAN Touchscreen" "Evdev Axes Swap" "1"
        xinput set-prop "ELAN Touchscreen" "Evdev Axis Inversion" 0 1
        ;;
    "right" )
        xrandr -o normal
        xsetwacom set "Wacom ISDv4 EC Pen stylus" rotate none
        xsetwacom set "Wacom ISDv4 EC Pen eraser" rotate none
        synclient TouchpadOff=0
        xinput set-prop --type=int --format=8 "ELAN Touchscreen" "Evdev Axes Swap" "0"
        xinput set-prop "ELAN Touchscreen" "Evdev Axis Inversion" 0 0
        killall onboard
        ;;
    "left" )
        xrandr -o inverted
        xsetwacom set "Wacom ISDv4 EC Pen stylus" rotate half
        xsetwacom set "Wacom ISDv4 EC Pen eraser" rotate half
        xinput set-prop --type=int --format=8 "ELAN Touchscreen" "Evdev Axes Swap" "0"
        xinput set-prop "ELAN Touchscreen" "Evdev Axis Inversion" 1 1
        ;;
    * )
        synclient TouchpadOff=0
        echo "c est autre chose"
        current_orientation
        #echo $orientation
        ;;
esac
