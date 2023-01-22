### Prerequisites 
```bash
git clone https://github.com/riscv/riscv-gnu-toolchain
./configure --prefix=/opt/riscv --with-arch=rv32gc --with-abi=ilp32d
sudo make
```
- boost
- libsigc++

### How to run phase 1

To compile the phase 1 use 
```bash
make
```

To compile emulator use
```bash
cd build
```
```bash
cmake .. && make && sudo make install 
```

To run emulator use inside this folder
```bash
uriscv 
```

