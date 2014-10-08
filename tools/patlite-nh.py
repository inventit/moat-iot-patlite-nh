#!/usr/bin/env python

import socket
import binascii
import time

TCP_IP = '192.168.1.245'
TCP_PORT = 10000
BUFFER_SIZE = 1024
HEADER = "\x58\x58\x53\x00\x00\x06"
HEADER_OK_GOT_IT = "\x58\x58\x43\x00\x00\x00"
HEADER_TELL_ME = "\x58\x58\x47\x00\x00\x00"

LIGHT = {
    "OFF" :       "\x00",
    "ALWAYS_ON" : "\x01",
    "BLINK1" :    "\x02",
    "BLINK2" :    "\x03",
    "NOT_CHANGE": "\x09"
}

BUZZER = {
    "OFF" :       "\x00",
    "SIREN1" :    "\x01",
    "SIREN2" :    "\x02",
    "SIREN3" :    "\x03",
    "SIREN4" :    "\x04",
    "NOT_CHANGE": "\x09"
}

class NHRequestBuilder:
    def __init__(self):
        self.params = {}
        self._light("red", None)
        self._light("yellow", None)
        self._light("green", None)
        self._light("blue", None)
        self._light("white", None)
        self.buzzer(None)
        self.params["ok_got_it"] = False
    
    def _light(self, light, arg):
        try:
            val = LIGHT[arg]
        except KeyError:
            val = LIGHT["NOT_CHANGE"]
        self.params[light] = val
        self.params["ok_got_it"] = False
    
    def build(self):
        if self.params["ok_got_it"]:
            ret = HEADER_OK_GOT_IT
        else:
            ret = HEADER
            ret += self.params["red"]
            ret += self.params["yellow"]
            ret += self.params["green"]
            ret += self.params["blue"]
            ret += self.params["white"]
            ret += self.params["buzzer"]
        return ret

    def buzzer(self, arg):
        try:
            val = BUZZER[arg]
        except KeyError:
            val = BUZZER["NOT_CHANGE"]
        self.params["buzzer"] = val
        self.params["ok_got_it"] = False
        return self
        
    def red(self, arg):
        self._light("red", arg)
        return self
    def yellow(self, arg):
        self._light("yellow", arg)
        return self
    def green(self, arg):
        self._light("green", arg)
        return self
    def blue(self, arg):
        self._light("blue", arg)
        return self
    def white(self, arg):
        self._light("white", arg)
        return self

    def ok_got_it(self):
        self.params["ok_got_it"] = True
        return self

req = NHRequestBuilder().red("BLINK2").yellow("BLINK1").green("BLINK1").build()
print binascii.hexlify(req)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print"0>> connecting"
s.connect((TCP_IP, TCP_PORT))
print"1>> sending"
s.send(req)
print"2>> receiving"
res = s.recv(BUFFER_SIZE)
print binascii.hexlify(res)

time.sleep(5)

req = NHRequestBuilder().ok_got_it().build()
print"1>> sending"
s.send(req)
print"2>> receiving"
res = s.recv(BUFFER_SIZE)
print binascii.hexlify(res)

s.close()
