#!/usr/bin/env python3

# This script will attempt to find the MEGA2560 and return the serial port.

import sys
import platform
import subprocess

PRODUCT_ID="0x7523"
VENDOR_ID="0x1a86"

if platform.system() == "Darwin":
    result = subprocess.run("""
       ioreg -r -c IOUSBHostDevice -x -l | perl -ne 'BEGIN {} /"USB Serial Number" = "(.+)"/ && ($sn=$1); /"idProduct" = (.+)/ && ($ip=$1); /"idVendor" = (.+)/ && ($iv=$1); /"IOCalloutDevice" = "(.+)"/ && print "$sn,$ip,$iv,$1\n"'
    """, stdout=subprocess.PIPE, shell=True)
    for line in result.stdout.decode("utf-8").splitlines():
        usb_id, product_id, vendor_id, serial_port = line.split(",")
        if product_id == PRODUCT_ID and vendor_id == VENDOR_ID:
            print(serial_port)
            sys.exit(0)
    print("MEGA2560 not found.")
    sys.exit(1)

else:
    print("Platform not supported")
    sys.exit(1)
