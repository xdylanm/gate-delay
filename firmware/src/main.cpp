// ATTiny13a Pulse Delay Firmware
// Implements: Falling edge detect on PB3, analog delay via ADC1, output pulse on PB0

#include <avr/io.h>
#include <avr/interrupt.h>

#include "adc_to_delay.h"

// Pin definitions
#define INPUT_PIN   PB3
#define OUTPUT_PIN  PB0
#define ANALOG_CH   1 // ADC1

// Timing variables, related to current pin state
volatile uint16_t input_pulse_start = 0;
volatile uint16_t pulse_width = 0;
// 0=wait leading edge, 1=received leading edge, 2=wait trailing edge, 3=processing delayed pulse
volatile uint8_t delay_in_state = 0; 
volatile uint8_t last_pin_state;

// Time tracking (ticks)
volatile uint16_t tick_counter;
volatile uint8_t ticks_pending;
uint16_t tick_current;

// Delayed pulse output
#define PULSE_FIFO_DEPTH 4
#define ACTIVE_PULSE 0x01
#define READY_LEADING_EDGE 0x02
#define READY_TRAILING_EDGE 0x04

struct Pulse 
{
    uint16_t start;
    uint16_t stop;
    uint8_t state;
};
Pulse pulse_fifo[PULSE_FIFO_DEPTH];
uint8_t pulse_head;
uint8_t pulse_tail;

#ifdef EN_DEBUG_PIN
#define DBG_LED_PIN PB1
inline void delay_x10ms(uint8_t const x) 
{
    for (uint8_t n = 0; n < x; ++n) {
        _delay_loop_2(24000); 
    }
}

void debug_blink(uint8_t const nblink, bool const forever=true) 
{
    cli();  // disable global interrupts

    do {
        for (uint8_t i = 0; i < nblink; ++i) {
            PORTB |= (1 << DBG_LED_PIN);    // on
            delay_x10ms(25);
            PORTB &= ~(1 << DBG_LED_PIN);   // off
            delay_x10ms(25);
        }
        delay_x10ms(100);
    } while (forever);

    sei();
}
#endif

uint16_t read_adc(uint8_t const ch) 
{
    ADMUX = (ADMUX & 0xF0) | ch;
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

void setup() 
{
    // set the main clock to 9.6MHz (no prescale)
    CLKPR = (1 << CLKPCE);  // pre scale change enable
    CLKPR = 0;              // within 4 clocks, 0 = no prescale

    // Set OUTPUT_PIN as output, INPUT_PIN as input
    DDRB |= (1 << OUTPUT_PIN);
    DDRB &= ~(1 << INPUT_PIN);
    PORTB |= (1 << INPUT_PIN); // Pull-up default enabled (MCUCR.PUD=0)
    
    // Set up ADC: Vcc ref, select ADC1, left adjust, prescaler 64
    ADMUX = (1 << MUX0); // ADC1
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) ; // Enable, prescaler 128

    // trigger a conversion to initialize the ADC once
    read_adc(ANALOG_CH);

    // Set up Pin Change Interrupt on PB3
    GIMSK |= (1 << PCIE);
    PCMSK |= (1 << INPUT_PIN);

    delay_in_state = 0;
    last_pin_state = 1;     // active low

    pulse_head = 0;
    pulse_tail = 0;
    for (int8_t i = 0; i < PULSE_FIFO_DEPTH; ++i) {
        pulse_fifo[i].state = 0;
    }

#ifdef EN_DEBUG_PIN
    // Set DBG_LED_PIN as output
    DDRB |= (1 << DBG_LED_PIN);
    debug_blink(3,false);
#endif

    // Set up Timer0: normal mode, prescaler 8
    TCCR0B = (1 << CS01);   // 0b010 clk/8, start
    TCNT0 = 0;              // reset
    TIMSK0 |= (1 << TOIE0); // Enable overflow interrupt

    // set this just before entering the main loop to manage clock jitter with the stack
    tick_counter = 0;
    tick_current = 0;   // tick_counter
    ticks_pending = 0;  // tick_counter - tick_current
        
    sei(); // Enable global interrupts

}

