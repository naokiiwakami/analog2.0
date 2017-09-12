/* MInI Board II controller (ATTiny2313) */

#include <avr/io.h>
#include <avr/interrupt.h>

/*---------------------------------------------------------*/
/**
 * Port Assignment
 */
#define CV_PITCH   OCR0A
#define CV_BEND    OCR1A

#define GATE1		PORTB1	

#define GATE1_ON(x)		(PORTB |= (1 << GATE1))
#define GATE1_OFF(x)	(PORTB &= ~(1 << GATE1))

/*---------------------------------------------------------*/
#define true  1
#define false 0


#ifndef F_CPU
#define F_CPU    20000000	/*  20MHz processor */
#endif

/* set baud rate here */		// for MIDI
#define UART_BAUD_RATE 31250
#define UART_BUF_SIZE    16

/* automatically calcuate baud register value */
#define UART_BAUD_SELECT (F_CPU/(UART_BAUD_RATE*16l)-1)

volatile uint8_t uart_rxd_buf_cnt;
uint8_t *uart_rxd_in_ptr, *uart_rxd_out_ptr;
uint8_t uart_rxd_buffer[UART_BUF_SIZE];

volatile uint16_t rxByte;

/*---------------------------------------------------------*/
/**
 * MIDI Parser / Message Handler Definitions
 */

/* MIDI Channel Voice message */
#define MSG_NOTE_OFF          0x80
#define MSG_NOTE_ON           0x90
#define MSG_POLY_KEY_PRESSURE 0xA0
#define MSG_CONTROL_CHANGE    0xB0
#define MSG_PROGRAM_CHANGE    0xC0
#define MSG_CHANNEL_PRESSURE  0xD0
#define MSG_PITCH_BEND        0xE0

/* MIDI Control Changes */
#define CC_BREATH                0x02
#define CC_EXPRESSION            0x0B
#define CC_DAMPER_PEDAL          0x40

/* MIDI Channel Mode Messages */
#define CC_ALL_SOUND_OFF         0x78
#define CC_RESET_ALL_CONTROLLERS 0x79
#define CC_LOCAL_CONTROL         0x7A
#define CC_ALL_NOTES_OFF         0x7B
#define CC_OMNI_MODE_OFF         0x7C
#define CC_OMNI_MODE_ON          0x7D
#define CC_MONO_MODE_ON          0x7E
#define CC_POLY_MODE_ON          0x7F

/* MIDI System Messages */
#define SYSEX_OUT 0xF7
#define SYSEX_IN  0xF0

#define MAX_DATA 0x7F

/* Notes */
#define C0 0x0B

/* MIDI parser buffer related */
#define isSystemExclusive(status) ((status) == SYSEX_IN)
#define isChannelStatus(status)   ((status) < SYSEX_IN)
#define getMessageFromStatus(status)  ((status) & 0xF0)
#define getChannelFromStatus(status)  ((status) & 0x0F)

volatile uint8_t midi_status;      // MIDI status
volatile uint8_t midi_message;     // MIDI message (= status & 0xF0)
volatile uint8_t midi_channel;     // MIDI channel (= status & 0x0F)

volatile uint8_t midi_channel_target; // Target MIDI channel of the controller

volatile uint8_t midi_data[2];     // MIDI data buffer
volatile uint8_t midi_data_ptr;    // MIDI data buffer pointer
volatile uint8_t midi_data_length; // Expected MIDI data length
volatile uint8_t midi_data_ready;  // data ready flag

/*---------------------------------------------------------*/
/*
 * variables for the controllers
 */
#define MAX_NOTE_COUNT 8

// voice controller values
volatile uint8_t  voice_CurrentNote;
volatile uint8_t  voices_NotesCount;
volatile uint8_t  voices_Notes[MAX_NOTE_COUNT];

