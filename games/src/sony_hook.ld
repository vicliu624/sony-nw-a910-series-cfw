ENTRY(sony_home_entry)
SECTIONS {
 . = 0x00c00000;
 .text : { *(.text.entry) *(.text*) }
 .rodata : { *(.rodata*) }
 .data : { *(.data*) }
 .bss : { *(.bss*) *(COMMON) }
 /DISCARD/ : { *(.comment) *(.ARM.exidx*) *(.ARM.extab*) *(.note*) }
}
