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

fortuna.upload('''
    in  a, (0)       ; DB 00
    ld  a, (0x60)    ; 3A 60 00
''')

"""
fortuna.upload('''
    ld  a, 'H'
    out (0), a
x:  jp x
''')
"""

fortuna.reset()
#input()
fortuna.steps(1)
#input()
fortuna.steps(1)
#input()
fortuna.steps(1)

print(hex(fortuna.read_ram(0x60, 1)[0]))
# self.assertEqual(self.fortuna.read_ram(addr, 1)[0], byte)
