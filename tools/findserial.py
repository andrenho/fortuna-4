#!/usr/bin/env python3

# This script will attempt to find either the MEGA2560 debugger or the Pico I/O board 
# and return the serial port.

import re
import sys
import platform
import subprocess

DBG_PRODUCT_ID="7523"
DBG_VENDOR_ID="1a86"
IO_PRODUCT_ID="000a"
IO_VENDOR_ID="2e8a"

if len(sys.argv) != 2 or (sys.argv[1] != "dbg" and sys.argv[1] != "io"):
    print("Usage: " + sys.argv[0] + " [dbg|io]")
    sys.exit(1)

if platform.system() == "Darwin":
    result = subprocess.run("""
       ioreg -r -c IOUSBHostDevice -x -l | perl -ne 'BEGIN {} /"USB Serial Number" = "(.+)"/ && ($sn=$1); /"idProduct" = (.+)/ && ($ip=$1); /"idVendor" = (.+)/ && ($iv=$1); /"IOCalloutDevice" = "(.+)"/ && print "$sn,$ip,$iv,$1\n"'
    """, stdout=subprocess.PIPE, shell=True)
    for line in result.stdout.decode("utf-8").splitlines():
        usb_id, product_id, vendor_id, serial_port = line.split(",")
        found = False
        if (sys.argv[1] == "io" and product_id == "0x"+IO_PRODUCT_ID and vendor_id == "0x"+IO_VENDOR_ID) \
                or (sys.argv[1] == "dbg" and product_id == "0x"+DBG_PRODUCT_ID and vendor_id == "0x"+DBG_VENDOR_ID):
            print(serial_port)
            sys.exit(0)
    print("Device not found")
    sys.exit(1)

else:
    result = subprocess.run("find /sys/bus/usb/devices/usb*/ -name dev", stdout=subprocess.PIPE, shell=True)
    for sysdevpath in result.stdout.decode("utf-8").rstrip().split("\n"):
        result2 = subprocess.run("udevadm info -q name -p " + sysdevpath.removesuffix("/dev"), stdout=subprocess.PIPE, shell=True)
        devname = result2.stdout.decode("utf-8").rstrip()
        if not devname.startswith("bus/"):
            result3 = subprocess.run("udevadm info -q property --export -p " + sysdevpath.removesuffix("/dev"), stdout=subprocess.PIPE, shell=True)
            vrs = result3.stdout.decode("utf-8").rstrip()
            if "ID_VENDOR_ID" in vrs and "ID_MODEL_ID" in vrs:
                vendor_id = re.search("""ID_VENDOR_ID='(....)""", vrs).group(1)
                product_id = re.search("""ID_MODEL_ID='(....)""", vrs).group(1)
                if (sys.argv[1] == "io" and product_id == IO_PRODUCT_ID and vendor_id == IO_VENDOR_ID) \
                        or (sys.argv[1] == "dbg" and product_id == DBG_PRODUCT_ID and vendor_id == DBG_VENDOR_ID):
                    print("/dev/" + devname)
                    sys.exit(0)

    print("Device not found")
    sys.exit(1)
