# ZeroNet Ledger app

POC application that allow you to create and to sign your ZeroNet website updates using the Ledger Nano S.

### Get Started

You will need a Ledger Nano S.

### Install Ledger dev env

I will explain it later...

You can also check that : http://ledger.readthedocs.io/en/latest/nanos/setup.html


### Load to Nano S

```
python -m ledgerblue.loadApp --targetId 0x31100002 --apdu --fileName ./bin/token.hex --appName ZeroNet --appFlags 0x00 --icon "0100ffffff0000000000008001e007f81f781e381c181fd81ff81bf818381c781ef81fe00780010000"
```
