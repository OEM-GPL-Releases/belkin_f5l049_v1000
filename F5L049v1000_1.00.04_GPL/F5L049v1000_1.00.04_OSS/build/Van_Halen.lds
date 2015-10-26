OUTPUT_FORMAT(elf32-tradbigmips)
OUTPUT_ARCH(mips)
ENTRY(startup_comp)

/* - ram_init
 * Put the code which is expand the kernel data from ROM to RAM.
 * - ram
 * Put the compressd kernel data and the code for uncompressing .
 */
MEMORY
{
	rom_kern    : org = 0xbf120000, len = 0x00180000
	rom_rootfs  : org = 0xbf2A0000, len = 0x00D50000
	ram_init    : org = 0x80002000, len = 0x00010000
	ram         : org = 0x81000000, len = 0x04000000
}

SECTIONS
{
  _start_rom_addr = 0xbf120000;
  _start_ram_addr = 0x81000000;

  /***********************************************/
  /* Boot code                                   */
  /***********************************************/
  .boot 0xbf120000 :
  {
    ../boot/boot.o(.text)
    . = ALIGN(16);
  } > rom_kern

  /***********************************************/
  /* Code for expanding from ROM to RAM          */
  /***********************************************/
  _start_init_ram_addr = 0x80002000;
  _init_start = .;

  .init 0x80002000 :
  AT( ADDR(.boot) + SIZEOF(.boot))
  {
    *(.init)
    . = ALIGN(16);
  } > ram_init

  /**********************************************/
  /* Code for uncompressing and compressd data  */
  /**********************************************/
  _init_end = ADDR(.boot) + SIZEOF(.boot) + SIZEOF(.init);
  _shift_addr   = _init_end;
  _kernel_begin = _init_end;

  .text 0x81000000 :
  AT( ADDR(.text) - _start_ram_addr + _shift_addr)
  {
    *(.text)
    *(.rodata)
    *(.rodata.str1.4)
    *(.rodata.cst4)
  } > ram

  _etext = .;

  .MIPS.options  :
  AT( ADDR(.sbss) - _start_ram_addr + _shift_addr )
  {
     *(.MIPS.options)
  } > ram

  .reginfo :
  AT( ADDR(.reginfo) - _start_ram_addr + _shift_addr )
  {
    *(.reginfo)
  } > ram

  .data    :
  AT( ADDR(.data) - _start_ram_addr + _shift_addr )
  {
    *(.data)
    CONSTRUCTORS
    . = ALIGN(16);
  } > ram

  _edata = .;

  .sbss :
  AT( ADDR(.sbss) - _start_ram_addr + _shift_addr )
  { 
    *(.sbss)
    *(.scommon)
  } > ram

  .bss       :
  AT( ADDR(.bss) - _start_ram_addr + _shift_addr )
  {
    *(.dynbss)
    *(.bss)
    *(COMMON)
    . = ALIGN(16) ;
    _end = . ;
    PROVIDE (end = .);
  } > ram 

   _kernel_end = _end - _start_ram_addr + _shift_addr;

  /***********************************************/
  /* ROOTFS                                      */
  /***********************************************/
  .rootfs 0xbf2A0000 : 
  {
    rootfs.o(.rootfs)
  } > rom_rootfs

  /***********************************************/
  /* ???                                         */
  /***********************************************/
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
}