// other controller values
// Note that these values are not always equivalent to actual
// CV values since we aply anti-click mechanism in reflecting these
// values to CV to avoid transient noise.
volatile uint16_t ctrl_PitchBend; 
volatile uint8_t ctrl_PitchBend_updating;


/*
 * variables for keyboard reader
 */
volatile uint8_t key_prev;
volatile uint8_t tone_factor;


/**
 * init_uart()
 * Initializes the UART interface
 */
void init_uart(void)
{

  uint16_t baud = UART_BAUD_SELECT;
  UBRRH = (uint8_t)(baud >> 8);
  UBRRL = (uint8_t)baud;
  UCSRB = (1<<RXCIE)|(1<<RXEN);

  uart_rxd_in_ptr  = uart_rxd_out_ptr = uart_rxd_buffer;
  uart_rxd_buf_cnt = 0;
}

void init_timer(void)
{
    // Timer0 configuration
    TCCR0A = (1<<COM0A1) /*|(1<<COM0A0)*/|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM01)|(1<<WGM00);
    TCCR0B = (1<<CS00);
    TIMSK = _BV(TOIE0);

    TCCR1A = (1<<COM1A1)|(1<<COM1A0)|(1<<COM1B1)|(1<<COM1B0)| _BV(WGM11) | _BV(WGM10);
    TCCR1B = (1<<WGM12) | _BV(CS10);

    TIMSK |= _BV(TOIE1);

  /**
   * Time1 configuration
   *
   * Waveform generation : Fast PWM, 10bit
   * Clock               : No prescaling
   * OC1A initialization : Reset
   * OC1B initialization : Reset
   */
    TCNT1 = 0x0000;
	/*
    TCCR1A = 0xA3;
    TCCR1B = 0x9;
	*/
	// ICR1 = 0xFFF;
    CV_PITCH   = 0xFF;	// pitch CV 
}

/**
 * ISR()
 * signal handler for receive complete interrupt (RCI)
 */
ISR(USART_RX_vect)
{

  cli();
  *uart_rxd_in_ptr = UDR;	/* read byte from receive register */
  uart_rxd_buf_cnt++;
  if (++uart_rxd_in_ptr >= uart_rxd_buffer + UART_BUF_SIZE) /* Pointer wrapping */
    uart_rxd_in_ptr = uart_rxd_buffer;
  sei();
}

/**
 * uart_getchar()
 * reads UART read buffer and returns a single character.
 */
int16_t uart_getchar(void)
{
  uint8_t c;

  if (uart_rxd_buf_cnt>0) {
    cli();
    uart_rxd_buf_cnt--;
    c = *uart_rxd_out_ptr;				/* get character from buffer */
    if (++uart_rxd_out_ptr >= uart_rxd_buffer + UART_BUF_SIZE) /* pointer wrapping */
      uart_rxd_out_ptr = uart_rxd_buffer;
    sei();
    return c;
  } else {
    return -1;							/* buffer is empty */
  }
}

void init_midi_controllers()
{
  ctrl_PitchBend = 8192 >> 4; // 0x20 0x00
  ctrl_PitchBend_updating = 0;
  voices_NotesCount = 0;

  // Set target MIDI channel.  If the button is on, read the channel from the octave switch.
  // Otherwise the channel is 1 (is zero internally).
  midi_channel_target = (PIND & _BV(PD2)) ? 0 : ((PIND >> 3) ^ 0x0f);

  GATE1_OFF();
}

/**
 * iosetup()
 * Initializes I/O ports.
 */
void iosetup(void)
{
  /* setup output data port */
  DDRB = _BV(PB0) |  // tuning tone
         _BV(PB1) |  // gate out
         _BV(PB2) |  // note CV
         _BV(PB3);   // pitch bend

    PORTB = 0xf0; // key reader


    PORTD = _BV(PD2) | _BV(PD3) | _BV(PD4) | _BV(PD5) | _BV(PD6);
}


//-----------------------------------------------
// Update PWM width for Pitch CV

