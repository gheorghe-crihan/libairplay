# libairplay

A C++ library to stream photos and videos via Airplay.

- Can be broadcasted to any Airplay-compatible device in the LAN via Bonjour
- Lightweight and readable code that allows for quick discovery of Airplay-compatible hosts,
including but not limited to Apple TV's.
- Straightforward options and actions (the [airplay-hangman](https://github.com/firebolt55439/airplay-hangman) repository uses this).

# Building

## MacOS
```sh
# Use Xcode/BSD-derived make:
make -f Makefile.osx
```

## Linux
```sh
# Needs Avahi/mDNS compatibility libraries installed. Use GNU make:
$ make -f Makefile.linux
```

# Tested
Under RPI Zero W Debian Bullseye.

# Status
This is Work-In-Progress.
Next step involves implementing the Fruit pairing protocol, similar to
https://github.com/mikebrady/shairport-sync/blob/3c8ceb7c97c8782903ec48e280023436711e0913/pair_ap/pair_fruit.c .
