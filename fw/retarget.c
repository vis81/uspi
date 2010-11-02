//------------------------------------------------------------------------------
/// \unit
///
/// !Purpose
///
/// This file Configures the target-dependent low level functions for character I/O.
///
/// !Contents
/// The code implement the lower-level functions as follows:
///    - fputc
///    - ferror
///    - _ttywrch
///    - _sys_exit
///
///
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------
#include "serial.h"
#include <stdio.h>

// Disable semihosting
#pragma import(__use_no_semihosting_swi) 

struct __FILE { int handle;} ;

//------------------------------------------------------------------------------
///  Outputs a character to a file.
//------------------------------------------------------------------------------
int fputc(int ch, FILE *f) {
    if ((f == stdout) || (f == stderr)) {
        kputchar(ch);
        return ch;
    }
    else {
        return EOF;
    }
}

//------------------------------------------------------------------------------
///  Returns the error status accumulated during file I/O.
//------------------------------------------------------------------------------
int ferror(FILE *f) {
    return EOF;
}


void _ttywrch(int ch) {
    kputchar((unsigned char)ch);
}


void _sys_exit(int return_code) {
    label:  goto label;  /* endless loop */
}
