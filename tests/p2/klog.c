/*
 * @file klog.c
 * @author Maldus512 
 * @brief Small library that implements a circular log buffer. When properly traced (with ASCII representation),
 *          `klog_buffer` displays a series of printed lines.
*/

#define KLOG_LINES     64     // Number of lines in the buffer. Adjustable, only limited by available memory
#define KLOG_LINE_SIZE 42     // Length of a single line in characters


static void next_line(void);
static void next_char(void);


unsigned int klog_line_index                         = 0;       // Index of the next line to fill
unsigned int klog_char_index                         = 0;       // Index of the current character in the line
char         klog_buffer[KLOG_LINES][KLOG_LINE_SIZE] = {0};     // Actual buffer, to be traced in uMPS3


// Print str to klog
void klog_print(char *str) {
    while (*str != '\0') {
        // If there is a newline skip to the next one
        if (*str == '\n') {
            next_line();
            str++;
        } 
        // Otherwise just fill the current one
        else {
            klog_buffer[klog_line_index][klog_char_index] = *str++;
            next_char();
        }
    }
}

/* 
*   Funzione per la stampa di numeri nei registri di memoria usati per il debugging.
*   N.B. La funzione stampa numeri in un intervallo compreso tra 0 e 99
*/
void klog_print_dec(unsigned int num) {
    const char digits[] = "0123456789";
    if(num < 10){
        do {
            klog_buffer[klog_line_index][klog_char_index] = digits[num % 10];
            num /= 10;
            next_char();
        } while (num > 0);
    }else{
        int buff = num % 10;
        num /= 10;
        klog_buffer[klog_line_index][klog_char_index] = digits[num % 10];
        next_char();
        klog_buffer[klog_line_index][klog_char_index] = digits[buff];
        next_char();
    }
}

// Princ a number in hexadecimal format (best for addresses)
void klog_print_hex(unsigned int num) {
    const char digits[] = "0123456789ABCDEF";

    do {
        klog_buffer[klog_line_index][klog_char_index] = digits[num % 16];
        num /= 16;
        next_char();
    } while (num > 0);
}


// Move onto the next character (and into the next line if the current one overflows)
static void next_char(void) {
    if (++klog_char_index >= KLOG_LINE_SIZE) {
        klog_char_index = 0;
        next_line();
    }
}


// Skip to next line
static void next_line(void) {
    klog_line_index = (klog_line_index + 1) % KLOG_LINES;
    klog_char_index = 0;
    // Clean out the rest of the line for aesthetic purposes
    for (unsigned int i = 0; i < KLOG_LINE_SIZE; i++) {
        klog_buffer[klog_line_index][i] = ' ';
    }
}