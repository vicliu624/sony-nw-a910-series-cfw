ENTRY(sony_home_entry)
SECTIONS {
 . = 0x00c00000;
 .text : { *(.text.entry) *(.text*) }
 .rodata : { *(.rodata*) }
 .data : { *(.data*) }
 __payload_file_end = .;
 .bss (NOLOAD) : { *(.bss*) *(COMMON) }
 __payload_mem_end = .;
 /DISCARD/ : { *(.comment) *(.ARM.exidx*) *(.ARM.extab*) *(.note*) }
}
