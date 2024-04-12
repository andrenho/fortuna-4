import os
import platform
import random
import serial
import subprocess
import time

class Fortuna:

    def __init__(self, project_root="../.."):
        self.project_root = project_root
        result = subprocess.run(project_root + "/tools/findserial.py dbg", stdout=subprocess.PIPE, shell=True)
        if result.returncode != 0:
            raise Exception("Failed to find a serial port: " + result.stdout.decode("utf-8"))
        serial_port = result.stdout.decode("utf-8").rstrip()
        print("Connecting to " + serial_port)
        self.ser = serial.Serial(serial_port, 115200)
        time.sleep(1)
        random.seed()

    def get_response(self, convert_to_int=True):
        self.ser.readline()
        r = self.ser.readline().decode('utf-8').replace('\r', '').replace('\n', '')
        print("<- " + r)
        s = r.split()
        if convert_to_int:
            return s[0] == '+', list(map(lambda h: int(h, 16), s[1:]))
        else:
            return s[0] == '+', s[1:]

    def send(self, cmd, pars=[]):
        req = cmd + ' ' + ' '.join(map(lambda v: '%x' % v, pars))
        print("-> " + req)
        self.ser.write(bytes(req + '\n', 'utf-8'))

    def ack(self):
        self.send('A')
        return self.get_response()[0]

    def compile(self, source):
        exe = self.project_root + '/tools/compiler/vasmz80_oldstyle'
        if platform.system() == 'Windows':
            exe += '.exe'
        if platform.system() == 'Darwin':
            exe += '_macos'
        with open('src.z80', 'w') as f:
            f.write(source)
        cp = subprocess.run(
            [exe, '-chklabels', '-Llo', '-ignore-mult-inc', '-nosym', '-x', '-Fbin', '-o', 'rom.bin', 'src.z80'],
            capture_output=True, text=True)
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
        self.send('S')
        assert self.get_response()[0]
        for _ in range(steps):
            self.send('S')
            assert self.get_response()[0]

    def read_ram(self, addr):
        self.send('R', [addr, 1])
        r, v = self.get_response()
        assert r
        return v[1]
