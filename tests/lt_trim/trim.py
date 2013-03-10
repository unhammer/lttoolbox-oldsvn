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

class TestTrim(unittest.TestCase):
    def setUp(self):
        for cmd in ["../lttoolbox/lt-comp lr data/trim_mono.dix data/trim_mono.bin",
                    "../lttoolbox/lt-comp lr data/trim_bi.dix data/trim_bi.bin",
                    "../lttoolbox/lt-trim data/trim_mono.bin data/trim_bi.bin data/trim_trimmed.bin",
                    "../lttoolbox/lt-comp lr data/trim_bi_regex.dix data/trim_bi_regex.bin",
                    "../lttoolbox/lt-trim data/trim_mono.bin data/trim_bi_regex.bin data/trim_trimmed_regex.bin"]:
            exit_code, out = getstatusoutput(cmd)
            self.assertEqual(exit_code, 0,
                             msg="\n$ "+cmd+"\n"+out)

    def in_out(self, insandouts, dictionary):
        proc = Popen(["../lttoolbox/lt-proc", "-e", "-z", dictionary],
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

    def test_simple_yes(self):
        self.in_out(
            [('ete', '^ete/ete<n><f><sg><ind>$'),
             ('løe', '^løe/løe<n><f><sg><ind>$'),
             ('æve', '^æve/æve<n><f><sg><ind>$'),
             ('kråke', '^kråke/kråke<n><f><sg><ind>$')],
            "data/trim_trimmed.bin"
            )

    def test_simple_no(self):
        self.in_out(
            [('et', '^et/*et$'),
             ('kråe', '^kråe/*kråe$'),
             ('nobidixe', '^nobidixe/*nobidixe$')],
            "data/trim_trimmed.bin"
            )

    def test_regex_no(self):
        self.in_out(
            [('nobidixe', '^nobidixe/*nobidixe$'),
             ('user', '^user/*user$'),
             ('user\@foo.no', '^user/*user$\@^foo/*foo$^./.<sent>$^no/*no$')],
            "data/trim_trimmed_regex.bin"
            )

    def test_regex_yes(self):
        self.in_out(
            [('ete', '^ete/ete<n><f><sg><ind>$'),
             ('løe', '^løe/løe<n><f><sg><ind>$'),
             ('æve', '^æve/æve<n><f><sg><ind>$'),
             ('user\@foo.yes', '^user\@foo.yes/user\@foo.yes<email>$')],
            "data/trim_trimmed_regex.bin"
            )

    def test_compounds_carry_over(self):
        self.in_out(
            [('rivesemje', '^rivesemje/rive<n><f><sg><ind>+semje<n><f><sg><ind>$')],
            "data/trim_trimmed.bin"
            )

    def test_compounds_trim(self):
        self.in_out(
            [('rive', '^rive/rive<n><f><sg><ind>$'),
             ('semje', '^semje/semje<n><f><sg><ind>$'),
             ('cmpnobidixe', '^cmpnobidixe/*cmpnobidixe$'),
             ('tåkecmpnobidixe', '^tåkecmpnobidixe/*tåkecmpnobidixe$')],
            "data/trim_trimmed.bin"
            )

    def test_j_yes(self):
        self.in_out(
            [('a', '^a/a<pr>$'),
             ('el que', '^el que/el qual<rel><nn>$'),
             ('el qual', '^el qual/el qual<rel><nn>$')],
            "data/trim_trimmed.bin"
            )

    def test_j_no(self):
        self.in_out(
            [('el nobidix', '^el/*el$ ^nobidix/*nobidix$')],
            "data/trim_trimmed.bin"
            )
