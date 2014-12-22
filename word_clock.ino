/**
 * Firmware for a word clock for a special Cube wife.
 *
 * Hardware consists of an array of MAX7219/MAX7221 driven LED arrays, a
 * realtime clock and an LDR and tilt switches for I/O.
 */

#include <Time.h>
#include <DS1307RTC.h>
#include <Wire.h>

// XXX: Must be included in this file to work around defficiencies in the
// Arduino toolchain...
#include <SPI.h>

#include "UnicomReceiver.h"
#include "display.h"
#include "word_clock.h"
#include "words.h"
#include "tween.h"
#include "automata.h"
#include "text.h"
#include "face.h"


////////////////////////////////////////////////////////////////////////////////
// Display buffer
////////////////////////////////////////////////////////////////////////////////

// Pair of frame buffers to contain current time and previous time
char buf_a[HEIGHT * WIDTH];
char buf_b[HEIGHT * WIDTH];

// Current and previous frame buffers
char *prev_buf = buf_a;
char *cur_buf  = buf_b;

// Switch the buffer in use
void flip() {
	if (prev_buf == buf_a) {
		prev_buf = buf_b;
		cur_buf  = buf_a;
	} else {
		prev_buf = buf_a;
		cur_buf  = buf_b;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Unicom receiever (optical clock setting interface logic)
////////////////////////////////////////////////////////////////////////////////


// Are we currently updating via unicom?
bool unicom_updating = false;

// Proportion of bits received successfully (0 - 1 inclusive).
int unicom_bits_arrived;

const int unicom_num_bits = 32;

UnicomReceiver unicom(LDR_PIN);

/**
 * Function which uses Unicom to update the current time. This should be called
 * frequently.
 */
void unicom_loop() {
	// Step the Unicom state machine
	unicom.refresh();
	
	// Accumulate incoming time data
	static size_t bytes_received;
	static unsigned long received_time;
	static unsigned long receive_started;
	
	UnicomReceiver::state_t state = unicom.getState();
	switch (state) {
		default:
		case UnicomReceiver::STATE_SYNCING:
			// No valid data coming in
			unicom_updating = false;
			break;
		
		case UnicomReceiver::STATE_LOCKED:
			// We've got a clock signal, data will arrive shortly
			unicom_updating = true;
			unicom_bits_arrived = 0;
			
			bytes_received = 0;
			received_time = 0ul;
			receive_started = millis();
			break;
		
		case UnicomReceiver::STATE_RECEIVING:
			char c;
			if (unicom.getByte(&c)) {
				bytes_received++;
				
				// Accumulate first word of data (the time)
				if (bytes_received <= 4) {
					received_time |= ((unsigned long)c & 0xFFul) << ((bytes_received-1)*8);
				}
				
				// Once the time has arrived, update the RTC
				if (bytes_received == 4) {
					// Compensate for time spent transmitting
					received_time += (500ul + millis() - receive_started) / 1000ul;
					RTC.set(received_time);
					setTime(received_time);
					Serial.println(F("INFO: Successfuly updated clock using unicom."));
					unicom_updating = false;
				}
			}
			
			// Update the progress indicator
			unicom_bits_arrived = (bytes_received*8) + unicom.getBitsReceived();
			if (unicom_bits_arrived > unicom_num_bits)
				unicom_bits_arrived = unicom_num_bits;
			
			break;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Shake detection
////////////////////////////////////////////////////////////////////////////////

bool shaking = false;

void shake_detect_loop() {
	static const int pins[2] = {TILT_LEFT_PIN, TILT_RIGHT_PIN};
	static const int num_pins = sizeof(pins)/sizeof(pins[0]);
	
	static bool last_value[2];
	static unsigned long last_change_time[2] = {0,0};
	
	unsigned long now = millis();
	
	// Detect shaking
	bool is_shaking = true;
	for (int i = 0; i < num_pins; i++) {
		bool cur_value = digitalRead(pins[i]);
		if (cur_value != last_value[i]) {
			last_change_time[i] = now;
			last_value[i] = cur_value;
		}
		is_shaking &= (now - last_change_time[i]) < SHAKE_MAX_PERIOD;
	}
	
	static unsigned long shake_start = 0;
	
	// Detect When shaking has been going on long enough
	if (!is_shaking) {
		shake_start = now;
		shaking = false;
	} else {
		if ((now - shake_start) >= SHAKE_MIN_DURATION)
			shaking = true;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Setup code
////////////////////////////////////////////////////////////////////////////////

void setup() {
	// Setup debug
	Serial.begin(115200);
	Serial.println(F("INFO: Word Clock Firmware Running..."));
	
	// Setup display drivers
	display_begin();
	
	// Setup realtime clock
	setSyncProvider(RTC.get);
	if(timeStatus()!= timeSet) 
		Serial.println(F("ERROR: Unable get time from RTC."));
	else
		Serial.println(F("INFO: Successfuly read time from RTC"));
	
	// Set pull-ups for tilt switches
	pinMode(TILT_LEFT_PIN, INPUT);  digitalWrite(TILT_LEFT_PIN, HIGH);
	pinMode(TILT_RIGHT_PIN, INPUT); digitalWrite(TILT_RIGHT_PIN, HIGH);
	
	// Drive power to LDR potential divider
	pinMode(LDR_P_PIN, OUTPUT); digitalWrite(LDR_P_PIN, HIGH);
	pinMode(LDR_N_PIN, OUTPUT); digitalWrite(LDR_N_PIN, LOW);
	
	// Initially clear the display buffers
	for (int i = 0; i < WIDTH*HEIGHT; i++) {
		buf_a[i] = 0;
		buf_b[i] = 0;
	}
	
	// Seed the PRNG
	randomSeed(analogRead(LDR_PIN));
}


////////////////////////////////////////////////////////////////////////////////
// Main UI state-machine
////////////////////////////////////////////////////////////////////////////////


typedef enum {
	STATE_RESET,
	STATE_RESET_MESSAGE,
	STATE_CLOCK,
	STATE_CLOCK_UPDATE,
	STATE_SCROLL_MESSAGE,
	STATE_SCROLL_MESSAGE_UPDATE,
	STATE_MARRIAGE_DURATION,
	STATE_MARRIAGE_DURATION_UPDATE,
	STATE_MOTD,
	STATE_UNICOM,
	STATE_UNICOM_LOCKED,
	STATE_UNICOM_TRANSFERRING,
	STATE_UNICOM_OK,
	STATE_UNICOM_ERROR,
	STATE_SHAKE,
	STATE_LOVE_NOTE,
	STATE_LOVE_NOTE_UPDATE,
	STATE_AUTOMATA,
	STATE_AUTOMATA_UPDATE,
} state_t;



void loop() {
	unicom_loop();
	shake_detect_loop();
	
	// Current state
	static state_t state = STATE_RESET;
	
	// Used by STATE_CLOCK_UPDATE: the time most recently displayed.
	static int last_minute;
	static int last_hour;
	
	// Is a tween currently running?
	static bool tween_running;
	
	// For the implementation of timers: the time when the timer was started.
	static unsigned long last_time = 0;
	
	// For use when entering STATE_SCROLL_MESSAGE: state to enter after displaying
	// a scrolling message.
	static state_t post_scrolling_message_state;
	
	// For image sequences (e.g. STATE_LOVE_NOTE* and STATE_AUTOMATA*): the
	// current frame number
	static int animation_frame = 0;
	
	// Used for STATE_MARRIAGE_DURATION*: Marriage duration (fields are zeroed
	// after they've been displayed)
	static int aniversary_years;
	static int aniversary_months;
	static int aniversary_weeks;
	static int aniversary_days;
	
	// The time at which the message of the day should next be displayed
	static int motd_minute;
	static int motd_hour;
	
	// General purpose string-constructing buffer
	static char str[100];
	
	// Global state override: force the state machine to show unicom state during
	// updates
	if (unicom_updating) {
		switch (state) {
			case STATE_UNICOM:
			case STATE_UNICOM_LOCKED:
			case STATE_UNICOM_TRANSFERRING:
			case STATE_UNICOM_OK:
			case STATE_UNICOM_ERROR:
				// Do nothing; already showing unicom status
				break;
			
			default:
				// Enter unicom state
				state = STATE_UNICOM;
				break;
		}
	}
	
	// State machine logic
	switch (state) {
		case STATE_RESET:
			// On power up, display a friendly message
			Serial.println(F("INFO: UI state machine reset"));
			flip();
			words_set_mask(cur_buf, "for cube *");
			tween_start(prev_buf, cur_buf, TWEEN_FADE_FROM_BLACK, RESET_MESSAGE_TWEEN_FRAMES);
			state = STATE_RESET_MESSAGE;
			last_time = millis();
			break;
		
		case STATE_RESET_MESSAGE:
			// Let the message linger for some time after reset before showing the
			// message of the day
			if ((millis() - last_time) >= RESET_MESSAGE_TIMEOUT_MSEC)
				state = STATE_MOTD;
			break;
		
		case STATE_CLOCK:
		case STATE_CLOCK_UPDATE:
			{
				// Update the time, if it has changed (or just entering the clock state)
				time_t t = now();
				if (state == STATE_CLOCK || hour(t) != last_hour || minute(t) != last_minute) {
					last_minute = minute(t);
					last_hour = hour(t);
					
					// Render the time into the current buffer
					strcpy(str, "it is ");
					words_append_time(str, hour(t), minute(t));
					
					// Start animating a transition
					flip();
					words_set_mask(cur_buf, str);
					if (state == STATE_CLOCK)
						tween_start(prev_buf, cur_buf, TWEEN_FADE_THROUGH_BLACK, CLOCK_TWEEN_FRAMES);
					else
						tween_start(prev_buf, cur_buf, TWEEN_FADE, CLOCK_UPDATE_TWEEN_FRAMES);
					
					state = STATE_CLOCK_UPDATE;
				} else if (!tween_running && hour(t) == motd_hour && minute(t) == motd_minute) {
					state = STATE_MOTD;
				} else {
					state = STATE_CLOCK_UPDATE;
				}
				
				// React to shaking
				if (shaking)
					state = STATE_SHAKE;
			}
			break;
		
		case STATE_SCROLL_MESSAGE:
			// Start displaying a scrolling message
			flip();
			for (int i = 0; i < WIDTH*HEIGHT; i++)
				cur_buf[i] = 0;
			tween_start(prev_buf, cur_buf, TWEEN_FADE_TO_BLACK, SCROLL_MESSAGE_TWEEN_FRAMES);
			state = STATE_SCROLL_MESSAGE_UPDATE;
			break;
		
		case STATE_SCROLL_MESSAGE_UPDATE:
			// Proceed through the scrolling message
			if (!tween_running && millis() - last_time >= SCROLLING_MESSAGE_FRAME_MSEC) {
				if (text_next(cur_buf))
					tween_start(prev_buf, cur_buf, TWEEN_CUT, 1);
				else
					state = post_scrolling_message_state;
				
				last_time = millis();
			}
			break;
		
		case STATE_MARRIAGE_DURATION:
		case STATE_MARRIAGE_DURATION_UPDATE:
			{
				// If just entering this state, 
				if (state == STATE_MARRIAGE_DURATION) {
					// Re-wind time such that it appears our aniversary happened on the first
					// of the month to allow easy getting of the number of months from the
					// time library.
					time_t t = now() - (24ul*60ul*60ul*(unsigned long)(ANIVERSARY_DAY - 1));
					aniversary_years = year(t) - ANIVERSARY_YEAR;
					aniversary_months = month(t) - ANIVERSARY_MONTH;
					aniversary_days = day(t) - 1;
					aniversary_weeks = aniversary_days/7;
					aniversary_days -= aniversary_weeks*7;
					
					// Next time don't enter the duration starting state
					state = STATE_MARRIAGE_DURATION_UPDATE;
					
					// Cause the display to be redrawn immediately
					last_time = millis() - MARRIAGE_DURATION_PAUSE_MSEC;
					
					// First message always prefixed
					strcpy(str, "cubethan for ");
				} else {
					// Work out whether to prefix the current unit
					int units_left = (aniversary_years >= 1)
					               + (aniversary_months >= 1)
					               + (aniversary_weeks >= 1)
					               + (aniversary_days >= 1)
					               ;
					if (units_left == 1)
						strcpy(str, "and ");
					else
						strcpy(str, "");
				}
				
				// Show next unit after a pause
				if (millis() - last_time >= MARRIAGE_DURATION_PAUSE_MSEC) {
					if (aniversary_years > 19) {
						words_append_number(str, aniversary_years / 10);
						words_append(str, " decades");
						aniversary_years %= 10;
						state = STATE_MARRIAGE_DURATION_UPDATE;
					} else if (aniversary_years >= 1) {
						words_append_number(str, aniversary_years);
						words_append(str, " year");
						if (aniversary_years > 1) words_append(str, "s");
						aniversary_years = 0;
						state = STATE_MARRIAGE_DURATION_UPDATE;
					} else if (aniversary_months >= 1) {
						words_append_number(str, aniversary_months);
						words_append(str, " month");
						if (aniversary_months > 1) words_append(str, "s");
						aniversary_months = 0;
						state = STATE_MARRIAGE_DURATION_UPDATE;
					} else if (aniversary_weeks >= 1) {
						words_append_number(str, aniversary_weeks);
						words_append(str, " week");
						if (aniversary_weeks > 1) words_append(str, "s");
						aniversary_weeks = 0;
						state = STATE_MARRIAGE_DURATION_UPDATE;
					} else if (aniversary_days >= 1) {
						words_append_number(str, aniversary_days);
						words_append(str, " day");
						if (aniversary_days > 1) words_append(str, "s");
						aniversary_days = 0;
						state = STATE_MARRIAGE_DURATION_UPDATE;
					} else {
						// If nothing else to show, return to the clock
						state = STATE_CLOCK;
					}
					
					// Display the units if any remained.
					if (state == STATE_MARRIAGE_DURATION_UPDATE) {
						flip();
						words_set_mask(cur_buf, str);
						tween_start(prev_buf, cur_buf, TWEEN_FADE_THROUGH_BLACK, MARRIAGE_DURATION_TWEEN_FRAMES);
						last_time = millis();
					}
				}
			}
			break;
		
		case STATE_MOTD:
			// Schedule the next message of the day...
			if (day() == NEW_YEAR_EVE_DAY && month() == NEW_YEAR_EVE_MONTH) {
				// Make sure it happens just after midnight on new-year's eve...
				motd_hour = 0;
				motd_minute = 0;
			} else {
				// Otherwise, pick randomly in the next hour
				motd_hour = (motd_hour+1) % 24;
				motd_minute = random(0,60);
			}
			
			// Enter an appropriate state for the day
			if (day() == ANIVERSARY_DAY && month() == ANIVERSARY_MONTH) {
				// Aniversary today!
				int years = year() - ANIVERSARY_YEAR;
				sprintf(str
				       , "Happy %d%s Anniversary!"
				       , years
				       ,   ((years%10 == 1) && (years/10 != 1)) ? "st"
				         : ((years%10 == 2) && (years/10 != 1)) ? "nd"
				         : ((years%10 == 3) && (years/10 != 1)) ? "rd"
				         : "th"
				       );
				text_start(str);
				post_scrolling_message_state = STATE_MARRIAGE_DURATION;
				state = STATE_SCROLL_MESSAGE;
			} else if (day() == MET_DAY && month() == MET_MONTH) {
				// We met today!
				int years = year() - MET_YEAR;
				sprintf(str, "It's %d years since we met!", years);
				text_start(str);
				post_scrolling_message_state = STATE_MARRIAGE_DURATION;
				state = STATE_SCROLL_MESSAGE;
			} else if (day() == CUBE_BIRTHDAY_DAY && month() == CUBE_BIRTHDAY_MONTH) {
				// Cube's birthday
				int age = year() - CUBE_BIRTHDAY_YEAR;
				sprintf(str
				       , "Happy %d%s Birthday, Cube!"
				       , age
				       ,   ((age%10 == 1) && (age/10 != 1)) ? "st"
				         : ((age%10 == 2) && (age/10 != 1)) ? "nd"
				         : ((age%10 == 3) && (age/10 != 1)) ? "rd"
				         : "th"
				       );
				text_start(str);
				post_scrolling_message_state = STATE_MARRIAGE_DURATION;
				state = STATE_SCROLL_MESSAGE;
			} else if (day() == THAN_BIRTHDAY_DAY && month() == THAN_BIRTHDAY_MONTH) {
				// Than's birthday
				int age = year() - THAN_BIRTHDAY_YEAR;
				sprintf(str
				       , "Happy %d%s Birthday, 'than!"
				       , age
				       ,   ((age%10 == 1) && (age/10 != 1)) ? "st"
				         : ((age%10 == 2) && (age/10 != 1)) ? "nd"
				         : ((age%10 == 3) && (age/10 != 1)) ? "rd"
				         : "th"
				       );
				text_start(str);
				post_scrolling_message_state = STATE_MARRIAGE_DURATION;
				state = STATE_SCROLL_MESSAGE;
			} else if (day() == CHRISTMAS_DAY && month() == CHRISTMAS_MONTH) {
				// Jesus's birthday
				strcpy(str, "Merry Christmas!");
				text_start(str);
				post_scrolling_message_state = STATE_MARRIAGE_DURATION;
				state = STATE_SCROLL_MESSAGE;
			} else if (day() == NEW_YEAR_DAY && month() == NEW_YEAR_MONTH) {
				// New year's day
				sprintf(str, "Happy %d!", year());
				text_start(str);
				post_scrolling_message_state = STATE_MARRIAGE_DURATION;
				state = STATE_SCROLL_MESSAGE;
			} else {
				// Just another day with my cubewife! Just say how long we've been
				// together.
				state = STATE_MARRIAGE_DURATION;
			}
			break;
		
		case STATE_UNICOM:
			Serial.println(F("INFO: Unicom got lock!"));
			flip();
			face(cur_buf, 0, false);
			last_time = millis();
			tween_start(prev_buf, cur_buf, TWEEN_FADE_THROUGH_BLACK, UNICOM_START_TWEEN_FRAMES);
			state = STATE_UNICOM_LOCKED;
			break;
		
		case STATE_UNICOM_LOCKED:
			if (!unicom_updating) {
				Serial.println(F("ERROR: Unicom lost lock before transfer."));
				state = STATE_UNICOM_ERROR;
				last_time = millis();
			} else if (unicom_bits_arrived > 0) {
				Serial.println(F("INFO: Unicom data stream started."));
				state = STATE_UNICOM_TRANSFERRING;
			} else {
				// Blink the little face!
				if (!tween_running && millis() - last_time >= UNICOM_LOCKED_BLINK_PHASE_MSEC) {
					last_time = millis();
					face(cur_buf, 0, (animation_frame++)&1);
					tween_start(prev_buf, cur_buf, TWEEN_CUT, 1);
				}
			}
			break;
		
		case STATE_UNICOM_TRANSFERRING:
			if (unicom_updating) {
				if (unicom_bits_arrived <= unicom_num_bits) {
					face(cur_buf, (unicom_bits_arrived*14) / unicom_num_bits, false);
					tween_start(prev_buf, cur_buf, TWEEN_CUT, 1);
				}
			} else {
				if (unicom_bits_arrived >= unicom_num_bits) {
					Serial.println(F("INFO: Unicom data stream arrived successfuly."));
					state = STATE_UNICOM_OK;
				} else {
					Serial.print(F("ERROR: Unicom data stream failed after "));
					Serial.print(unicom_bits_arrived);
					Serial.println(F(" bits."));
					state = STATE_UNICOM_ERROR;
				}
				
				last_time = millis();
			}
			break;
		
		
		case STATE_UNICOM_OK:
			if (millis() - last_time >= UNICOM_FINAL_TIMEOUT_MSEC)
				state = STATE_CLOCK;
			break;
		
		
		case STATE_UNICOM_ERROR:
			if (unicom_bits_arrived >= -4) {
				if (millis() - last_time >= UNICOM_ERROR_FRAME_MSEC) {
					if (unicom_bits_arrived > 15)
						unicom_bits_arrived = 15;
					face(cur_buf, unicom_bits_arrived, false);
					tween_start(prev_buf, cur_buf, TWEEN_CUT, 1);
					unicom_bits_arrived--;
					last_time = millis();
				}
			} else if (millis() - last_time >= UNICOM_FINAL_TIMEOUT_MSEC) {
				state = STATE_CLOCK;
			}
			break;
		
		
		case STATE_SHAKE:
			Serial.println(F("INFO: Device shaken!"));
			
			switch (random(0, 3)) {
				default:
				case 0: state = STATE_MOTD;      break;
				case 1: state = STATE_LOVE_NOTE; break;
				case 2: state = STATE_AUTOMATA;  break;
			}
			
			if (unicom_updating)
				state = STATE_UNICOM;
			break;
		
		
		case STATE_LOVE_NOTE:
		case STATE_LOVE_NOTE_UPDATE:
			if (state == STATE_LOVE_NOTE) {
				state = STATE_LOVE_NOTE_UPDATE;
				animation_frame = 0;
				last_time = millis() - I_LOVE_CUBE_FRAME_MSEC;
			}
			
			if (millis() - last_time >= I_LOVE_CUBE_FRAME_MSEC) {
				if (animation_frame < 3) {
					flip();
					switch (animation_frame) {
						case 0: words_set_mask(cur_buf, "than"); break;
						case 1: words_set_mask(cur_buf, "*");    break;
						case 2: words_set_mask(cur_buf, "cube"); break;
					}
					tween_start(prev_buf, cur_buf, TWEEN_FADE_THROUGH_BLACK, I_LOVE_CUBE_TWEEN_FRAMES);
					last_time = millis();
					animation_frame++;
				} else {
					state = STATE_CLOCK;
				}
			}
			if (unicom_updating)
				state = STATE_UNICOM;
			break;
		
		case STATE_AUTOMATA:
		case STATE_AUTOMATA_UPDATE:
			if (state == STATE_AUTOMATA) {
				state = STATE_AUTOMATA_UPDATE;
				animation_frame = 0;
				last_time = millis() - AUTOMATA_FRAME_MSEC;
			}
			
			if ((animation_frame == 1 && millis() - last_time >= AUTOMATA_FIRST_FRAME_MSEC)
			    || (animation_frame != 1 && millis() - last_time >= AUTOMATA_FRAME_MSEC)) {
				if (animation_frame < AUTOMATA_NUM_FRAMES) {
					flip();
					if (animation_frame == 0)
						words_set_mask(cur_buf, "*");
					else
						automata_xor(cur_buf, prev_buf);
					tween_start(prev_buf, cur_buf, TWEEN_FADE, AUTOMATA_TWEEN_FRAMES);
					last_time = millis();
				} else {
					state = STATE_CLOCK;
				}
				animation_frame++;
			}
			if (unicom_updating)
				state = STATE_UNICOM;
			break;
		
		default:
			state = STATE_RESET;
			Serial.print(F("Entered unknown state "));
			Serial.println(state);
			break;
	}
	
	Serial.println(shaking);
	
	// Update the display
	int intensity;
	const char *buf;
	tween_running = tween_next(&buf, &intensity);
	if (tween_running) {
		display_buf(buf, intensity);
	}
	
}
