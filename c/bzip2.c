/* bzip2 amalgamated translation unit */
#define BZ_NO_STDIO
#include "bzip2/bz_version.h"
#include "bzip2/bzlib.h"
#include "bzip2/bzlib_private.h"
#include "bzip2/blocksort.c"
#include "bzip2/huffman.c"
#include "bzip2/crctable.c"
#include "bzip2/randtable.c"
#include "bzip2/compress.c"
#include "bzip2/decompress.c"
#include "bzip2/bzlib.c"

void bz_internal_error(int errcode) {
    (void)errcode;
}
