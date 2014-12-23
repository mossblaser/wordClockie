include <common.h>

module face() {
	translate([0,0,BEZEL_DEPTH + ARRAY_DEPTH])
		rotate([180,0,0])
			difference() {
				bezel(ARRAY_DEPTH + BEZEL_DEPTH);
				
				// Cut out for array
				translate([0.0, 0.0, BEZEL_DEPTH])
					cube([ARRAY_WIDTH, ARRAY_HEIGHT, ARRAY_DEPTH + 1]);
				
				// Cut out for face of array
				translate([ (ARRAY_WIDTH - FACE_OPENING_WIDTH)/2
				          , (ARRAY_HEIGHT - FACE_OPENING_HEIGHT)/2
				          , -0.5
				          ])
					cube([FACE_OPENING_WIDTH, FACE_OPENING_HEIGHT, BEZEL_DEPTH + 1]);
				
				// Chamfer edge of array face to aid overhang printing
				hull () {
					translate([ (ARRAY_WIDTH - FACE_OPENING_WIDTH)/2
					          , (ARRAY_HEIGHT - FACE_OPENING_HEIGHT)/2
					          , 0.0
					          ])
						cube([FACE_OPENING_WIDTH,FACE_OPENING_HEIGHT,0.01]);
					translate([0.0, 0.0, BEZEL_DEPTH])
						cube([ARRAY_WIDTH,ARRAY_HEIGHT,1]);
				}
				
				// Screw holes
				translate([ -M4_NUT_RADIUS
				          , M4_NUT_RADIUS*1.5
				          , -0
				          ]) {
					translate([0, 0, BEZEL_DEPTH*1.5])
						cylinder(r=M4_BOLT_RADIUS, h=BEZEL_DEPTH+ARRAY_DEPTH+1);
					translate([-M4_NUT_RADIUS, -M4_NUT_RADIUS, ARRAY_DEPTH - M4_NUT_HEIGHT])
						cube([BEZEL_WIDTH, M4_NUT_RADIUS*2, M4_NUT_HEIGHT]);
					hull () {
						translate([0, 0, BEZEL_DEPTH + M4_NUT_HEIGHT])
							cylinder(r=M4_BOLT_RADIUS, h=0.1);
						translate([-M4_NUT_RADIUS, -M4_NUT_RADIUS, ARRAY_DEPTH - M4_NUT_HEIGHT])
							cube([M4_NUT_RADIUS*2, M4_NUT_RADIUS*2, M4_NUT_HEIGHT]);
					}
				}
				translate([ -M4_NUT_RADIUS
				          , ARRAY_HEIGHT - M4_NUT_RADIUS*1.5
				          , -0
				          ]) {
					translate([0, 0, BEZEL_DEPTH*1.5])
						cylinder(r=M4_BOLT_RADIUS, h=BEZEL_DEPTH+ARRAY_DEPTH+1);
					translate([-M4_NUT_RADIUS, -M4_NUT_RADIUS, ARRAY_DEPTH - M4_NUT_HEIGHT])
						cube([BEZEL_WIDTH, M4_NUT_RADIUS*2, M4_NUT_HEIGHT]);
					hull () {
						translate([0, 0, BEZEL_DEPTH + M4_NUT_HEIGHT])
							cylinder(r=M4_BOLT_RADIUS, h=0.1);
						translate([-M4_NUT_RADIUS, -M4_NUT_RADIUS, ARRAY_DEPTH - M4_NUT_HEIGHT])
							cube([M4_NUT_RADIUS*2, M4_NUT_RADIUS*2, M4_NUT_HEIGHT]);
					}
				}
				
				translate([ ARRAY_WIDTH+M4_NUT_RADIUS
				          , M4_NUT_RADIUS*1.5
				          , -0
				          ]) {
					translate([0, 0, BEZEL_DEPTH*1.5])
						cylinder(r=M4_BOLT_RADIUS, h=BEZEL_DEPTH+ARRAY_DEPTH+1);
					translate([-BEZEL_WIDTH+M4_NUT_RADIUS, -M4_NUT_RADIUS, ARRAY_DEPTH - M4_NUT_HEIGHT])
						cube([BEZEL_WIDTH, M4_NUT_RADIUS*2, M4_NUT_HEIGHT]);
					hull () {
						translate([0, 0, BEZEL_DEPTH + M4_NUT_HEIGHT])
							cylinder(r=M4_BOLT_RADIUS, h=0.1);
						translate([-M4_NUT_RADIUS, -M4_NUT_RADIUS, ARRAY_DEPTH - M4_NUT_HEIGHT])
							cube([M4_NUT_RADIUS*2, M4_NUT_RADIUS*2, M4_NUT_HEIGHT]);
					}
				}
				translate([ ARRAY_WIDTH+M4_NUT_RADIUS
				          , ARRAY_HEIGHT - M4_NUT_RADIUS*1.5
				          , -0
				          ]) {
					translate([0, 0, BEZEL_DEPTH*1.5])
						cylinder(r=M4_BOLT_RADIUS, h=BEZEL_DEPTH+ARRAY_DEPTH+1);
					translate([-BEZEL_WIDTH+M4_NUT_RADIUS, -M4_NUT_RADIUS, ARRAY_DEPTH - M4_NUT_HEIGHT])
						cube([BEZEL_WIDTH, M4_NUT_RADIUS*2, M4_NUT_HEIGHT]);
					hull () {
						translate([0, 0, BEZEL_DEPTH + M4_NUT_HEIGHT])
							cylinder(r=M4_BOLT_RADIUS, h=0.1);
						translate([-M4_NUT_RADIUS, -M4_NUT_RADIUS, ARRAY_DEPTH - M4_NUT_HEIGHT])
							cube([M4_NUT_RADIUS*2, M4_NUT_RADIUS*2, M4_NUT_HEIGHT]);
					}
				}
			}
}

face();
