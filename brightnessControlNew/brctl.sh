#!/bin/sh 
#the valid value's for $1 are from 10 to 100!
#acpi_call -p '\_SB.PCI0.VID.LCD0._BCM' -i $1
# This is the old way.
# sysctl hw.acpi.video.lcd0.brightness=${1}
# This is the new way...
backlight +${1}
