// Add custom code for global functions here which will be included in the global section

// --- Compteur d'eau (reed switch 2 fils, GPIO4, 1 impulsion = 1 litre) ---
// Migré depuis l'ESP32 "water-meter" (ESPHome, pulse_meter). Débit -> custom_floats[0] (param 20700),
// volume total -> custom_floats[1] (param 20701).
// GPIO14 (rangée gauche) inaccessible physiquement -> GPIO22 essayé mais réservé par Wire.begin() (I2C SCL,
// appelé sans condition dans le setup()) -> GPIO4 (rangée droite, libre, non utilisé hors cartes Olimex).
//
// Historique : l'approche par interruption (FALLING) + anti-rebond par écart de temps entre impulsions a
// donné un sur-comptage ~2x stable et reproductible face à l'ESPHome (confirmé même isolé, sans l'ESPHome
// branché), insensible à un anti-rebond de 200 à 800ms et à un condensateur matériel de 100nF. Abandon de
// cette approche au profit d'une mesure de durée réelle de fermeture du contact (scrutation, sans
// interruption), identique au principe du filtre PULSE d'ESPHome qui, lui, fonctionne correctement sur ce
// même capteur.
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#include <Preferences.h>

#define WATER_METER_PIN 4
#define WATER_METER_STABLE_LOW_MS 200UL        // durée continue à l'état bas requise pour valider une impulsion (comme ESPHome internal_filter: 200ms)
#define WATER_METER_CALC_INTERVAL_MS 5000UL    // rafraîchissement de l'affichage du débit toutes les 5 s
#define WATER_METER_SAVE_INTERVAL_MS 300000UL  // sauvegarde NVS toutes les 5 min (usure flash)
#define WATER_METER_SEED_LITERS 165UL          // valeur relevée sur l'ESPHome au moment de la migration (05/08/2026)
#define WATER_METER_TIMEOUT_MS 120000UL        // 2 min sans impulsion -> débit affiché à 0 (comme ESPHome)

Preferences waterPrefs;

bool water_pin_was_low = false;
bool water_pulse_counted_this_low = false;
unsigned long water_low_since_ms = 0;
unsigned long water_pulse_count = 0;
unsigned long water_last_valid_pulse_ms = 0;
unsigned long water_last_pulse_interval_ms = 0;

unsigned long water_total_liters = 0;
unsigned long water_pulses_at_last_calc = 0;
unsigned long water_last_calc_ms = 0;
unsigned long water_last_save_ms = 0;
#endif
