# Remote Input Devices Sharing

[![status](https://gitlab.com/hev/hev-rinput/badges/master/pipeline.svg)](https://gitlab.com/hev/hev-rinput/commits/master)

Remote input devices sharing over network for Linux.

## How to Build

**Linux**:
```bash
git clone --recursive https://gitlab.com/hev/hev-rinput
cd hev-rinput
make
```

**Android**:
```bash
mkdir hev-rinput
cd hev-rinput
git clone --recursive https://gitlab.com/hev/hev-rinput jni
ndk-build
```

## How to Use

**Receiver**:
```bash
cat conf/main.ini
[Main]
Port=6380
Address=0.0.0.0

sudo bin/hev-rinput conf/main.ini
```

**Sender**:
```bash
cat conf/main.ini
[Main]
Port=6380
Address=192.168.1.2
; See /usr/include/linux/input-event-codes.h
SwitchKeyCode=119

sudo bin/hev-rinput conf/main.ini
```

## Authors
* **Heiher** - https://hev.cc

## License
LGPL
