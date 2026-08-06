/* 
 * This code is run at the end of each main loop and utilizes the main loop variables 
 * custom_timer (set each loop to millis()) and custom_timer_compare.
 * This short example prints a "Ping!" message every 60 seconds.
*/

if (custom_timer > custom_timer_compare+60000) {    // every 60 seconds
  custom_timer_compare = millis();
  printFmtToDebug("%lu Ping!\r\n", millis());
}

// --- Compteur d'eau : scrutation du contact (durée continue à l'état bas), calcul du débit, sauvegarde ---
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
{
  unsigned long water_now_ms = millis();
  bool water_pin_low_now = (digitalRead(WATER_METER_PIN) == LOW);

  if (water_pin_low_now && !water_pin_was_low) {
    water_low_since_ms = water_now_ms;
    water_pulse_counted_this_low = false;
  }

  if (water_pin_low_now && !water_pulse_counted_this_low && (water_now_ms - water_low_since_ms) >= WATER_METER_STABLE_LOW_MS) {
    water_pulse_count++;
    water_pulse_counted_this_low = true;
    if (water_last_valid_pulse_ms > 0) {
      water_last_pulse_interval_ms = water_now_ms - water_last_valid_pulse_ms;
    }
    water_last_valid_pulse_ms = water_now_ms;
    water_total_liters++;                          // 1 impulsion = 1 litre
    custom_floats[1] = water_total_liters;          // L, param 20701
    printFmtToDebug("Water debug: pulse counted, total=%lu interval_ms=%lu\r\n", water_total_liters, water_last_pulse_interval_ms);
  }

  water_pin_was_low = water_pin_low_now;

  if (water_now_ms - water_last_calc_ms >= WATER_METER_CALC_INTERVAL_MS) {
    unsigned long water_since_last_pulse_ms = water_now_ms - water_last_valid_pulse_ms;

    if (water_pulse_count >= 2 && water_last_valid_pulse_ms > 0 && water_since_last_pulse_ms < WATER_METER_TIMEOUT_MS) {
      custom_floats[0] = 60000.0 / water_last_pulse_interval_ms; // L/min, param 20700
    } else {
      custom_floats[0] = 0;
    }

    printFmtToDebug("Water debug: total_pulses=%lu flow=%.2f pin_state=%d\r\n", water_pulse_count, custom_floats[0], !water_pin_low_now);

    water_last_calc_ms = water_now_ms;
  }

  if (water_now_ms - water_last_save_ms >= WATER_METER_SAVE_INTERVAL_MS) {
    waterPrefs.putULong("total", water_total_liters);
    water_last_save_ms = water_now_ms;
  }
}
#endif
