// Add custom code for global functions here which will be included in the global section

// --- Compteur d'eau (reed switch 2 fils, GPIO4, 1 impulsion = 1 litre) ---
// Migré depuis l'ESP32 "water-meter" (ESPHome, pulse_meter). Débit -> custom_floats[0] (param 20700),
// volume total -> custom_floats[1] (param 20701). Fonctionne en parallèle de l'ESPHome le temps de valider.
// GPIO14 (rangée gauche) inaccessible physiquement -> GPIO22 essayé mais réservé par Wire.begin() (I2C SCL,
// appelé sans condition dans le setup()) -> GPIO4 (rangée droite, libre, non utilisé hors cartes Olimex).
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#include <Preferences.h>

#define WATER_METER_PIN 4
#define WATER_METER_DEBOUNCE_US 800000UL       // 800 ms anti-rebond (200/400ms insuffisants : sur-comptage ~2x stable observé face à l'ESPHome).
                                                 // Tolère jusqu'à 75 L/min avant sous-comptage, largement suffisant pour une conduite principale.
#define WATER_METER_CALC_INTERVAL_MS 5000UL    // rafraîchissement de l'affichage du débit toutes les 5 s
#define WATER_METER_SAVE_INTERVAL_MS 300000UL  // sauvegarde NVS toutes les 5 min (usure flash)
#define WATER_METER_SEED_LITERS 165UL          // valeur relevée sur l'ESPHome au moment de la migration (05/08/2026)
#define WATER_METER_TIMEOUT_US 120000000UL     // 2 min sans impulsion -> débit affiché à 0 (comme ESPHome)

Preferences waterPrefs;

volatile unsigned long water_pulse_isr_count = 0;
volatile unsigned long water_last_pulse_us = 0;
volatile unsigned long water_last_pulse_interval_us = 0;  // temps entre les 2 dernières impulsions, pour un débit précis
volatile unsigned long water_isr_wrong_state_count = 0;   // diagnostic : nb de déclenchements avec pin HIGH (FALLING qui se comporterait comme CHANGE)

unsigned long water_total_liters = 0;
unsigned long water_pulses_at_last_calc = 0;
unsigned long water_last_calc_ms = 0;
unsigned long water_last_save_ms = 0;

void IRAM_ATTR water_meter_isr() {
  if (digitalRead(WATER_METER_PIN) != LOW) {
    water_isr_wrong_state_count++;   // diagnostic : ISR déclenchée alors que la broche n'est pas basse
    return;
  }
  unsigned long now = micros();
  unsigned long since_last = now - water_last_pulse_us;
  if (since_last > WATER_METER_DEBOUNCE_US) {
    water_last_pulse_interval_us = since_last;
    water_pulse_isr_count++;
    water_last_pulse_us = now;
  }
}
#endif

