	.globl	mdfs_bin_start
mdfs_bin_start:
	.incbin	"drivers/devfreq/dramfreq/mdfs/mdfs.code"
	.globl	mdfs_bin_end
mdfs_bin_end:
	.align	2
