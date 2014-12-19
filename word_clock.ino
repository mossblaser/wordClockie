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

// Indicates if the last attempt to synchronise the time using unicom succeeded
bool unicom_succeeded;

// The number of bits received so far (if receiving)
size_t unicom_bits_arrived;

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
	static unsigned long cur_time;
	static unsigned long receive_started;
	
	UnicomReceiver::state_t state = unicom.getState();
	switch (state) {
		default:
		case UnicomReceiver::STATE_SYNCING:
			// No valid data coming in
			bytes_received = 0;
			cur_time = 0;
			unicom_updating = false;
			break;
		
		case UnicomReceiver::STATE_LOCKED:
			// We've got a clock signal, data will arrive shortly
			unicom_updating = true;
			unicom_bits_arrived = 0;
			unicom_succeeded = false;
			receive_started = millis();
			break;
		
		case UnicomReceiver::STATE_RECEIVING:
			char c;
			if (unicom.getByte(&c)) {
				// Accumulate first word of data (the time)
				if (bytes_received < 4) {
					cur_time |= ((unsigned long)c & 0xFFul) << (bytes_received*8);
				}
				bytes_received++;
				
				// Once the time has arrived, update the RTC
				if (bytes_received == 4) {
					// Compensate for time spent transmitting
					cur_time += (millis() - receive_started) / 1000;
					RTC.set(cur_time);
					setTime(cur_time);
					Serial.println("INFO: Successfuly updated clock using unicom.");
					unicom_succeeded = true;
				}
			}
			
			// Update the current bit count
			unicom_bits_arrived = (bytes_received*8) + unicom.getBitsReceived();
			break;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Setup code
////////////////////////////////////////////////////////////////////////////////

void setup() {
	// Setup debug
	Serial.begin(115200);
	Serial.println("INFO: Word Clock Firmware Running...");
	
	// Setup display drivers
	display_begin();
	
	// Setup realtime clock
	setSyncProvider(RTC.get);
	if(timeStatus()!= timeSet) 
		Serial.println("ERROR: Unable get time from RTC.");
	else
		Serial.println("INFO: Successfuly read time from RTC");
	
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
	STATE_UNICOM_LOCKED,
	STATE_UNICOM_TRANSFERRING,
	STATE_UNICOM_OK,
	STATE_UNICOM_ERROR,
} state_t;



void loop() {
	static state_t state = STATE_RESET;
	
	unicom_loop();
	
	// The time last displayed
	static int last_minute;
	static int last_hour;
	
	// Is a tween currently running?
	static bool tween_running;
	
	// For the implementation of timers
	static unsigned long last_time = 0;
	
	// State to enter after displaying a scrolling message
	static state_t post_scrolling_message_state;
	
	// Marriage duration
	static int aniversary_years;
	static int aniversary_months;
	static int aniversary_weeks;
	static int aniversary_days;
	
	// The time at which the message of the day should next be displayed
	static int motd_minute;
	static int motd_hour;
	
	static char str[100];
	
	switch (state) {
		case STATE_RESET:
			// On power up, display a friendly message
			Serial.println("INFO: UI state machine reset");
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
		
		case STATE_UNICOM_LOCKED:
			if (!unicom_updating) {
				state = STATE_UNICOM_ERROR;
			}
		
		default:
			state = STATE_RESET;
			Serial.print("Entered unknown state ");
			Serial.println(state);
			break;
	}
	
	// Update the display
	int intensity;
	const char *buf;
	tween_running = tween_next(&buf, &intensity);
	if (tween_running) {
		display_buf(buf, intensity);
	}
	
}
