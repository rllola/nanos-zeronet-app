#!/usr/bin/env python

from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
from secp256k1 import PublicKey


print " ##### ZeroNet Nano S Ledger demo ##### "


print "Demo 1 : print pubKey and get btc address"
try:
    dongle = getDongle(True)
    publicKey = dongle.exchange(bytes("8004000000".decode('hex')))
    print "publicKey " + str(publicKey).encode('hex')
except CommException as comm:
	if comm.sw == 0x6985:
		print "Aborted by user"
	else:
		print "Invalid status " + str(comm.sw)
