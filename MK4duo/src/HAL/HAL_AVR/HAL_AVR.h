/**
 * MK4duo Firmware for 3D Printer, Laser and CNC
 *
 * Based on Marlin, Sprinter and grbl
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 * Copyright (C) 2013 Alberto Cotronei @MagoKimbra
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * This is the main Hardware Abstraction Layer (HAL).
 * To make the firmware work with different processors and toolchains,
 * all hardware related code should be packed into the hal files.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Description:          *** HAL for Arduino ***
 *
 * Contributors:
 * Copyright (c) 2014 Bob Cousins bobcousins42@googlemail.com
 *                    Nico Tonnhofer wurstnase.reprap@gmail.com
 *
 * Copyright (c) 2015 - 2016 Alberto Cotronei @MagoKimbra
 *
 * ARDUINO_ARCH_AVR
 */

#ifndef _HAL_AVR_H_
#define _HAL_AVR_H_

// --------------------------------------------------------------------------
// Includes
// --------------------------------------------------------------------------
#include <math.h>
#include <stdint.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "HAL_math_AVR.h"

// --------------------------------------------------------------------------
// Types
// --------------------------------------------------------------------------
typedef uint16_t  hal_timer_t;
typedef uint16_t  ptr_int_t;

// --------------------------------------------------------------------------
// Includes
// --------------------------------------------------------------------------
#include "fastio.h"
#include "HAL_watchdog_AVR.h"

// Serial
//#define EXTERNALSERIAL  // Force using arduino serial
#ifndef EXTERNALSERIAL
  #include "HardwareSerial.h"
  #define MKSERIAL MKSerial
#else
  #define MKSERIAL Serial
#endif

// --------------------------------------------------------------------------
// Defines
// --------------------------------------------------------------------------

#ifndef CRITICAL_SECTION_START
  #define CRITICAL_SECTION_START  unsigned char _sreg = SREG; cli();
  #define CRITICAL_SECTION_END    SREG = _sreg;
#endif

#ifdef analogInputToDigitalPin
  #undef analogInputToDigitalPin
#endif

#define analogInputToDigitalPin(p) ((p) + 0xA0)

#define PACK

#if ENABLED(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#undef LOW
#define LOW         0
#undef HIGH
#define HIGH        1

// EEPROM START
#define EEPROM_OFFSET 100

// Voltage for Pin
#define HAL_VOLTAGE_PIN 5.0

#define HAL_TIMER_TYPE_MAX 0xFFFF

// Macros for stepper.cpp
#define HAL_MULTI_ACC(longIn1, longIn2) MultiU24X32toH16(longIn1, longIn2)

// TEMPERATURE
#define ANALOG_REF_AREF       0
#define ANALOG_REF_AVCC       _BV(REFS0)
#define ANALOG_REF            ANALOG_REF_AVCC
#define ANALOG_PRESCALER      _BV(ADPS0)|_BV(ADPS1)|_BV(ADPS2)
#define OVERSAMPLENR          16
#define ABS_ZERO              -273.15
#define AD_RANGE              1023

#define HARDWARE_PWM          false

#define GET_PIN_MAP_PIN(index) index
#define GET_PIN_MAP_INDEX(pin) pin
#define PARSED_PIN_INDEX(code, dval) parser.intval(code, dval)

// --------------------------------------------------------------------------
// Timer
// --------------------------------------------------------------------------

constexpr uint32_t  HAL_TIMER_RATE        = ((F_CPU) / 8);
constexpr float     HAL_ACCELERATION_RATE = (4096.0 * 4096.0 / (HAL_TIMER_RATE));

#define STEPPER_TIMER_PRESCALE      8
#define STEPPER_TIMER_TICKS_PER_US  (HAL_TIMER_RATE / 1000000)
#define STEPPER_TIMER_MIN_INTERVAL  8                                         // minimum time in µs between stepper interrupts
#define STEPPER_PULSE_CYCLES        ((MINIMUM_STEPPER_PULSE) * CYCLES_PER_US) // Stepper pulse duration, in cycles

#define TEMP_TIMER_FREQUENCY        ((F_CPU) / 64.0 / 64.0) // 3096 Hz

#define STEPPER_TIMER               1
#define STEPPER_TCCR                TCCR1A
#define STEPPER_TIMSK               TIMSK1
#define STEPPER_OCIE                OCIE1A

#define TEMP_TIMER                  0
#define TEMP_OCR                    OCR0B
#define TEMP_TCCR                   TCCR0A
#define TEMP_TIMSK                  TIMSK0
#define TEMP_OCIE                   OCIE0B

