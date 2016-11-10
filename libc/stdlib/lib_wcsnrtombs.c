
/*
DESCRIPTION
The <<wcsrtombs>> function converts a string of wide characters indirectly
pointed to by <[src]> to a corresponding multibyte character string stored in
the array pointed to by <[dst}>.  No more than <[len]> bytes are written to
<[dst}>.

If <[dst}> is NULL, no characters are stored.

If <[dst}> is not NULL, the pointer pointed to by <[src]> is updated to point
to the character after the one that conversion stopped at.  If conversion
stops because a null character is encountered, *<[src]> is set to NULL.

The mbstate_t argument, <[ps]>, is used to keep track of the shift state.  If
it is NULL, <<wcsrtombs>> uses an internal, static mbstate_t object, which
is initialized to the initial conversion state at program startup.

The <<wcsnrtombs>> function behaves identically to <<wcsrtombs>>, except that
conversion stops after reading at most <[nwc]> characters from the buffer
pointed to by <[src]>.

RETURNS
The <<wcsrtombs>> and <<wcsnrtombs>> functions return the number of bytes
stored in the array pointed to by <[dst]> (not including any terminating
null), if successful, otherwise it returns (size_t)-1.

PORTABILITY
<<wcsrtombs>> is defined by C99 standard.
<<wcsnrtombs>> is defined by the POSIX.1-2008 standard.
*/

#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

size_t wcsnrtombs(FAR char *dst, FAR const wchar_t **src, size_t nwc, size_t len, mbstate_t *ps)
{
  return len;
} 

