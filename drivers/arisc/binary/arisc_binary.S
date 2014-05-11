	.globl	arisc_binary_start
	.globl	arisc_binary_end
arisc_binary_start:
#if defined CONFIG_ARCH_SUN8IW1P1
	.incbin	"drivers/arisc/binary/arisc_sun8iw1p1.code"
#elif defined CONFIG_ARCH_SUN8IW3P1
	.incbin	"drivers/arisc/binary/arisc_sun8iw3p1.code"
#elif defined CONFIG_ARCH_SUN9IW1P1
	.incbin	"drivers/arisc/binary/arisc_sun9iw1p1.code"
#else
#error "please select a platform\n"
#endif
arisc_binary_end:
	.align	2
