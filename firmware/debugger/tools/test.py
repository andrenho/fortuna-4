#!/usr/bin/env python3

import random
import unittest
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
import libfortuna4


class Fortuna4Tests(unittest.TestCase):
    ser = None

    @classmethod
    def setUpClass(cls):
        cls.fortuna = libfortuna4.Fortuna(project_root="../../..")

    def test_ack(self):
        self.assertTrue(self.fortuna.ack())

    def test_ram_one_byte(self):
        self.fortuna.ack()
        addr = random.randrange(256, 64 * 1024 - 1)
        data = random.randrange(255)
        self.fortuna.write_ram(addr, [data])
        self.assertEqual(self.fortuna.read_ram(addr, 1)[0], data)

    def test_whole_ram(self):
        self.fortuna.ack()
        for i in range(1, 256 * 4):
            if random.randrange(20) == 0:  # one chance in 20
                address = i * 64
                data = [random.randrange(255) for _ in range(64)]
                self.fortuna.write_ram(address, data)
                self.assertEqual(self.fortuna.read_ram(address, len(data)), data)

    def test_z80_write_to_mem(self):
        self.fortuna.ack()
        self.fortuna.swap_breakpoint(9)
        for i in range(30):
            addr = random.randrange(100, 64 * 1024 - 300)
            self.fortuna.upload('''
                ld      hl, ''' + str(addr) + '''
                ld      b, 0xff
            loop:
                ld      (hl), b
                inc     hl
                djnz    loop
            done:
                jp      done
            ''')
            self.fortuna.reset()
            self.fortuna.debug_run()
            #self.fortuna.execute(1000)
            expected = [i for i in range(1, 256)]
            expected.reverse()
            self.assertEqual(self.fortuna.read_ram(addr, 255), expected)

if __name__ == '__main__':
    unittest.main()
