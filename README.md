# Remote Input Devices Sharing

[![status](https://github.com/heiher/hev-rinput/actions/workflows/build.yaml/badge.svg?branch=master&event=push)](https://github.com/heiher/hev-rinput)

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

* **hev** - https://hev.cc

## License

LGPL