void SetCV()
{
    uint8_t pwm_value;
    pwm_value = voice_CurrentNote > C0 ? voice_CurrentNote - C0 : 1;
    CV_PITCH = pwm_value * 2;
}

/**
 * Pitch Bend Channel Message Handler.
 * Update the channel_PitchBend value.
 * This value is referred by timer1 interrupt handler that updates pitch bend CV.
 */
void pitch_bend(uint8_t lsb, uint8_t msb)
{
  uint16_t temp = 1023 - (lsb / 16 + msb * 8);
  ctrl_PitchBend_updating = 1;
  ctrl_PitchBend = temp * 5 / 32;
  ctrl_PitchBend_updating = 0;
}

/**
 * Note Off Channel Message Handler.
 * This algorithm is based on the policy of "high note high priority"
 */
void note_off(uint8_t note_number)
{                                                                                          	uint8_t i;
  uint8_t flg = false;

  // Find 
  for(i = 0; i < voices_NotesCount; i++) {
	if (flg) {
	  voices_Notes[i-1] = voices_Notes[i];
	}
	if (voices_Notes[i] == note_number) {
	  flg = true;
	}
  }

  if (flg) {
	--voices_NotesCount;
  }
  else {
	// This is not a voice we are tracking
	return;
  }

  // Update states of the instruments
  //
  if(voices_NotesCount == 0) {
	// There are no voices on the table.  It's time to switch the gate off
	GATE1_OFF();
  }
  else {
	// Update the CV to current highest.  Keep the gate on.
	voice_CurrentNote = voices_Notes[0];
	SetCV();
  }
}

void all_notes_off()
{
  // clear voices table and switch the gate off
  voices_NotesCount = 0;
  GATE1_OFF();
}

/**
 * Note On Channel Message Handler.
 * This algorithm is based on the policy of "high note high priority"
 */
void note_on(uint8_t note_number, uint8_t velocity)
{
  uint8_t i;

  // note on with velocity zero is equivalent to note off
  if (velocity == 0) {
	note_off(note_number);
    return;
  }

  // Finish the work quickly if the voice table is empty
  if (voices_NotesCount == 0) {
	++voices_NotesCount;
	voices_Notes[0] = note_number;
	voice_CurrentNote = note_number;
	SetCV();
	GATE1_ON();
  }

  // Update the current voice table.
  uint8_t pos = voices_NotesCount;
  for(i = 0; i < voices_NotesCount; i++) {
	// If we already have the note we do nothing.
	if(voices_Notes[i] == note_number) {
	  return;
	}

	// Search for the insertion point
	if (note_number > voices_Notes[i]) {
	  pos = i;
	  break;
	}
  }

  // Insert the new note
  if (pos < MAX_NOTE_COUNT) {
	if (++voices_NotesCount > MAX_NOTE_COUNT) {
	  voices_NotesCount = MAX_NOTE_COUNT;
	}

	for (i=voices_NotesCount; --i>pos; ) {
	  voices_Notes[i] = voices_Notes[i-1];
	}
	voices_Notes[pos] = note_number;
  }
  
  voice_CurrentNote =  voices_Notes[0];

  SetCV();
  GATE1_ON();
}

void control_change(uint8_t control_number, uint8_t value)
{
  switch(control_number) {
  case CC_BREATH:
  case CC_EXPRESSION:
	// set_volume(midi_data[1]);
	break;
  case CC_RESET_ALL_CONTROLLERS:
	init_midi_controllers();
	break;
  case CC_LOCAL_CONTROL:
	// TODO: We should enable/disable on-board keyboard when we get this
	break;
  case CC_ALL_SOUND_OFF:
  case CC_ALL_NOTES_OFF:
	// all_notes_off();
	break;
  }
}

/**
 * Handle MIDI Channel Messages
 */
