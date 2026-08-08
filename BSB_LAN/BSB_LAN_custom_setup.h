// Add custom code for setup function here which will be included at the end of the function

// --- Compteur d'eau : initialisation ---
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
pinMode(WATER_METER_PIN, INPUT_PULLUP);

waterPrefs.begin("watermtr", false);
water_total_liters = waterPrefs.getULong("total", WATER_METER_SEED_LITERS);

water_last_calc_ms = millis();
water_last_save_ms = millis();

custom_floats[0] = 0;                   // débit L/min (param 20700)
custom_floats[1] = water_total_liters;  // volume total L (param 20701)
custom_floats[2] = 12345;               // marqueur de test OTA (param 20702) -- à retirer une fois l'upload confirmé

attachInterrupt(digitalPinToInterrupt(WATER_METER_PIN), water_meter_isr, CHANGE);
printFmtToDebug("Water meter: CHANGE interrupt attached on GPIO%d, idle pin_state=%d (should be 1/HIGH if wiring OK)\r\n", WATER_METER_PIN, digitalRead(WATER_METER_PIN));
#endif

