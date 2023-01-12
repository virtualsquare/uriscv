### How to run phase 1

To compile phase 1 use 
```bash
make
```

```bash
cp kernel* ../../build
```

To compile emulator inside `build` directory use
```bash
cmake .. && sudo make && sudo make install && ./app/./emu
```

