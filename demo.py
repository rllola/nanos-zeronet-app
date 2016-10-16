#!/usr/bin/env python

from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
from secp256k1 import PublicKey
from pybitcointools import pubkey_to_address
import json

print " ##### ZeroNet Nano S Ledger demo ##### "


print "Demo 1 : print pubKey and get btc address"
try:
    dongle = getDongle(True)
    publicKey = dongle.exchange(bytes("8004000000".decode('hex')))
    print "Address : " + pubkey_to_address(str(publicKey).encode('hex'))
except CommException as comm:
	if comm.sw == 0x6985:
		print "Aborted by user"
	else:
		print "Invalid status " + str(comm.sw)

print "Demo 2 : sign conten.json"

with open('content.json') as json_content:
    content = json.load(json_content)
    textToSign = "%s:%s" % (content["signs_required"], content["address"])
    try:
        offset = 0
        while offset <> len(textToSign):
            if (len(textToSign) - offset) > 255:
                chunk = textToSign[offset : offset + 255]
            else:
        		chunk = textToSign[offset:]
            if (offset + len(chunk)) == len(textToSign):
        	    p1 = 0x80
            else:
        	    p1 = 0x00
            apdu = bytes("8002".decode('hex')) + chr(p1) + chr(0x00) + chr(len(chunk)) + bytes(chunk)
            signature = dongle.exchange(apdu)
            offset += len(chunk)
        print "signature " + str(signature).encode('hex')
    except CommException as comm:
    	if comm.sw == 0x6985:
    		print "Aborted by user"
    	else:
    		print "Invalid status " + str(comm.sw)