#define TIMER_OCR_0                 OCR0A
#define TIMER_OCR_1                 OCR1A
#define TIMER_COUNTER_0             TCNT0
#define TIMER_COUNTER_1             TCNT1

#define PULSE_TIMER_PRESCALE        STEPPER_TIMER_PRESCALE

#define HAL_STEPPER_TIMER_START()   HAL_stepper_timer_start()
#define HAL_TEMP_TIMER_START()      HAL_temp_timer_start()

#define ENABLE_STEPPER_INTERRUPT()  SBI(STEPPER_TIMSK, STEPPER_OCIE)
#define DISABLE_STEPPER_INTERRUPT() CBI(STEPPER_TIMSK, STEPPER_OCIE)
#define STEPPER_ISR_ENABLED()       TEST(STEPPER_TIMSK, STEPPER_OCIE)

#define ENABLE_TEMP_INTERRUPT()     SBI(TEMP_TIMSK, TEMP_OCIE)
#define DISABLE_TEMP_INTERRUPT()    CBI(TEMP_TIMSK, TEMP_OCIE)
#define TEMP_ISR_ENABLED()          TEST(TEMP_TIMSK, TEMP_OCIE)

#define _CAT(a, ...) a ## __VA_ARGS__
#define HAL_timer_set_count(timer, count)           (_CAT(TIMER_OCR_, timer) = count)
#define HAL_timer_get_count(timer)                  _CAT(TIMER_OCR_, timer)
#define HAL_timer_get_current_count(timer)          _CAT(TIMER_COUNTER_, timer)
#define HAL_timer_restricts(timer, interval_ticks)  NOLESS(_CAT(TIMER_OCR_, timer), _CAT(TIMER_COUNTER_, timer) + interval_ticks)

/**
 * On AVR there is no hardware prioritization and preemption of
 * interrupts, so this emulates it. The UART has first priority
 * (otherwise, characters will be lost due to UART overflow).
 * Then: Stepper, Endstops, Temperature, and -finally- all others.
 */
#define HAL_timer_isr_prologue(TIMER_NUM)
#define HAL_timer_isr_epilogue(TIMER_NUM)

/* 18 cycles maximum latency */
#define STEPPER_TIMER_ISR \
extern "C" void TIMER1_COMPA_vect (void) __attribute__ ((signal, naked, used, externally_visible)); \
extern "C" void TIMER1_COMPA_vect_bottom (void) asm ("TIMER1_COMPA_vect_bottom") __attribute__ ((used, externally_visible)); \
void TIMER1_COMPA_vect (void) { \
  __asm__ __volatile__ ( \
    A("push r16")                      /* 2 Save R16 */ \
    A("in r16, __SREG__")              /* 1 Get SREG */ \
    A("push r16")                      /* 2 Save SREG into stack */ \
    A("lds r16, %[timsk0]")            /* 2 Load into R0 the Temperature timer Interrupt mask register */ \
    A("push r16")                      /* 2 Save it into the stack */ \
    A("andi r16,~%[msk0]")             /* 1 Disable the temperature ISR */ \
    A("sts %[timsk0], r16")            /* 2 And set the new value */ \
    A("lds r16, %[timsk1]")            /* 2 Load into R0 the stepper timer Interrupt mask register */ \
    A("andi r16,~%[msk1]")             /* 1 Disable the stepper ISR */ \
    A("sts %[timsk1], r16")            /* 2 And set the new value */ \
    A("sei")                           /* 1 Enable global interrupts */    \
    A("push r0")                       /* C runtime can modify all the following registers without restoring them */ \
    A("push r1")                       \
    A("push r18")                      \
    A("push r19")                      \
    A("push r20")                      \
    A("push r21")                      \
    A("push r22")                      \
    A("push r23")                      \
    A("push r24")                      \
    A("push r25")                      \
    A("push r26")                      \
    A("push r27")                      \
    A("push r30")                      \
    A("push r31")                      \
    A("clr r1")                        /* C runtime expects this register to be 0 */ \
    A("call TIMER1_COMPA_vect_bottom") /* Call the bottom handler */   \
    A("pop r31")                       \
    A("pop r30")                       \
    A("pop r27")                       \
    A("pop r26")                       \
    A("pop r25")                       \
    A("pop r24")                       \
    A("pop r23")                       \
    A("pop r22")                       \
    A("pop r21")                       \
    A("pop r20")                       \
    A("pop r19")                       \
    A("pop r18")                       \
    A("pop r1")                        \
    A("pop r0")                        \
    A("cli")                           /* 1 Disable global interrupts */ \
    A("ori r16,%[msk1]")               /* 2 Reenable the stepper ISR */ \
    A("sts %[timsk1], r16")            /* 2 And restore the old value */ \
    A("pop r16")                       /* 2 Get the temperature timer Interrupt mask register */ \
    A("sts %[timsk0], r16")            /* 2 And restore the old value */ \
    A("pop r16")                       /* 2 Get the old SREG */ \
    A("out __SREG__, r16")             /* 1 And restore the SREG value */ \
    A("pop r16")                       /* 2 Restore R16 */ \
    A("reti")                          /* 4 Return from interrupt */ \
    :                                   \
    : [timsk0] "i" ((uint16_t)&TIMSK0), \
      [timsk1] "i" ((uint16_t)&TIMSK1), \
      [msk0] "M" ((uint8_t)(1<<OCIE0B)),\
      [msk1] "M" ((uint8_t)(1<<OCIE1A)) \
    : \
  ); \
} \
void TIMER1_COMPA_vect_bottom(void)

