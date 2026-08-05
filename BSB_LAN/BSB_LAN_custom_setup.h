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

attachInterrupt(digitalPinToInterrupt(WATER_METER_PIN), water_meter_isr, FALLING);
#endif

