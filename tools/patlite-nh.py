#!/usr/bin/env python

import socket
import binascii

TCP_IP = '192.168.1.245'
TCP_PORT = 10000
BUFFER_SIZE = 1024
HEADER = "\x58\x58\x53\x00\x00\x06"

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

class RequestBuilder:
    def __init__(self):
        self.params = {}
        self._light("red", None)
        self._light("yellow", None)
        self._light("green", None)
        self._light("blue", None)
        self._light("white", None)
        self.buzzer(None)
    
    def _light(self, light, arg):
        try:
            val = LIGHT[arg]
        except KeyError:
            val = LIGHT["NOT_CHANGE"]
        self.params[light] = val
    
    def build(self):
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

req = RequestBuilder().red("BLINK2").yellow("BLINK1").green("BLINK2").buzzer("SIREN2").build()
print binascii.hexlify(req)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print"0>> connecting"
s.connect((TCP_IP, TCP_PORT))
print"1>> sending"
s.send(req)
print"2>> receiving"
res = s.recv(BUFFER_SIZE)
s.close()
print binascii.hexlify(res)
