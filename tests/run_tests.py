import sys
import os.path as path
sys.path.append(path.realpath("."))

import unittest

if __name__ == "__main__":
    if(len(sys.argv)>1):
        for d in sys.argv[1:]:
            module = __import__(d.rstrip("/"))
            suite = unittest.TestLoader().loadTestsFromModule(module)
            unittest.TextTestRunner(verbosity = 2).run(suite)
    else:
        import lt_proc
        import lt_trim
        suite = unittest.TestLoader().loadTestsFromModule(lt_proc)
        unittest.TextTestRunner(verbosity = 2).run(suite)
        suite = unittest.TestLoader().loadTestsFromModule(lt_trim)
        unittest.TextTestRunner(verbosity = 2).run(suite)