/* 14 cycles maximum latency */
#define TEMP_TIMER_ISR \
extern "C" void TIMER0_COMPB_vect (void) __attribute__ ((signal, naked, used, externally_visible)); \
extern "C" void TIMER0_COMPB_vect_bottom(void)  asm ("TIMER0_COMPB_vect_bottom") __attribute__ ((used, externally_visible)); \
void TIMER0_COMPB_vect (void) { \
  __asm__ __volatile__ ( \
    A("push r16")                       /* 2 Save R16 */ \
    A("in r16, __SREG__")               /* 1 Get SREG */ \
    A("push r16")                       /* 2 Save SREG into stack */ \
    A("lds r16, %[timsk0]")             /* 2 Load into R0 the Temperature timer Interrupt mask register */ \
    A("andi r16,~%[msk0]")              /* 1 Disable the temperature ISR */ \
    A("sts %[timsk0], r16")             /* 2 And set the new value */ \
    A("sei")                            /* 1 Enable global interrupts */    \
    A("push r0")                        /* C runtime can modify all the following registers without restoring them */ \
    A("push r1")                        \
    A("push r18")                       \
    A("push r19")                       \
    A("push r20")                       \
    A("push r21")                       \
    A("push r22")                       \
    A("push r23")                       \
    A("push r24")                       \
    A("push r25")                       \
    A("push r26")                       \
    A("push r27")                       \
    A("push r30")                       \
    A("push r31")                       \
    A("clr r1")                         /* C runtime expects this register to be 0 */ \
    A("call TIMER0_COMPB_vect_bottom")  /* Call the bottom handler */   \
    A("pop r31")                        \
    A("pop r30")                        \
    A("pop r27")                        \
    A("pop r26")                        \
    A("pop r25")                        \
    A("pop r24")                        \
    A("pop r23")                        \
    A("pop r22")                        \
    A("pop r21")                        \
    A("pop r20")                        \
    A("pop r19")                        \
    A("pop r18")                        \
    A("pop r1")                         \
    A("pop r0")                         \
    A("cli")                            /* 1 Disable global interrupts */ \
    A("ori r16,%[msk0]")                /* 2 Enable temperature ISR */ \
    A("sts %[timsk0], r16")             /* 2 And restore the old value */ \
    A("pop r16")                        /* 2 Get the old SREG */ \
    A("out __SREG__, r16")              /* 1 And restore the SREG value */ \
    A("pop r16")                        /* 2 Restore R16 */ \
    A("reti")                           /* 4 Return from interrupt */ \
    :                                   \
    : [timsk0] "i"((uint16_t)&TIMSK0),  \
      [msk0] "M" ((uint8_t)(1<<OCIE0B)) \
    : \
  ); \
} \
void TIMER0_COMPB_vect_bottom(void)

// Processor-level delays for hardware interfaces

#define nop() __asm__ __volatile__("nop;\n\t":::)

FORCE_INLINE static void HAL_delay_4cycles(uint8_t cy) {
  __asm__ __volatile__(
    "1:\n\t"
    " dec %[cnt]\n\t"
    " nop\n\t"
    " brne 1b\n\t"
    : [cnt] "+r"(cy)  // output: +r means input+output
    :                 // input:
    : "cc"            // clobbers:
  );
}

FORCE_INLINE static void HAL_delay_cycles(uint16_t cycles) {

  if (__builtin_constant_p(cycles)) {
    #define MAXNOPS 4

    if (cycles <= (MAXNOPS)) {
      switch (cycles) { case 4: nop(); case 3: nop(); case 2: nop(); case 1: nop(); }
    }
    else {
      const uint32_t rem = (cycles) % (MAXNOPS);
      switch (rem) { case 3: nop(); case 2: nop(); case 1: nop(); }
      if ((cycles = (cycles) / (MAXNOPS)))
        HAL_delay_4cycles(cycles); // if need more then 4 nop loop is more optimal
    }

    #undef MAXNOPS
  }
  else
    HAL_delay_4cycles(cycles / 4);
}

