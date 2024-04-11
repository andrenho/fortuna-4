import random
import serial
import subprocess
import time
import unittest


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


if __name__ == '__main__':
    unittest.main()
