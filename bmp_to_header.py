#!/usr/bin/env python

"""
Usage:

	python bmp_to_header.py inputfilename width height > out.h
"""

import sys

from PIL import Image

filename = sys.argv[1]

width  = int(sys.argv[2])
height = int(sys.argv[3])

# Remove alpha (replace with white)
im = Image.open(filename).convert("RGBA")
bg = Image.new("RGBA", im.size, (255,255,255,255))
bg.paste(im, (0,0), im)
im = bg.convert("1")

# Create C array
out = "prog_uchar {}[{}*{}] PROGMEM = {{\n".format(
	filename.split(".")[0],
	(width+7)//8,
	height
)
for y in range(im.size[1]):
	out += "\t"
	for x_byte in range((im.size[0]+7) // 8):
		byte = 0
		for x_bit in range(8):
			x = x_byte*8 + x_bit
			if x < width:
				byte |= int(not (im.getpixel((x,y)))) << x_bit
		out += "0x%02X, "%(byte)
	out += "\n"
out += "};"

print(out)
