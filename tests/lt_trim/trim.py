# -*- coding: utf-8 -*-

import unittest
import sys
import time

from subproc import Popen, PIPE
from commands import getstatusoutput

def recv(proc):
    last_read = proc.recv(1)
    sleeps = 0

    while last_read == None and sleeps < 20:
        time.sleep(0.1)
        last_read = proc.recv(1)
        sleeps += 1

    return last_read

def readUntilNull(proc):
    ret = []
    last_read = '?'
    while last_read != '\x00':
        last_read = recv(proc)
        if last_read == None:
            break;

        ret.append(last_read)

    return ''.join(ret)

class TestSimple(unittest.TestCase):
    def setUp(self):
        for cmd in ["../lttoolbox/lt-comp lr data/trim_mono.dix data/trim_mono.bin",
                    "../lttoolbox/lt-comp lr data/trim_bi.dix data/trim_bi.bin",
                    "../lttoolbox/lt-trim data/trim_mono.bin data/trim_bi.bin data/trim_trimmed.bin"]:
            exit_code, out = getstatusoutput(cmd)
            self.assertEqual(exit_code, 0,
                             msg="\n$ "+cmd+"\n"+out)

    def in_out(self, insandouts):
        proc = Popen(["../lttoolbox/lt-proc", "-e", "-z", "data/trim_trimmed.bin"],
                     stdin=PIPE,
                     stdout=PIPE,
                     stderr=PIPE)

        for i, o in insandouts:
            proc.send(i + ".[][\n]\x00")
            test_o = readUntilNull(proc)
            self.assertEqual(test_o,
                             o + "^./.<sent>$[][\n]\x00")

        proc.stdin.close()
        proc.stdout.close()

    def test_simple(self):
        self.in_out(
            [('ete', '^ete/ete<n><f><sg><ind>$'),
             ('løe', '^løe/løe<n><f><sg><ind>$'),
             ('æve', '^æve/æve<n><f><sg><ind>$'),
             ('nobidixe', '^nobidixe/*nobidixe$')]
            )

    def test_compounds_carry_over(self):
        self.in_out(
            [('rivesemje', '^rivesemje/rive<n><f><sg><ind>+semje<n><f><sg><ind>$')]
            )

    def test_compounds_trim(self):
        self.in_out(
            [('rive', '^rive/rive<n><f><sg><ind>$'),
             ('semje', '^semje/semje<n><f><sg><ind>$'),
             ('cmpnobidixe', '^cmpnobidixe/*cmpnobidixe$'),
             ('tåkecmpnobidixe', '^tåkecmpnobidixe/*tåkecmpnobidixe$')]
            )



