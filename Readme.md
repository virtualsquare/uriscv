## Progetto Sistemi Virtuali 2022/2023
### RISCV

### Compilare l'emulatore
```bash
mkdir -p build && cd build
cmake .. && make && ./app/./emu
```

#### Setup Clang LSP
```bash
mkdir -p build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
mv compile_commands.json ..
```

Note per tesi
- fare benchmark tra utilizzo del file binario e file elf durante esecuzione
dell'emulatore
