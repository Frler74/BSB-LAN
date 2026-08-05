// Add custom code for global functions here which will be included in the global section

// --- Compteur d'eau (reed switch 2 fils, GPIO14, 1 impulsion = 1 litre) ---
// Migré depuis l'ESP32 "water-meter" (ESPHome, pulse_meter). Débit -> custom_floats[0] (param 20700),
// volume total -> custom_floats[1] (param 20701). Fonctionne en parallèle de l'ESPHome le temps de valider.
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#include <Preferences.h>

#define WATER_METER_PIN 14
#define WATER_METER_DEBOUNCE_US 200000UL       // 200 ms anti-rebond, comme le filtre ESPHome d'origine
#define WATER_METER_CALC_INTERVAL_MS 5000UL    // recalcul du débit toutes les 5 s
#define WATER_METER_SAVE_INTERVAL_MS 300000UL  // sauvegarde NVS toutes les 5 min (usure flash)
#define WATER_METER_SEED_LITERS 165UL          // valeur relevée sur l'ESPHome au moment de la migration (05/08/2026)

Preferences waterPrefs;

volatile unsigned long water_pulse_isr_count = 0;
volatile unsigned long water_last_pulse_us = 0;

unsigned long water_total_liters = 0;
unsigned long water_pulses_at_last_calc = 0;
unsigned long water_last_calc_ms = 0;
unsigned long water_last_save_ms = 0;

void IRAM_ATTR water_meter_isr() {
  unsigned long now = micros();
  if (now - water_last_pulse_us > WATER_METER_DEBOUNCE_US) {
    water_pulse_isr_count++;
    water_last_pulse_us = now;
  }
}
#endif

