include <common.h>

// Coordinates of internal circuit boards
APM_X = BACK_THICKNESS + ARRAY_WIDTH/2 - APM_WIDTH - 2*LDR_RADIUS - 1.5;
APM_Y = BASE_LENGTH - BACK_THICKNESS - APM_HEIGHT - APM_PIN_HEIGHT;

JACK_X = APM_X - JACK_HEIGHT - 5.0;
JACK_Y = BASE_LENGTH-JACK_DEPTH;

// The base (erosion also protrudes the front face allowing pairs of these to
// form a hollow enclosure.
module base_shape(erosion = 0) {
	difference() {
		hull() {
			// Rounded base
			translate([0,0,erosion]) {
				translate([erosion,-erosion,0])
					cube([ARRAY_WIDTH-2*erosion, BASE_LENGTH-BASE_ROUNDING_RADIUS, 0.01]);
				translate([BASE_ROUNDING_RADIUS, BASE_LENGTH-BASE_ROUNDING_RADIUS, 0])
					cylinder(h = 0.01, r = BASE_ROUNDING_RADIUS-erosion);
				translate([ARRAY_WIDTH-BASE_ROUNDING_RADIUS, BASE_LENGTH-BASE_ROUNDING_RADIUS, 0])
					cylinder(h = 0.01, r = BASE_ROUNDING_RADIUS-erosion);
			}
			
			// Extend to the top of the hole
			rotate([90-DISPLAY_ANGLE,0,0])
				translate([ARRAY_WIDTH/2, BEZEL_WIDTH + BACK_HEIGHT - erosion,-BACK_INNER_DEPTH - BACK_THICKNESS])
					scale([ (ARRAY_WIDTH-2*erosion)/(BASE_TOP_ROUNDING_RADIUS-erosion)/2, 1, 1])
						cylinder(r = BASE_TOP_ROUNDING_RADIUS-erosion, h = BACK_THICKNESS+erosion);
		}
		
		// Trim off front to match angle of display
		translate([-0.5,-erosion,0])
			rotate([90-DISPLAY_ANGLE,0,0])
				cube([ARRAY_WIDTH+1, ARRAY_HEIGHT + 2*BEZEL_WIDTH, BASE_THICKNESS+erosion]);
	}
}
module jack() {
	cube([JACK_WIDTH, JACK_DEPTH, JACK_HEIGHT]);
}


module rubber_band_hook(height = 5, radius = 2) {
	cylinder(r = radius, h = height);
	translate([0,0,height])
		cylinder(r1 = radius, r2 = radius * 0.6, h = 1.5);
	translate([0,0,height + 1.5])
		cylinder(r1 = radius * 0.6, r2 = radius, h = 1.5);
}


