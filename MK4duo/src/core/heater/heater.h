/**
 * MK4duo Firmware for 3D Printer, Laser and CNC
 *
 * Based on Marlin, Sprinter and grbl
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 * Copyright (C) 2019 Alberto Cotronei @MagoKimbra
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
#pragma once

#if HEATER_COUNT > 0

/**
 * heater.h - heater object
 */

#include "sensor/sensor.h"
#include "pid/pid.h"

union flagheater_t {
  uint8_t all;
  struct {
    bool  Active            : 1;
    bool  UsePid            : 1;
    bool  Pidtuned          : 1;
    bool  HWInvert          : 1;
    bool  Thermalprotection : 1;
    bool  Idle              : 1;
    bool  Fault             : 1;
    bool  Pidtuning         : 1;
  };
  flagheater_t() { all = 0x00; }
};

enum HeatertypeEnum : uint8_t { IS_HOTEND, IS_BED, IS_CHAMBER, IS_COOLER };
enum TRState        : uint8_t { TRInactive, TRFirstHeating, TRStable, TRRunaway };

constexpr uint16_t  temp_check_interval[HEATER_TYPE]  = { HOTEND_CHECK_INTERVAL, BED_CHECK_INTERVAL, CHAMBER_CHECK_INTERVAL, COOLER_CHECK_INTERVAL };
constexpr uint8_t   temp_hysteresis[HEATER_TYPE]      = { HOTEND_HYSTERESIS, BED_HYSTERESIS, CHAMBER_HYSTERESIS, COOLER_HYSTERESIS };
constexpr uint8_t   watch_increase[HEATER_TYPE]       = { WATCH_HOTEND_INCREASE, WATCH_BED_INCREASE, WATCH_CHAMBER_INCREASE, WATCH_COOLER_INCREASE };

// Struct Heater data
typedef struct {

  HeatertypeEnum  type;

  pin_t           pin;

  flagheater_t    flag;

  uint8_t         ID;

  int16_t         mintemp,
                  maxtemp;

} heater_data_t;

class Heater {

  public: /** Public Parameters */

    heater_data_t data;
    pid_data_t    pid;
    sensor_data_t sensor;

    uint16_t      watch_target_temp;

    uint8_t       pwm_value,
                  consecutive_low_temp;

    int16_t       target_temperature,
                  idle_temperature;

    float         current_temperature;

  private: /** Private Function */

    TRState       thermal_runaway_state;

    millis_s      watch_next_ms;

    uint16_t      idle_timeout_time;

  public: /** Public Function */

    void init();

    void setTarget(const int16_t celsius);
    void wait_for_target(bool no_wait_for_cooling=true);
    void get_output();
    void set_output_pwm();
    void check_and_power();
    void PID_autotune(const float target_temp, const uint8_t ncycles, const uint8_t method, const bool storeValues=false);
    void print_M301();
    void print_M305();
    void print_M306();
    #if HAS_AD8495 || HAS_AD595
      void print_M595();
    #endif
    void start_idle_timer(const millis_l timeout_time);
    void reset_idle_timer();

    void thermal_runaway_protection();
    void start_watching();

    FORCE_INLINE void update_current_temperature() { this->current_temperature = this->sensor.getTemperature(); }
    FORCE_INLINE bool tempisrange() { return (WITHIN(this->current_temperature, this->data.mintemp, this->data.maxtemp)); }
    FORCE_INLINE bool isHeating()   { return this->target_temperature > this->current_temperature; }
    FORCE_INLINE bool isCooling()   { return this->target_temperature <= this->current_temperature; }

    FORCE_INLINE bool wait_for_heating() {
      return this->isActive() && ABS(this->current_temperature - this->target_temperature) > temp_hysteresis[data.type];
    }

    // Flag bit 0 Set Active
    FORCE_INLINE void setActive(const bool onoff) {
      if (!isFault() && sensor.type != 0 && onoff)
        data.flag.Active = true;
      else
        data.flag.Active = false;
    }
    FORCE_INLINE bool isActive() { return data.flag.Active; }

    // Flag bit 1 Set use Pid
    FORCE_INLINE void setUsePid(const bool onoff) { data.flag.UsePid = onoff; }
    FORCE_INLINE bool isUsePid() { return data.flag.UsePid; }

    // Flag bit 2 Set Set Pid Tuned
    FORCE_INLINE void setPidTuned(const bool onoff) { data.flag.Pidtuned = onoff; }
    FORCE_INLINE bool isPidTuned() { return data.flag.Pidtuned; }

    // Flag bit 3 Set Hardware inverted
    FORCE_INLINE void setHWinvert(const bool onoff) { data.flag.HWInvert = onoff; }
    FORCE_INLINE bool isHWinvert() { return data.flag.HWInvert; }

    // Flag bit 4 Set Thermal Protection
    FORCE_INLINE void setThermalProtection(const bool onoff) { data.flag.Thermalprotection = onoff; }
    FORCE_INLINE bool isThermalProtection() { return data.flag.Thermalprotection; }

    // Flag bit 5 Set Idle
    FORCE_INLINE void setIdle(const bool onoff, const int16_t idle_temp=0) {
      data.flag.Idle = onoff;
      idle_temperature = idle_temp;
      if (onoff) thermal_runaway_state = TRInactive;
    }
    FORCE_INLINE bool isIdle() { return data.flag.Idle; }

    // Flag bit 6 Set Fault
    FORCE_INLINE void setFault() {
      pwm_value = 0;
      setActive(false);
      data.flag.Fault = true;
    }
    FORCE_INLINE void ResetFault() {
      data.flag.Fault = false; 
      SwitchOff();
    }
    FORCE_INLINE bool isFault() { return data.flag.Fault; }

    // Flag bit 7 Set Pid Tuning
    FORCE_INLINE void setPidTuning(const bool onoff) { data.flag.Pidtuning = onoff; }
    FORCE_INLINE bool isPidTuning() { return data.flag.Pidtuning; }

    FORCE_INLINE void resetFlag() { data.flag.all = false; }

    FORCE_INLINE void SwitchOff() {
      target_temperature = 0;
      pwm_value = 0;
      setActive(false);
    }

  private: /** Private Function */

    void _temp_error(PGM_P const serial_msg, PGM_P const lcd_msg);
    void min_temp_error();
    void max_temp_error();

};

extern Heater hotends[HOTENDS];
extern Heater beds[BEDS];
extern Heater chambers[CHAMBERS];
extern Heater coolers[COOLERS];

#endif // HEATER_COUNT > 0
