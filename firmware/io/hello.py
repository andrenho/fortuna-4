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
    ld  a, 'H'
    out (0), a
    ld  a, 'e'
    out (0), a
    ld  a, 'l'
    out (0), a
    ld  a, 'l'
    out (0), a
    ld  a, 'o'
    out (0), a
''')
