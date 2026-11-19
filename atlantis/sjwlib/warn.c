/*

    File:           warn.c
    
    Created:        Tue Sep 24 16:04:59 EST 1991
    
    Author:         Stephen Walker
                    CSIRO Division of Oceanography
    
    Purpose:        Print out a warning message to stderr
    
    Arguments:      s - pointer to the string to print

    Returns:        void
    
    Revisions:      none

    $Id: warn.c 3126 2012-04-13 01:23:55Z gor171 $

*/

/** Print out a warning message to stderr.
  *
  * @param format pointer to the printf-like formatted string.
  * @param ellipsis variable arguments.
  */
#include <stdarg.h>
#include <stdio.h>
#include "sjwlib.h"

int suppress_runtime_warnings = 0;

void warn(char *format, ...)
{
    va_list args;

    if (suppress_runtime_warnings) {
        return;
    }

    fprintf(stderr,"WARNING: ");
    va_start(args,format);
    vfprintf(stderr,format,args);
    va_end(args);
}


void quiet(char *format, ...)
{

}

void (*keyprm_errfn)(char *format, ...);// = quit;
