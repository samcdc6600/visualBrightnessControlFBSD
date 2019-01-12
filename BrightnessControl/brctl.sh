#!/bin/sh
#the valid value's for $1 are from 10 to 100!
acpi_call -p '\_SB.PCI0.VID.LCD0._BCM' -i $1
