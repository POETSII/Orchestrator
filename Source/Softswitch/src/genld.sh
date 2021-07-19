#!/bin/bash

# Load config parameters
while read -r EXPORT; do
    eval $EXPORT;
done <<< `${TINSEL_ROOT}/config.py envs`

# Compute space available for instructions
MaxInstrBytes=$((4 * 2**$LogInstrsPerCore - $MaxBootImageBytes))
# Compute space available for thread data
BytesPerDRAMPartition=$((2**$LogBytesPerDRAMPartition))
CoresPerDRAM=$((2**($LogCoresPerDCache+$LogDCachesPerDRAM)))
GlobalBytesPerCore=$((($BytesPerDRAMPartition*$ThreadsPerCore)-(1048576/$CoresPerDRAM)))
CoreDRAMNum=$((($1>>$LogThreadsPerCore)%$CoresPerDRAM))
InterleavedPartitionOffset=0x80000000

cat - << EOF
/* THIS FILE HAS BEEN GENERATED AUTOMATICALLY. */
/* DO NOT MODIFY. INSTEAD, MODIFY THE genld.sh SCRIPT. */

OUTPUT_ARCH( "riscv" )

MEMORY
{
  instrs  : ORIGIN = $MaxBootImageBytes, LENGTH = $MaxInstrBytes
  globals : ORIGIN = (0x100000+($CoreDRAMNum*$GlobalBytesPerCore)) | $InterleavedPartitionOffset, LENGTH = $GlobalBytesPerCore
  thread0 : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 1)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  thread1 : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 2)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  thread2 : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 3)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  thread3 : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 4)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  thread4 : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 5)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  thread5 : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 6)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  thread6 : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 7)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  thread7 : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 8)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  thread8 : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 9)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  thread9 : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 10)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  threadA : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 11)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  threadB : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 12)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  threadC : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 13)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  threadD : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 14)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  threadE : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 15)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
  threadF : ORIGIN = ($BytesPerDRAM - ((($CoreDRAMNum << $LogThreadsPerCore) + 16)*$BytesPerDRAMPartition)) | $InterleavedPartitionOffset, LENGTH = $BytesPerDRAMPartition
}

SECTIONS
{
  .text   : { *.o(.text*) }                              > instrs
  .thr0_base : { *(.thr0_base) }                         > thread0
  .t0data : { vars_*_0.o(.sdata* .data* .bss* .sbss*) }  > thread0
  .thr1_base : { *(.thr1_base) }                         > thread1
  .t1data : { vars_*_1.o(.sdata* .data* .bss* .sbss*) }  > thread1
  .thr2_base : { *(.thr2_base) }                         > thread2
  .t2data : { vars_*_2.o(.sdata* .data* .bss* .sbss*) }  > thread2
  .thr3_base : { *(.thr3_base) }                         > thread3
  .t3data : { vars_*_3.o(.sdata* .data* .bss* .sbss*) }  > thread3
  .thr4_base : { *(.thr4_base) }                         > thread4
  .t4data : { vars_*_4.o(.sdata* .data* .bss* .sbss*) }  > thread4
  .thr5_base : { *(.thr5_base) }                         > thread5
  .t5data : { vars_*_5.o(.sdata* .data* .bss* .sbss*) }  > thread5
  .thr6_base : { *(.thr6_base) }                         > thread6
  .t6data : { vars_*_6.o(.sdata* .data* .bss* .sbss*) }  > thread6
  .thr7_base : { *(.thr7_base) }                         > thread7
  .t7data : { vars_*_7.o(.sdata* .data* .bss* .sbss*) }  > thread7
  .thr8_base : { *(.thr8_base) }                         > thread8
  .t8data : { vars_*_8.o(.sdata* .data* .bss* .sbss*) }  > thread8
  .thr9_base : { *(.thr9_base) }                         > thread9
  .t9data : { vars_*_9.o(.sdata* .data* .bss* .sbss*) }  > thread9
  .thr10_base : { *(.thr10_base) }                       > threadA
  .tAdata : { vars_*_10.o(.sdata* .data* .bss* .sbss*) } > threadA
  .thr11_base : { *(.thr11_base) }                       > threadB
  .tBdata : { vars_*_11.o(.sdata* .data* .bss* .sbss*) } > threadB
  .thr12_base : { *(.thr12_base) }                       > threadC
  .tCdata : { vars_*_12.o(.sdata* .data* .bss* .sbss*) } > threadC
  .thr13_base : { *(.thr13_base) }                       > threadD
  .tDdata : { vars_*_13.o(.sdata* .data* .bss* .sbss*) } > threadD
  .thr14_base : { *(.thr14_base) }                       > threadE
  .tEdata : { vars_*_14.o(.sdata* .data* .bss* .sbss*) } > threadE
  .thr15_base : { *(.thr15_base) }                       > threadF
  .tFdata : { vars_*_15.o(.sdata* .data* .bss* .sbss*) } > threadF
  .bss    : { *.o(.bss*) }                  > globals = 0
  .rodata : { *.o(.rodata*) }               > globals
  .sdata  : { *.o(.sdata*) }                > globals
  .data   : { *.o(.data*) }                 > globals
  __heapBase = ALIGN(.);
}
EOF
