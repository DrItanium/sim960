.section "ram", "awx", @nobits
ram_base:
.section "BootCode", "x", @progbits
.org 0
boot:

.section processor_space, "w", @progbits
PROCESSOR_SPACE_START:
IBR_BASE:
.long boot
.long pcrb_location


