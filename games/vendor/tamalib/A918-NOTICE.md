Upstream: https://github.com/jcrona/tamalib
Commit: ce304d55f9a73c60232ce3f552e7983db3fa399c
License: GPL-2.0-or-later; original notices and LICENSE retained.

A918 changes to cpu.c: move previous_cycles into snapshot state, initialize the
HALT interrupt selector, reset timer/pipeline state on reset, and add a bounded,
pointer-free little-endian snapshot export/import. hal_types.h is the upstream
template copied one directory above this library, as required by hal.h.
