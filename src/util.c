#include "util.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void log_message ( const char *formatStr, ... )
{
  va_list params;
  char buf[BUFSIZ];

  va_start ( params, formatStr );
  vsprintf ( buf, formatStr, params );

  printf ( "%s\n", buf );

  va_end ( params );
}
