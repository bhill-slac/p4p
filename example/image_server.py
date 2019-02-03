#!/usr/bin/env python
"""Serve up an image

$ python image_server.py pv:face

Then later run in eg. ipython -pylab

  from p4p.client.thread import Context
  ctxt=Context('pva')
  imshow(ctxt.get('pv:face'))
"""

import logging
import sys

from scipy.misc import face

from p4p.nt import NTNDArray
from p4p.server import Server, StaticProvider
from p4p.server.thread import SharedPV

def getargs():
    from argparse import ArgumentParser
    P = ArgumentParser()
    P.add_argument('pvname')
    P.add_argument('-g', '--gray', action='store_const', const=True, default=True)
    P.add_argument('-C', '--color', action='store_const', const=False, dest='gray')
    P.add_argument('-d', '--debug', action='store_const', const=logging.DEBUG, default=logging.INFO)
    return P.parse_args()

args = getargs()

logging.basicConfig(level=args.debug)

pv = SharedPV(nt=NTNDArray(),
              initial=face(gray=args.gray))

provider = StaticProvider('face')
provider.add(args.pvname, pv)
print('serving pv:', args.pvname)

Server.forever(providers=[provider])
