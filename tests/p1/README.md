### How to run phase 1

To compile the phase 1 use 
```bash
make
```
Then copy the kernel into the build directory
```bash
cp kernel* ../../build
```

To compile emulator use
```bash
cd build
```
```bash
cmake .. && sudo make && sudo make install && ./app/./emu
```
