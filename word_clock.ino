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
}


////////////////////////////////////////////////////////////////////////////////
// Main UI state-machine
////////////////////////////////////////////////////////////////////////////////


typedef enum {
	STATE_RESET,
	STATE_RESET_MESSAGE,
	STATE_CLOCK,
	STATE_CLOCK_UPDATE,
} state_t;



void loop() {
	static state_t state = STATE_RESET;
	
	unicom_loop();
	
	// The time last displayed
	static int last_minute;
	static int last_hour;
	
	// For the implementation of timers
	static unsigned long last_time = 0;
	
	switch (state) {
		case STATE_RESET:
			// On power up, display a friendly message
			Serial.println("INFO: UI state machine reset");
			flip();
			words_set_mask(cur_buf, "for cube");
			tween_start(prev_buf, cur_buf, TWEEN_FADE_FROM_BLACK, RESET_MESSAGE_TWEEN_FRAMES);
			state = STATE_RESET_MESSAGE;
			last_time = millis();
			break;
		
		case STATE_RESET_MESSAGE:
			// Let the message linger for some time after reset before showing the
			// clock
			if ((millis() - last_time) >= RESET_MESSAGE_TIMEOUT_MSEC) {
				state = STATE_CLOCK;
			}
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
					char str[100];
					strcpy(str, "its ");
					words_append_time(str, hour(t), minute(t));
					
					// Start animating a transition
					flip();
					words_set_mask(cur_buf, str);
					if (state == STATE_CLOCK)
						tween_start(prev_buf, cur_buf, TWEEN_FADE_THROUGH_BLACK, CLOCK_TWEEN_FRAMES);
					else
						tween_start(prev_buf, cur_buf, TWEEN_FADE, CLOCK_UPDATE_TWEEN_FRAMES);
				}
				
				state = STATE_CLOCK_UPDATE;
			}
			break;
		
		default:
			state = STATE_RESET;
			Serial.print("Entered unknown state ");
			Serial.println(state);
			break;
	}
	
	// Update the display
	int intensity;
	const char *buf;
	if (tween_next(&buf, &intensity))
		display_buf(buf, intensity);
	
}
