#!/usr/bin/env python3

import platform
import random
import subprocess
import unittest
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "..", ".."))
import libfortuna4

fortuna = libfortuna4.Fortuna()
fortuna.ack()

fortuna.reset()

fortuna.upload_and_run('''
    ld  a, 0x38
    out (1), a
    ld  a, 0xff
    in  a, (1)       ; DB 00
    ld  (0x60), a    ; 32 60 00
''')

print(hex(fortuna.read_ram(0x60, 1)[0]))
# self.assertEqual(self.fortuna.read_ram(addr, 1)[0], byte)
