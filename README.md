# Super Smash Bros. 64 Deep RL Artificial Intelligence

To build the python API:

```sh
git clone git://github.com/thecomet/ssb64-ai
cd ssb64-ai/emulator
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j
```

You will need to do the following manual steps as well (also from the build directory):

```sh
# Symlink the "data" folder which contains emulator configuration files and savestates
ln -s ../m64py/data data

# The RSP plugin has to currently be renamed manually:
mv mupen64plus-rsp-*.so mupen64plus-rsp-hle.so

# The RSP plugin may be broken, in which case see if you can install mupen64plus-rsp-hle
# from your package manager (name may vary). Then (path may vary):
cp /usr/lib64/mupen64plus/mupen64plus-rsp-hle.so .

# You will also need to get Super Smash Bros. (U) [!].z64 from *somewhere*
# and place it here (in the build directory)
wget ...
```

To check if everything is working, you can try running the example script:
```sh
cp ../m64py/python/run_game.py .
python3 run_game.py
```

This should load up Dreamland with two Pikachus connected to P2 and P4.

Most of the documentation on how to use the API can be viewed by running:
```python
python
>>> import m64py
>>> help(m64py)
```
