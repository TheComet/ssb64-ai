# Super Smash Bros. 64 Python API

On ubuntu you will need to install the following required dependencies:
```sh
sudo apt install nasm zlib1g-dev libpng-dev libsdl2-dev libpython3-dev libfreetype6-dev binutils-dev
```

It is recommended to install the following optional dependencies as well:
```sh
sudo apt install libresample1-dev libspeexdsp-dev libboost-filesystem-dev
```

```sh
git clone git://github.com/thecomet/ssb64-ai
cd ssb64-ai/emulator
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=path/to/your/python/project/or/env ../
make -jX
make install
```

This will compile the emulator, API, package it all up with config files and install it
into a directory called ```m64pai``` which is a python module (can be imported in python).

You can test whether it works by executing the ```run_game.py``` python script. You will
need to copy Super Smash Bros. 64 (U) [!].z64 into the build directory first, though:

```sh
wget Super Smash Bros. 64 (U) [!].z64
python3 run_game.py
```

This should load up Dreamland with two Pikachus connected to P2 and P4.

Most of the documentation on how to use the API can be viewed by running:
```python
python
>>> import m64py
>>> help(m64py)
```

