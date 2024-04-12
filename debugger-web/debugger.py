#!/usr/bin/env python3

import argparse
import json
import http.server
import os
import platform
import serial
import socketserver
import subprocess
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
import libfortuna4

DEBUG = 0


#################
#               #
#  HTTP SERVER  #
#               #
#################

class Server(http.server.SimpleHTTPRequestHandler):

    def send_object(self, obj=None, success=True):
        if success:
            self.send_response(200, 'OK')
        else:
            self.send_response(500, 'Server error')
        self.end_headers()
        if obj is None:
            obj = {}
        self.wfile.write(bytes(json.dumps(obj), 'utf-8'))

    def parse_url(self, path):
        urlp = path.split('?')
        path = urlp[0]
        resource = path[1:].split('/')
        variables = {}
        if len(urlp) > 1:
            for v in urlp[1].split('&'):
                key, value = v.split('=')
                variables[key] = value
        return path, resource, variables

    def do_GET(self):
        path, resource, variables = self.parse_url(self.path)
        if path == '/' or path.endswith('.html') or path.endswith('.css') or path.endswith('.js'):
            super().do_GET()
        elif resource[0] == 'memory':
            page = int(resource[1])
            self.send_object(serial.memory_page(page))
        elif resource[0] == 'code':
            r = serial.compile(filename=args.source)
            self.send_object(r, r['status'] == 0)
        else:
            self.send_response(404, 'Not found')
            self.end_headers()
            self.wfile.write(b'404 - Not found.\n')

    def do_POST(self):
        path, resource, variables = self.parse_url(self.path)
        if resource[0] == 'memory':
            address = int(resource[1])
            data = json.loads(self.rfile.read(int(self.headers['Content-Length'])))['data']
            serial.memory_set(address, data)
            self.send_object()
        elif resource[0] == 'post':
            self.send_object(serial.self_test())
        elif resource[0] == 'step-cycle':
            self.send_object(serial.step_cycle())
        elif resource[0] == 'step':
            if 'nmi' in variables and variables['nmi'] == 'true':
                self.send_object(serial.step_nmi())
            else:
                self.send_object({'pc': serial.step()})
        elif resource[0] == 'next':
            self.send_object(serial.next())
        elif resource[0] == 'debug-run':
            self.send_object({'pc': serial.debug_run()})
        elif resource[0] == 'reset':
            serial.reset()
            self.send_object()
        elif resource[0] == 'breakpoint':
            bkp = int(resource[1])
            self.send_object(serial.swap_breakpoint(bkp))
        elif resource[0] == 'run':
            serial.run()
            self.send_object()
        else:
            self.send_response(404, 'Not found')
            self.end_headers()
            self.wfile.write(b'404 - Not found.\n')


parser = argparse.ArgumentParser()
parser.add_argument('source')
parser.add_argument('-l', '--log', action='store_true')
args = parser.parse_args()

if args.log:
    DEBUG = 1

serial = libfortuna4.Fortuna(debug=args.log, project_root="..")

socketserver.TCPServer.allow_reuse_address = True
print("Listening on 8000...")
httpd = socketserver.TCPServer(('127.0.0.1', 8000), Server)
httpd.allow_reuse_address = True
httpd.serve_forever()
