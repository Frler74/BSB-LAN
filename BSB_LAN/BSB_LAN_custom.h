/*
 * This code is run at the end of each main loop and utilizes the main loop variables
 * custom_timer (set each loop to millis()) and custom_timer_compare.
 * This short example prints a "Ping!" message every 60 seconds.
*/

if (custom_timer > custom_timer_compare+60000) {    // every 60 seconds
  custom_timer_compare = millis();
  printFmtToDebug("%lu Ping!\r\n", millis());
}

// --- Compteur d'eau : calcul du débit + volume total à partir des impulsions détectées par interruption, sauvegarde périodique ---
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
{
  unsigned long water_now_ms = millis();

  if (water_now_ms - water_last_calc_ms >= WATER_METER_CALC_INTERVAL_MS) {
    noInterrupts();
    unsigned long water_pulses_now = water_pulse_count;
    unsigned long water_last_interval_us = water_last_pulse_interval_us;
    unsigned long water_last_pulse_time_us = water_last_valid_pulse_us;
    interrupts();

    unsigned long water_delta_pulses = water_pulses_now - water_pulses_at_last_calc;
    if (water_delta_pulses > 0) {
      water_total_liters += water_delta_pulses;        // 1 impulsion = 1 litre
      custom_floats[1] = water_total_liters;            // L, param 20701
    }

    unsigned long water_since_last_pulse_ms = (micros() - water_last_pulse_time_us) / 1000;
    if (water_pulses_now >= 2 && water_last_pulse_time_us > 0 && water_since_last_pulse_ms < WATER_METER_TIMEOUT_MS) {
      custom_floats[0] = 60000000.0 / water_last_interval_us; // L/min, param 20700
    } else {
      custom_floats[0] = 0;
    }

    printFmtToDebug("Water debug: total_pulses=%lu delta=%lu flow=%.2f pin_state=%d\r\n", water_pulses_now, water_delta_pulses, custom_floats[0], digitalRead(WATER_METER_PIN));

    water_pulses_at_last_calc = water_pulses_now;
    water_last_calc_ms = water_now_ms;
  }

  if (water_now_ms - water_last_save_ms >= WATER_METER_SAVE_INTERVAL_MS) {
    waterPrefs.putULong("total", water_total_liters);
    water_last_save_ms = water_now_ms;
  }
}
#endif
