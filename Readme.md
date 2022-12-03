## Progetto Sistemi Virtuali 2022/2023
### RISCV

### Compilare l'emulatore
```bash
mkdir -p build && cd build
cmake .. && make && ./app/./emu
```

### Compilare i test
```bash 
cd tests
make
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

Conversione asm MIPS -> RISCV 
- i $ non si ci sono
- s0-s7 --> s2-s11
- rfe --> mret
- k0-k1 --> s0-s1

Prerequisiti
- boost
- libsigc++

Accorgimenti per libsigc++ su arch
- se scaricato tramite aur la path usata e' /usr/include/sigc++-2.0,
  che rompe chiaramente tutto
  va quindi fatto un symlink a quella cartella in modo che risulti
  /usr/include/sigc++
- sigc++config.h ha lo stesso problema, al posto di trovarsi in /usr/include
  si trova in /usr/lib/sigc++-2.0/include