module back() {
	difference () {
		union () {
			// The back of the display
			difference() {
				// Display backing
				rotate([-90-DISPLAY_ANGLE,0,0]) {
					translate([0,0,BACK_INNER_DEPTH + BACK_THICKNESS])
						rotate([180,0,0])
							translate([0,BEZEL_WIDTH,0])
							union () {
								difference () {
									bezel(BACK_INNER_DEPTH + BACK_THICKNESS, BEZEL_WIDTH);
									translate([0,0,BACK_THICKNESS])
										bezel(BACK_INNER_DEPTH+1, 0.01);
									
									// Screw holes
									translate([ -M4_NUT_RADIUS
									          , M4_NUT_RADIUS*1.5
									          , -0
									          ]) {
										cylinder(r=M4_BOLT_RADIUS, h=BACK_INNER_DEPTH+BACK_THICKNESS+1);
										cylinder(r=M4_BOLT_HEAD_RADIUS, h=M4_BOLT_HEAD_HEIGHT*2);
									}
									translate([ -M4_NUT_RADIUS
									          , ARRAY_HEIGHT - M4_NUT_RADIUS*1.5
									          , -0
									          ]) {
										cylinder(r=M4_BOLT_RADIUS, h=BACK_INNER_DEPTH+BACK_THICKNESS+1);
										cylinder(r=M4_BOLT_HEAD_RADIUS, h=M4_BOLT_HEAD_HEIGHT*2);
									}
									translate([ ARRAY_WIDTH+M4_NUT_RADIUS
									          , M4_NUT_RADIUS*1.5
									          , -0
									          ]) {
										cylinder(r=M4_BOLT_RADIUS, h=BACK_INNER_DEPTH+BACK_THICKNESS+1);
										cylinder(r=M4_BOLT_HEAD_RADIUS, h=M4_BOLT_HEAD_HEIGHT*2);
									}
									translate([ ARRAY_WIDTH+M4_NUT_RADIUS
									          , ARRAY_HEIGHT - M4_NUT_RADIUS*1.5
									          , -0
									          ]) {
										cylinder(r=M4_BOLT_RADIUS, h=BACK_INNER_DEPTH+BACK_THICKNESS+1);
										cylinder(r=M4_BOLT_HEAD_RADIUS, h=M4_BOLT_HEAD_HEIGHT*2);
									}
								}
							}
				}
				
				// Flatten off bottom
				translate([-BEZEL_WIDTH,0,-1000])
					cube([ARRAY_WIDTH + BEZEL_WIDTH*2, 1000, 1000]);
				
				// Open up back
				rotate([90-DISPLAY_ANGLE,0,0])
					translate([0, BEZEL_WIDTH,-BACK_INNER_DEPTH - BACK_THICKNESS - 0.5])
						cube([ARRAY_WIDTH, BACK_HEIGHT, BACK_INNER_DEPTH + BACK_THICKNESS + 1]);
			}
			
			rotate([-90-DISPLAY_ANGLE,0,0])
				translate([0,0,BACK_INNER_DEPTH + BACK_THICKNESS])
					rotate([180,0,0])
						translate([0,BEZEL_WIDTH,0])
							// Supports for display
							translate([0,0,BACK_THICKNESS+BACK_INNER_DEPTH - BACK_SUPPORT_THICKNESS]) {
								translate([0, ARRAY_HEIGHT/4, 0])
									cylinder(r = BACK_SUPPORT_RADIUS, h = BACK_SUPPORT_THICKNESS);
								translate([0, ARRAY_HEIGHT*3/4, 0])
									cylinder(r = BACK_SUPPORT_RADIUS, h = BACK_SUPPORT_THICKNESS);
								translate([ARRAY_WIDTH, ARRAY_HEIGHT/4, 0])
									cylinder(r = BACK_SUPPORT_RADIUS, h = BACK_SUPPORT_THICKNESS);
								translate([ARRAY_WIDTH, ARRAY_HEIGHT*3/4, 0])
									cylinder(r = BACK_SUPPORT_RADIUS, h = BACK_SUPPORT_THICKNESS);
							}
			
			// The base itself
			difference() {
				base_shape();
				base_shape(BASE_THICKNESS);
				
				// Cut off any excess
				rotate([90-DISPLAY_ANGLE,0,0])
					translate([0, BEZEL_WIDTH,-BACK_INNER_DEPTH])
						cube([ARRAY_WIDTH, ARRAY_HEIGHT, BACK_INNER_DEPTH]);
			}
			
			// Mounting hooks for PCBs
			
			// Arduino Pro Mini
			translate([APM_X - 2.5, APM_Y, BASE_THICKNESS]) {
				translate([0,APM_HEIGHT/4,0])
					rubber_band_hook(APM_THICKNESS);
				translate([0,APM_HEIGHT*3/4,0])
					rubber_band_hook(APM_THICKNESS);
				
				translate([APM_WIDTH + 5,APM_HEIGHT/4,0])
					rubber_band_hook(APM_THICKNESS);
				translate([APM_WIDTH + 5,APM_HEIGHT*3/4,0])
					rubber_band_hook(APM_THICKNESS);
			}
			
			// Other electronics (extra hooks on left/right side of APM)
			translate([BACK_THICKNESS + 4, APM_Y, BASE_THICKNESS]) {
				translate([0,APM_HEIGHT/4,0])
					rubber_band_hook(APM_THICKNESS);
				translate([2,APM_HEIGHT*3/4,0])
					rubber_band_hook(APM_THICKNESS);
				
				translate([ARRAY_WIDTH - 3*BACK_THICKNESS - 5,APM_HEIGHT/4,0])
					rubber_band_hook(APM_THICKNESS);
				translate([ARRAY_WIDTH - 3*BACK_THICKNESS - 7,APM_HEIGHT*3/4,0])
					rubber_band_hook(APM_THICKNESS);
			}
			
			// Back support for jack
			translate([JACK_X, JACK_Y-2-3, BASE_THICKNESS])
				cube([JACK_HEIGHT,3,JACK_WIDTH*0.3]);
		}
		
		// Cut out a hole for the DC jack
		translate([JACK_X, JACK_Y, BASE_THICKNESS])
			translate([0,0,JACK_WIDTH])
				rotate([0,90,0])
					jack();
		
		// Hole for APM programmer pins
		translate([APM_X + (APM_WIDTH-APM_PIN_WIDTH)/2, APM_Y + APM_HEIGHT, BASE_THICKNESS + APM_THICKNESS])
			cube([APM_PIN_WIDTH, APM_PIN_HEIGHT*2, APM_PIN_THICKNESS]);
		
		
		// Cut out a hole for the LDR
		translate([ARRAY_WIDTH/2, BASE_LENGTH/2, -0.5])
			cylinder(r = LDR_RADIUS, h = BASE_THICKNESS+1);
		
		// Feet
		translate([FOOT_RADIUS+BASE_THICKNESS, FOOT_RADIUS+BASE_THICKNESS, -1])
			cylinder(r = FOOT_RADIUS, h = FOOT_INSET + 1);
		translate([ARRAY_WIDTH-FOOT_RADIUS-BASE_THICKNESS, FOOT_RADIUS+BASE_THICKNESS, -1])
			cylinder(r = FOOT_RADIUS, h = FOOT_INSET + 1);
		translate([FOOT_RADIUS+BASE_ROUNDING_RADIUS, BASE_LENGTH-FOOT_RADIUS-BASE_THICKNESS, -1])
			cylinder(r = FOOT_RADIUS, h = FOOT_INSET + 1);
		translate([ARRAY_WIDTH-FOOT_RADIUS-BASE_ROUNDING_RADIUS, BASE_LENGTH-FOOT_RADIUS-BASE_THICKNESS, -1])
			cylinder(r = FOOT_RADIUS, h = FOOT_INSET + 1);
		
		// Truncate (for test prints)
		//translate([-100, -100, 15])
		//	cube([1000, 1000, 1000]);
	}
}

back();

// Blanks for various circuit boards
%translate([APM_X, APM_Y, BASE_THICKNESS])
	cube([APM_WIDTH, APM_HEIGHT, APM_THICKNESS]);
translate([APM_X + (APM_WIDTH-APM_PIN_WIDTH)/2, APM_Y + APM_HEIGHT, BASE_THICKNESS + APM_THICKNESS])
	%cube([APM_PIN_WIDTH, APM_PIN_HEIGHT, APM_PIN_THICKNESS]);

%translate([(ARRAY_WIDTH*3)/5 - 5, BASE_LENGTH - 4*BACK_THICKNESS - RTC_WIDTH, BASE_THICKNESS])
	cube([RTC_HEIGHT, RTC_WIDTH, RTC_THICKNESS]);

%translate([JACK_X, JACK_Y-2, BASE_THICKNESS])
	translate([0,0,JACK_WIDTH])
		rotate([0,90,0])
			jack();
