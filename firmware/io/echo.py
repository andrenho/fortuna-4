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
start:
    in  a, (0)      ; last character pressed
    or  a           ; if no character (A == 0)
    jr  z, start    ;   return to start
    out (0), a      ; otherwise print character
    jr  start
''')