#undef nop

class InterruptProtectedBlock {
  uint8_t sreg;
  public:
    inline void protect() {
      cli();
    }

    inline void unprotect() {
      SREG = sreg;
    }

    inline InterruptProtectedBlock(bool later = false) {
      sreg = SREG;
      if (!later) cli();
    }

    inline ~InterruptProtectedBlock() {
      SREG = sreg;
    }
};

void HAL_stepper_timer_start();
void HAL_temp_timer_start();
void HAL_temp_isr();

class HAL {

  public: /** Constructor */

    HAL();

    virtual ~HAL();

  public: /** Public Parameters */

    #if ANALOG_INPUTS > 0
      static int16_t AnalogInputValues[NUM_ANALOG_INPUTS];
      static bool Analog_is_ready;
    #endif

    static bool execute_100ms;

  public: /** Public Function */

    #if ANALOG_INPUTS > 0
      static void analogStart();
      static void AdcChangePin(const pin_t old_pin, const pin_t new_pin);
    #endif

    static void hwSetup();

    static void showStartReason();

    static int getFreeRam();
    static void resetHardware();

    static void setPwmFrequency(const pin_t pin, uint8_t val);

    static inline void analogWrite(const pin_t pin, const uint8_t value) {
      ::analogWrite(pin, value);
    }
    static inline void digitalWrite(const pin_t pin, const uint8_t value) {
      ::digitalWrite(pin, value);
    }
    static inline uint8_t digitalRead(const pin_t pin) {
      return ::digitalRead(pin);
    }
    static inline void pinMode(const pin_t pin, const uint8_t mode) {
      switch (mode) {
        case INPUT:
          ::pinMode(pin, INPUT); break;
        case OUTPUT:
          ::pinMode(pin, OUTPUT); break;
        case OUTPUT_LOW:
          ::pinMode(pin, OUTPUT);
          ::digitalWrite(pin, LOW);
          break;
        case OUTPUT_HIGH:
          ::pinMode(pin, OUTPUT);
          ::digitalWrite(pin, HIGH);
          break;
        default: break;
      }
    }
    static inline void setInputPullup(const pin_t pin, const bool onoff) {
      ::digitalWrite(pin, onoff);
    }

    FORCE_INLINE static void delayNanoseconds(const uint32_t delayNs) {
      HAL_delay_cycles(delayNs * (CYCLES_PER_US) / 1000L);
    }
    FORCE_INLINE static void delayMicroseconds(const uint32_t delayUs) {
      HAL_delay_cycles(delayUs * (CYCLES_PER_US));
    }
    static inline void delayMilliseconds(uint16_t delayMs) {
      uint16_t del;
      while (delayMs > 0) {
        del = delayMs > 100 ? 100 : delayMs;
        delay(del);
        delayMs -= del;
        watchdog.reset();
      }
    }
    static inline uint32_t timeInMilliseconds() {
      return millis();
    }

    static inline void serialSetBaudrate(const uint16_t baud) {
      MKSERIAL.begin(baud);
      HAL::delayMilliseconds(1);
    }
    static inline bool serialByteAvailable() {
      return MKSERIAL.available() > 0;
    }
    static inline uint8_t serialReadByte() {
      return MKSERIAL.read();
    }
    static inline void serialWriteByte(const char b) {
      MKSERIAL.write(b);
    }
    static inline void serialFlush() {
      MKSERIAL.flush();
    }

    // SPI related functions
    #if ENABLED(SOFTWARE_SPI)
      static void spiBegin();
      static void spiInit(uint8_t spiRate);
      static uint8_t spiReceive(void);
      static void spiReadBlock(uint8_t* buf, uint16_t nbyte);
      static void spiSend(uint8_t b);
      static void spiSendBlock(uint8_t token, const uint8_t* buf);
    #else
      // Hardware setup
      static void spiBegin();
      static void spiInit(uint8_t spiRate);
      // Write single byte to SPI
      static void spiSend(byte b);
      static void spiSend(const uint8_t* buf, size_t n);
      // Read single byte from SPI
      static uint8_t spiReceive(uint8_t send=0xFF);
      // Read from SPI into buffer
      static void spiReadBlock(uint8_t* buf, uint16_t nbyte);
      // Write from buffer to SPI
      static void spiSendBlock(uint8_t token, const uint8_t* buf);
    #endif

};

#endif /* _HAL_AVR_H_ */
