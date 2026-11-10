/* init_fini.c - _init and _fini for __libc_init_array / __libc_fini_array
 *
 * Linked with -nostartfiles: Reset_Handler (the startup file) is the entry, so
 * newlib's crt0 (_start, which calls exit and pulls in stdio) is not needed.
 * crti/crtn, which define these two, go with it. The C++ constructors still
 * run: __libc_init_array calls them through .init_array. */
void _init(void) {}
void _fini(void) {}
