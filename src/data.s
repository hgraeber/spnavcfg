	.section .rodata

	.globl devices_png
	.globl devices_png_size
devices_png:
	.incbin "icons/devices.png"
devices_png_end:
	.align 4
devices_png_size:
	.long devices_png_end - devices_png_size
