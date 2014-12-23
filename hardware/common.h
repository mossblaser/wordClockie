$fn=60;

FACE_OPENING_WIDTH = 112;
FACE_OPENING_HEIGHT = 105;

ARRAY_WIDTH = 115.5;
ARRAY_HEIGHT = 108;
ARRAY_DEPTH = 8.5;

BEZEL_DEPTH = 1.5;
BEZEL_WIDTH = 8;

BACK_THICKNESS = 3;
BACK_HEIGHT = ARRAY_HEIGHT*.666;

// XXX: Not based on actual requirements
BACK_INNER_DEPTH = 6;

BACK_SUPPORT_RADIUS = 4;
BACK_SUPPORT_THICKNESS = 2;

BASE_LENGTH = 57;
BASE_THICKNESS = 3;
BASE_ROUNDING_RADIUS = 20;
BASE_TOP_ROUNDING_RADIUS = 10;

// XXX: Guessed sizes
JACK_WIDTH = 9+0.5;
JACK_HEIGHT = 11+0.5;
JACK_DEPTH = 15;

FOOT_RADIUS = 16.5/2;
FOOT_INSET = 2;

// Adruino Pro Mini Dimensions
APM_WIDTH = 18;
APM_HEIGHT = 33;
APM_THICKNESS = 4.5; // XXX Guess

// Programmer pins
APM_PIN_WIDTH = 17.78;
APM_PIN_HEIGHT = 5.84;
APM_PIN_THICKNESS = 3.81;

// Realtime clock module
RTC_WIDTH = 22;
RTC_HEIGHT = 38;
RTC_THICKNESS = 14;

LDR_RADIUS = 6.1/2;

DISPLAY_ANGLE = 15;

M4_BOLT_HEAD_RADIUS = 5.8/2;
M4_BOLT_HEAD_HEIGHT = 2.5;
M4_BOLT_RADIUS = 3.75/2;
M4_NUT_RADIUS = 3.0;
M4_NUT_HEIGHT = 2.7;

module bezel(depth, bezel_width = BEZEL_WIDTH) {
	difference() {
		hull () {
			// Rounded Face
			translate([0,0,bezel_width])
				sphere(r = bezel_width);
			translate([ARRAY_WIDTH,0,bezel_width])
				sphere(r = bezel_width);
			translate([0,ARRAY_HEIGHT,bezel_width])
				sphere(r = bezel_width);
			translate([ARRAY_WIDTH,ARRAY_HEIGHT,bezel_width])
				sphere(r = bezel_width);
			
			// Flat back
			translate([0.0, 0.0, bezel_width + BEZEL_DEPTH]) {
				translate([0,0,0])
					cylinder(h = depth - bezel_width, r = bezel_width);
				translate([ARRAY_WIDTH,0,0])
					cylinder(h = depth - bezel_width, r = bezel_width);
				translate([0,ARRAY_HEIGHT,0])
					cylinder(h = depth - bezel_width, r = bezel_width);
				translate([ARRAY_WIDTH,ARRAY_HEIGHT,0])
					cylinder(h = depth - bezel_width, r = bezel_width);
			}
		}
		
		// Trim the height
		translate([-bezel_width-0.5,-bezel_width-0.5, depth])
			cube([ ARRAY_WIDTH + 2*bezel_width + 1
			     , ARRAY_HEIGHT + 2*bezel_width + 1
			     , 1000
			     ]);
	}
}


echo( "Depth of screw shaft (not including head):",
      (-0.5*BEZEL_DEPTH) + ARRAY_DEPTH // In face
      + BACK_INNER_DEPTH+BACK_THICKNESS-M4_BOLT_HEAD_HEIGHT*2 // In back
    );
