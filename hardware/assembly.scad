include <common.h>
use <back.scad>
use <face.scad>

back();
rotate([90-DISPLAY_ANGLE,0,0])
	translate([0, ARRAY_HEIGHT+BEZEL_WIDTH, 0])
		face();
