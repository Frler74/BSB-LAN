// Add custom code for global functions here which will be included in the global section

// --- Compteur d'eau (reed switch 2 fils, GPIO4, 1 impulsion = 1 litre) ---
// Migré depuis l'ESP32 "water-meter" (ESPHome, pulse_meter). Débit -> custom_floats[0] (param 20700),
// volume total -> custom_floats[1] (param 20701).
// GPIO14 (rangée gauche) inaccessible physiquement -> GPIO22 essayé mais réservé par Wire.begin() (I2C SCL,
// appelé sans condition dans le setup()) -> GPIO4 (rangée droite, libre, non utilisé hors cartes Olimex).
//
// Historique :
// 1) Interruption FALLING + anti-rebond par écart de temps entre impulsions -> sur-comptage ~2x stable,
//    reproductible même isolé de l'ESPHome, insensible à 200-800ms d'anti-rebond et à un condensateur 100nF.
// 2) Scrutation logicielle (sans interruption) de la durée continue à l'état bas -> correct sur tests lents
//    contrôlés (bidons 5L/2L/1L, précision parfaite), mais sous-comptage ~10% mesuré à débit soutenu
//    (arrosage, ~6-7 L/min) : loop() sur BSB-LAN n'est pas régulier (bloqué 100-300ms+ pendant les requêtes
//    sur le bus de chauffage), donc une impulsion brève peut survenir entièrement entre deux passages de
//    boucle et n'être jamais vue.
// 3) Interruption CHANGE, horodatage micros() à chaque front, largeur mesurée (montant - descendant) ->
//    sur-comptage erratique ~3.5x, pire qu'avant. Cause : water_fall_time_us était réinitialisé à CHAQUE
//    front descendant, y compris un rebond de refermeture survenant au milieu d'une fermeture déjà en
//    cours -> chaque rebond pendant une fermeture longue pouvait être évalué comme sa propre impulsion
//    valide (largeur mesurée depuis le rebond précédent, pas depuis la vraie fermeture initiale).
// 4) Solution actuelle : mêmes interruptions CHANGE, mais un drapeau "impulsion en cours" empêche de
//    redémarrer le chrono sur un rebond de refermeture -> le chrono ne démarre qu'à la toute première
//    fermeture, et la largeur n'est mesurée/validée qu'au relâchement final. Reproduit fidèlement le
//    principe du filtre PULSE d'ESPHome (qui ne traite un front que si l'état logique a réellement changé
//    par rapport au dernier état connu).
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
#include <Preferences.h>

#define WATER_METER_PIN 4
#define WATER_METER_STABLE_LOW_US 150000UL     // largeur minimale d'impulsion validée (150ms, comme ESPHome internal_filter)
#define WATER_METER_CALC_INTERVAL_MS 5000UL    // rafraîchissement de l'affichage du débit toutes les 5 s
#define WATER_METER_SAVE_INTERVAL_MS 300000UL  // sauvegarde NVS toutes les 5 min (usure flash)
#define WATER_METER_SEED_LITERS 165UL          // valeur relevée sur l'ESPHome au moment de la migration (05/08/2026)
#define WATER_METER_TIMEOUT_MS 120000UL        // 2 min sans impulsion -> débit affiché à 0 (comme ESPHome)

Preferences waterPrefs;

volatile unsigned long water_fall_time_us = 0;
volatile bool water_pulse_in_progress = false;
volatile unsigned long water_pulse_count = 0;
volatile unsigned long water_last_valid_pulse_us = 0;
volatile unsigned long water_last_pulse_interval_us = 0;

unsigned long water_total_liters = 0;
unsigned long water_pulses_at_last_calc = 0;
unsigned long water_last_calc_ms = 0;
unsigned long water_last_save_ms = 0;

void IRAM_ATTR water_meter_isr() {
  unsigned long now = micros();
  if (digitalRead(WATER_METER_PIN) == LOW) {
    // front descendant : ne démarre le chrono que si aucune fermeture n'est déjà en cours
    // (ignore les rebonds de refermeture au milieu d'une fermeture déjà commencée)
    if (!water_pulse_in_progress) {
      water_fall_time_us = now;
      water_pulse_in_progress = true;
    }
  } else {
    // front montant : ne finalise que si une fermeture était en cours
    if (water_pulse_in_progress) {
      unsigned long width = now - water_fall_time_us;
      if (width >= WATER_METER_STABLE_LOW_US) {
        water_pulse_count++;
        if (water_last_valid_pulse_us > 0) {
          water_last_pulse_interval_us = now - water_last_valid_pulse_us;
        }
        water_last_valid_pulse_us = now;
      }
      water_pulse_in_progress = false;
    }
  }
}
#endif