// Pin change interrupt for gate trigger
ISR(PCINT0_vect) 
{
    uint8_t const cur_pin_state = (PINB & (1 << INPUT_PIN)) ? 1 : 0;
    bool const leading_edge = last_pin_state == 1 && cur_pin_state == 0;
    bool const trailing_edge = last_pin_state == 0 && cur_pin_state == 1;

    if (leading_edge && delay_in_state == 0) {
        input_pulse_start = tick_counter;
        delay_in_state = 1;
    } else if (trailing_edge && delay_in_state == 2) {
        uint16_t const input_pulse_end = tick_counter;
        if (input_pulse_end >= input_pulse_start) {
            pulse_width = input_pulse_end - input_pulse_start;
        } else {
            pulse_width = (UINT16_MAX - input_pulse_end) + input_pulse_start + 1;
        }
        delay_in_state = 3;
    }
    last_pin_state = cur_pin_state;
}

// Overflow for TCNT0: T_tick = Prescaler * 256 / F_CPU 
// * prescaler = 1, T_tick = 26us, overflow = 1.7s
// * prescaler = 8, T_tick = 213us, overflow = 14s
ISR(TIM0_OVF_vect) 
{
    tick_counter += 1;   
    ticks_pending +=1 ;
}

void loop() 
{
#ifdef EN_DEBUG_PIN
    if (ticks_pending > 8) {
        debug_blink(2);
    }
#endif

    Pulse* head = &(pulse_fifo[pulse_head]);
    if (delay_in_state == 1) {
    
        // Read analog voltage - expected less than 2 ticks of TCNT0
        uint16_t const adc_val = read_adc(ANALOG_CH);
        uint16_t const delay_ticks = adc_to_delay(adc_val);

        // uint16_t const delay_ticks = read_adc(ANALOG_CH) << 3; 

        // uint16_t const delay_ticks = 4695; // 14085; 
        
        if (head->state == 0) {
            // Important: this copy has to be atomic. On an 8-bit processor, it can get interrupted
            // halfway, resulting in a corrupted copy (e.g. [0000][1111] --> [0001][1111])
            cli();
            head->start = input_pulse_start;
            sei();
            head->start += delay_ticks;
            head->state = (READY_LEADING_EDGE | ACTIVE_PULSE);
    
            delay_in_state = 2; // advance to "wait for trailing edge" state 
        } else {
            delay_in_state = 0; // discard this pulse and wait again
        }
    } else if (delay_in_state == 3) {   // got the trailing edge
        cli();
        head->stop = pulse_width;
        sei();
        head->stop += head->start;
        head->state |= READY_TRAILING_EDGE;

        pulse_head = (pulse_head + 1) % PULSE_FIFO_DEPTH;
        delay_in_state = 0; // advance to "wait for leading edge" state
    } 
    
    Pulse* tail = &(pulse_fifo[pulse_tail]);
    if ((tail->state & READY_LEADING_EDGE) && (tick_current == tail->start)) {
        PORTB |= (1 << OUTPUT_PIN);
        tail->state &= ~(READY_LEADING_EDGE);
    } 
    
    if ((tail->state & READY_TRAILING_EDGE) && (tick_current == tail->stop)) {
        PORTB &= ~(1 << OUTPUT_PIN);
        tail->state = 0;    // remove active & ready trailing
        pulse_tail = (pulse_tail + 1) % PULSE_FIFO_DEPTH;   // consumed
    }

    // checked for edges, pop the tick stack
    if (ticks_pending > 0) {
        tick_current++;
        ticks_pending--;
    }
}

int main(void) 
{
    setup();

    while (1) {
        loop();
    }
    return 0;
}