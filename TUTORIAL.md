<!--
SPDX-FileCopyrightText: 2023 Gianmaria Rovelli

SPDX-License-Identifier: GPL-3.0-or-later
-->

## Progetto Sistemi Virtuali 2022/2023
### RISCV

### Compilare e installare l'emulatore
```bash 
mkdir -p build && cd build && cmake .. && make && sudo make install 
```

### Eseguire l'emulatore
```bash
uriscv-cli --config <config.json>
```

### Dependencies
#### Debian

```bash
sudo apt install git build-essential libc6 cmake libelf-dev libboost-dev libboost-program-options-dev libsigc++-2.0-dev gcc-riscv64-unknown-elf
```
