## Progetto Sistemi Virtuali 2022/2023
### RISCV

### Compilare l'emulatore
```bash mkdir -p build && cd build cmake .. && make && sudo make install 
```

### Eseguire l'emulatore
```bash
uriscv
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

#### Per toolchain
```bash
git clone https://github.com/riscv/riscv-gnu-toolchain
./configure --prefix=/opt/riscv --with-arch=rv32gc --with-abi=ilp32d
sudo make
```

Note per tesi
- fare benchmark tra utilizzo del file binario e file elf durante esecuzione
dell'emulatore

[Exceptions](https://mullerlee.cyou/2020/07/09/riscv-exception-interrupt/)
[Syscalls](https://www.cs.cornell.edu/courses/cs3410/2019sp/schedule/slides/14-ecf-pre-bw.pdf)

Conversione asm MIPS -> RISCV 
[Berkley](https://www.ocf.berkeley.edu/~qmn/linux/riscv.html)
[RISCV](https://riscv.org/wp-content/uploads/2019/12/riscv-spec-20191213.pdf) [Latest RISCV](https://github.com/riscv/riscv-isa-manual/releases/download/Priv-v1.12/riscv-privileged-20211203.pdf)
- i $ non si ci sono
- s0-s7 --> s2-s11
- rfe --> mret
- k0-k1 --> mscratch,dscratch (using t5,t6)
- jeq --> beq
- mult --> da fare
- mfc0,mfl0 --> non abilitato
- v0 -> a0

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