void handle_midi_channel_message()
{
  // do nothing for channels that are out of scope
  if (midi_channel != midi_channel_target) {
	return;
  }

  switch(midi_message) {
  case MSG_NOTE_OFF:
	note_off(midi_data[0]);
	break;
  case MSG_NOTE_ON:
   	note_on(midi_data[0], midi_data[1]);
	break;
  case MSG_CONTROL_CHANGE:
	control_change(midi_data[0], midi_data[1]);
	break;
  case MSG_PITCH_BEND:
	pitch_bend(midi_data[0], midi_data[1]);
	break;
  default:
	// do nothing for unsupported channel messages
	break;
  }
}

void init_midi_parser()
{
  midi_status = 0;
  midi_message = 0;
  midi_channel = 0xFF;
  midi_data_ptr= 0;
  midi_data_length= 1;
  midi_data_ready = 0;
}

void parse_and_handle()
{
  if (isSystemExclusive(midi_status)) {
	if (rxByte == SYSEX_OUT) {
	  midi_status = SYSEX_OUT;
	}
	return; // ignore any sysex data
  }

  if (rxByte > SYSEX_IN) {
	midi_status = rxByte;
	midi_data_length = 1; // We don't really parse system messages but toss every data
	midi_data_ptr = 0;
	return;
  }

  if (rxByte == SYSEX_IN) {
	midi_status= SYSEX_IN;
	return;
  }

  if (rxByte > MAX_DATA) {
	midi_status = rxByte;
    midi_channel = getChannelFromStatus(midi_status);
    midi_message = getMessageFromStatus(midi_status);

    // check the message and determine the
	switch(midi_message) {
	case MSG_NOTE_OFF:
	case MSG_NOTE_ON:
	case MSG_POLY_KEY_PRESSURE:
	case MSG_CONTROL_CHANGE:
	case MSG_PITCH_BEND:
	  midi_data_length = 2;
	  break;
	case MSG_PROGRAM_CHANGE:
	case MSG_CHANNEL_PRESSURE:
	  midi_data_length = 1;
	  break;
	}
	midi_data_ptr = 0;
	return;
  }

  // OK we are reading a data byte
  midi_data[midi_data_ptr++] = rxByte;
  if (midi_data_ptr ==  midi_data_length) {
	midi_data_ptr = 0;

	// We handle the data
	if ( isChannelStatus(midi_status) ) {
	  handle_midi_channel_message();
	}
  }
}

#define BUZZ_CYCLE (F_CPU / 0xff / 440 / 2)

ISR(TIMER0_OVF_vect)
{
    static uint8_t buzzcount = 0;
    
    if (++buzzcount == BUZZ_CYCLE) {
        buzzcount = 0;
        PORTB ^= tone_factor;
    }
}

ISR(TIMER1_OVF_vect)
{
    static uint16_t button_count = 0;
    uint8_t key;
    uint8_t octave;

    if (CV_BEND != ctrl_PitchBend && ! ctrl_PitchBend_updating) {
        if (CV_BEND > ctrl_PitchBend) {
            --CV_BEND;
        }
        else {
            ++CV_BEND;
        }
    }

    // check the keyboard
    //
    octave = ((PIND >> 3) ^ 0x0f) * 12;
    key = ((PINB >> 4) ^ 0x0f);
    if (key) {
        key += octave;
    }
    if (key != key_prev) {
        if (key_prev) {
            note_off(key_prev + C0);
        }
        if (key) {
            note_on(key + C0, 127);
        }
    }
    key_prev = key;

    if (! (PIND & _BV(PD2))) {
        ++button_count;
    }
    else {
        button_count = 0;
    }
    if (button_count >= 19531) {
        tone_factor ^= _BV(PORTB0);
        button_count = 0;
    }
}

int main(void)
{
  iosetup();
  init_uart();
  init_timer();
  
  init_midi_parser();
  init_midi_controllers();

  key_prev = 0;
  tone_factor = 0;

  sei();

  while(true){
    rxByte = uart_getchar();
    if (rxByte != -1) {
	  parse_and_handle();
    }
  }

}
