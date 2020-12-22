.section "ram", "awx", @nobits
ram_base:
.section "BootCode", "x", @progbits
data_start:
	lda PROCESSOR_SPACE_START, g0
	lda ram_base, g1

.section processor_space, "w", @progbits
PROCESSOR_SPACE_START:
IBR_BASE:
.long data_start
.long pcrb_location


