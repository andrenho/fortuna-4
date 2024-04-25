import os
import platform
import random
import serial
import subprocess
import time


class Fortuna:

    def __init__(self, debug=True, project_root="../.."):
        self.debug = debug
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
        if self.debug:
            print("<- " + r)
        s = r.split()
        if convert_to_int:
            return s[0] == '+', list(map(lambda h: int(h, 16), s[1:]))
        else:
            return s[0] == '+', s[1:]

    def send(self, cmd, pars=[]):
        req = cmd + ' ' + ' '.join(map(lambda v: '%x' % v, pars))
        if self.debug:
            print("-> " + req)
        self.ser.write(bytes(req + '\n', 'utf-8'))

    def ack(self):
        self.send('A')
        return self.get_response()[0]

    def compile(self, source=None, filename=None):
        if source == None and filename == None:
            raise Exception("Use either a source or a filename")

        exe = self.project_root + '/tools/compiler/vasmz80_oldstyle'
        if platform.system() == 'Windows':
            exe += '.exe'
        if platform.system() == 'Darwin':
            exe += '_macos'
        if platform.machine() == 'armv7l':
            exe += '_rpi'

        if source != None:
            with open('src.z80', 'w') as f:
                f.write(source)

        cp = subprocess.run(
            [exe, '-chklabels', '-L', 'listing.txt', '-Llo', '-ignore-mult-inc', '-nosym', '-x', '-Fbin', '-o',
             'rom.bin',
             filename or "src.z80"], capture_output=True, text=True)

        if source != None:
            os.remove('src.z80')

        if cp.returncode != 0:
            return {'stderr': cp.stderr, 'status': cp.returncode}

        rom = None
        with open('listing.txt', 'r') as f:
            dbg_source = f.read()
        if os.path.exists('listing.txt'):
            os.remove('listing.txt')
        if os.path.exists('rom.bin'):
            with open('rom.bin', 'rb') as f:
                rom = [x for x in bytearray(f.read())]
            os.remove('rom.bin')
        return {'src': dbg_source, 'rom': rom, 'stdout': cp.stdout, 'stderr': cp.stderr, 'status': cp.returncode}

    def upload(self, source):
        rom = self.compile(source=source)['rom']
        args = [0, len(rom)]
        args.extend(rom)
        self.send('W', args)
        assert self.get_response()[0]
        return len(rom)

    def steps(self, nsteps):
        for _ in range(nsteps):
            self.send('S')
            assert self.get_response()[0]

    def execute(self, nsteps):
        self.send('X')
        assert self.get_response()[0]
        self.send('S')
        self.steps(nsteps)

    def read_ram(self, addr, n_bytes):
        self.send('R', [addr, n_bytes])
        r, v = self.get_response()
        assert r
        return v[1:]

    def write_ram(self, addr, bytes):
        args = [addr, len(bytes)]
        args.extend(bytes)
        self.send('W', args)
        assert self.get_response()[0]

    def memory_page(self, page):
        self.send('R', [page * 0x100, 256])
        ok, data = self.get_response()
        return data[1:] if ok else None

    def memory_set(self, address, data):
        self.send('W', [address, len(data)] + data)
        ok, _ = self.get_response()
        return ok

    def step_cycle(self):
        self.send('s')
        ok, r = self.get_response()
        data, addr, m1, iorq, busak, wait, int_, wr, rd, mreq = r
        return {
            'data': data,
            'addr': addr,
            'm1': m1 == 1,
            'iorq': iorq == 1,
            'busak': busak == 1,
            'wait': wait == 1,
            'int': int_ == 1,
            'wr': wr == 1,
            'rd': rd == 1,
            'mreq': mreq == 1
        }

    def step(self):
        self.send('S')
        ok, r = self.get_response()
        return r[0]

    def step_status(self, r):
        if len(r) == 1:
            return {'pc': r[0]}
        else:
            af, bc, de, hl, afx, bcx, dex, hlx, ix, iy, sp, pc, st0, st1, st2, st3, st4, st5, st6, st7, bank, ramonly = r
            return {
                'af': af, 'bc': bc, 'de': de, 'hl': hl, 'afx': afx, 'bcx': bcx, 'dex': dex, 'hlx': hlx,
                'ix': ix, 'iy': iy, 'sp': sp, 'pc': pc,
                'stack': [st0, st1, st2, st3, st4, st5, st6, st7],
                'bank': bank,
                'ramonly': ramonly,
            }

    def step_nmi(self):
        self.send('N')
        ok, r = self.get_response()
        return self.step_status(r)

    def next(self):
        self.send('n')
        ok, r = self.get_response()
        return self.step_status(r)

    def reset(self):
        self.send('X')
        ok, _ = self.get_response()
        return ok

    def swap_breakpoint(self, bkp):
        self.send('B', [bkp])
        ok, r = self.get_response()
        return r

    def debug_run(self):
        self.send('D')
        ok, r = self.get_response()
        return r[0]

    def run(self):
        self.send('r')
        self.get_response()

    def upload_and_run(self, source):
        bkp = self.upload(source)
        self.swap_breakpoint(bkp)
        self.reset()
        self.debug_run()
