#!/usr/bin/env python3

import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
import libfortuna4

start = 0
sz = 1 # 64 * 1024

if len(sys.argv) > 1:
    start = int(sys.argv[1], 16)
if len(sys.argv) > 2:
    sz = int(sys.argv[2], 16)

fortuna = libfortuna4.Fortuna(project_root="../../..")
fortuna.ack()

if sz == 1:
    print(hex(fortuna.read_ram(start, 1)[0]))