import platform
import random
import serial
import subprocess
import time
import unittest
import os

class Fortuna4Tests(unittest.TestCase):

    ser = None

    @classmethod
    def setUpClass(cls):
        result = subprocess.run("./finddbg.py", stdout=subprocess.PIPE, shell=True)
        if result.returncode != 0:
            raise Exception("Failed to find a serial port: " + result.stdout.decode("utf-8"))
        serial_port = result.stdout.decode("utf-8").rstrip()
        print("Connecting to " + serial_port)
        cls.ser = serial.Serial(serial_port, 115200)
        time.sleep(1)
        random.seed()

    def get_response(self, convert_to_int=True):
        self.ser.readline()
        r = self.ser.readline().decode('utf-8').replace('\r', '').replace('\n', '')
        print("<- " + r)
        s = r.split()
        if convert_to_int:
            return (s[0] == '+', list(map(lambda h: int(h, 16), s[1:])))
        else:
            return (s[0] == '+', s[1:])

    def send(self, cmd, pars=[]):
        req = cmd + ' ' + ' '.join(map(lambda v: '%x' % v, pars))
        print("-> " + req)
        self.ser.write(bytes(req + '\n', 'utf-8'))

    def ack(self):
        self.send('A')
        return self.get_response()[0]

    def test_ack(self):
        self.assertTrue(self.ack())

    def compile(self, source):
        exe = '../../debugger-web/vasmz80_oldstyle'
        if platform.system() == 'Windows':
            exe += '.exe'
        if platform.system() == 'Darwin':
            exe += '_macos'
        with open('src.z80', 'w') as f:
            f.write(source)
        cp = subprocess.run([exe, '-chklabels', '-Llo', '-ignore-mult-inc', '-nosym', '-x', '-Fbin', '-o', 'rom.bin', 'src.z80'], capture_output=True, text=True)
        os.remove('src.z80')

        if cp.returncode != 0:
            raise Exception(cp.stderr)

        rom = None
        if os.path.exists('rom.bin'):
            with open('rom.bin', 'rb') as f:
                rom = [x for x in bytearray(f.read())]
            os.remove('rom.bin')
        return rom

    def upload(self, source):
        rom = self.compile(source)
        args = [0, len(rom)]
        args.extend(rom)
        self.send('W', args)
        assert self.get_response()[0]

    def steps(self, steps):
        self.send('X')
        assert self.get_response()[0]
        for _ in range(steps):
            self.send('s')
            assert self.get_response()[0]

    def read_ram(self, addr):
        self.send('R', [addr, 1])
        r, v = self.get_response()
        assert r
        return v[1]

    def test_ram_one_byte(self):
        self.ack()
        addr = random.randrange(64 * 1024 - 1)
        data = random.randrange(255)
        self.send('W', [addr, 1, data])
        self.assertTrue(self.get_response()[0])
        self.send('R', [addr, 1])
        r, v = self.get_response()
        self.assertTrue(r)
        self.assertEqual(v[1], data)

    def test_whole_ram(self):
        self.ack()
        for i in range(0, 256 * 4):
            if random.randrange(20) == 0:  # one chance in 20
                address = i * 64
                data = [random.randrange(255) for _ in range(64)]
                args = [address, len(data)]
                args.extend(data)
                print(args)
                self.send('W', args)
                self.assertTrue(self.get_response()[0])
                self.send('R', [address, len(data)])
                r, v = self.get_response()
                self.assertTrue(r)
                self.assertEqual(v[1:], data)

    def test_z80_write_to_mem(self):
        self.ack()
        self.upload('''
            ld a, 0x38
            ld (0x8), a
        ''')
        self.steps(30)
        self.assertEqual(self.read_ram(0x8), 0x38)


if __name__ == '__main__':
    unittest.main()
