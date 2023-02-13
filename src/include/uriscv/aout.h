#ifndef URISCV_AOUT_H
#define URISCV_AOUT_H

/* Number of BIOS reserved page frames */
#define N_BIOS_PAGES 1

/*
 * Core file header size in words: core file id tag (1W) + 1 full page frame
 * for BIOS exclusive use
 */
#define CORE_HDR_SIZE (N_BIOS_PAGES * 1024 + 1)

/*
 * AOUT header entries
 */
#define AOUT_HE_TAG 0
#define AOUT_HE_ENTRY 1
#define AOUT_HE_TEXT_VADDR 2
#define AOUT_HE_TEXT_MEMSZ 3
#define AOUT_HE_TEXT_OFFSET 4
#define AOUT_HE_TEXT_FILESZ 5
#define AOUT_HE_DATA_VADDR 6
#define AOUT_HE_DATA_MEMSZ 7
#define AOUT_HE_DATA_OFFSET 8
#define AOUT_HE_DATA_FILESZ 9
#define AOUT_HE_GP_VALUE 42

#define N_AOUT_HDR_ENT 43

#define AOUT_PHDR_SIZE 0xB0

#endif
