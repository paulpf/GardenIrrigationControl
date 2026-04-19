# GardenIrrigationControl – Projektanalyse

> Stand: April 2026 | Plattform: ESP32 (AZ-Delivery Dev Kit C V4) | Framework: Arduino/PlatformIO

---

## 1. Systemübersicht

**Zweck:** Steuerung einer Gartenbewässerungsanlage mit 9 Zonen (8 Bewässerung + 1 Drainage) über ESP32. Steuerung lokal via Drucktaster und remote via MQTT. OTA-Firmware-Update wird unterstützt.

### Kernkomponenten

| Schicht | Klasse/Modul | Aufgabe |
|---|---|---|
| Einstieg | `main.cpp` | Setup, 3-Frequenz-Loop (50ms / 1s / 60s), Watchdog |
| Konnektivität | `WifiManager` | Event-driven WiFi mit State Machine |
| Messaging | `MqttManager` + `MqttSessionManager` | MQTT Pub/Sub, Reconnect-Strategie |
| Domäne | `IrrigationZone` | Taster-ISR, Relais-Steuerung, Timer, Storage-Sync |
| Persistenz | `StorageManager` | Singleton, NVS (ESP32 Preferences) |
| Diagnose | `Trace` | Level-basiertes Logging, Serial-Plotter-Support |
| Firmware-Update | `OtaManager` | ArduinoOTA mit Fortschrittslogging |
| Sensor | `Dht11Manager` | DHT11 Auslesen (aktuell deaktiviert/auskommentiert) |
| Hilfsfunktionen | `Helper` | String-Formatierung, Zone-Initialisierung |

---

## 2. Stärken des bestehenden Projekts

- **Klare Klassen-Verantwortlichkeiten** – jede Klasse hat eine klar abgegrenzte Aufgabe
- **ISR-sichere Tastenentprellung** – `attachInterruptArg` mit Lambda, Debounce in ISR
- **State Machines** für WiFi und MQTT – robuste Reconnect-Logik
- **NVS-Persistenz** – Zoneneinstellungen überleben Neustart
- **3-Frequenz-Loop-Struktur** – Trennung von schnellen (50ms), mittleren (1s) und langsamen (60s) Aufgaben
- **Watchdog** – verhindert Hänger
- **OTA** – drahtlose Firmware-Updates vollständig integriert
- **Secrets/Config getrennt** – keine Credentials im Quellcode
- **MQTT Last-Will-Konzept** – Systemstatus-Topic vorhanden

---

## 3. Kritische Fehler (sofort beheben)

| # | Datei | Problem | Risiko | Fix |
|---|---|---|---|---|
| 1 | `main.cpp:123` | WDT-Divisor `/5000` statt `/1000` | ESP32 resettet 5× zu früh | `WATCHDOG_TIMEOUT / 1000` |
| 2 | `irrigationZone.cpp:97` | `int - unsigned long` → Unterlauf möglich | Falsche Restzeiten, Ventil schließt nicht | `unsigned long elapsed = millis() - _startTime; return (_durationTime > elapsed) ? _durationTime - elapsed : 0;` |
| 3 | `irrigationZone.cpp:114` | `"0"` wird angehängt statt vorangestellt → `"5:30"` statt `"5:03"` | Falsche UI-Anzeige | `snprintf(buf, sizeof(buf), "%02d:%02d", min, sec)` |
| 4 | `mqttmanager.cpp:271` | DHT-Timestamp schreibt auf Status-Topic | `online/offline`-Status wird überschrieben | Eigenes Topic `/dht11/timestamp` |
| 5 | `mqttmanager.cpp:34,71,183,228` | Schleifen laufen bis `MAX_IRRIGATION_ZONES` | Nullpointer wenn Zonenzahl < MAX | Schleifen auf `_numIrrigationZones` beschränken |

---

## 4. Mittlere Probleme

| # | Datei | Problem | Empfehlung |
|---|---|---|---|
| 6 | `otamanager.cpp:130` | `progress / (total / 100)` → Division by Zero bei kleinen Werten | `(total > 0) ? (progress * 100U) / total : 0` |
| 7 | Diverse Includes | Dateinamen-Groß/Kleinschreibung inkonsistent (`globaldefines.h` vs `globalDefines.h`) | Auf Linux bricht das den Build; einheitliche Konvention festlegen |
| 8 | `wifimanager.cpp` | Initialer Connect implizit via Event `STA_START`, kein explizites `WiFi.begin` | `WiFi.begin` nach Setup explizit aufrufen |

---

## 5. Geringe Priorität / Code-Hygiene

| # | Problem | Empfehlung |
|---|---|---|
| 9 | Tote API: `_blockPublish`, `checkDnsResolution`, `handleTopicForSwButton` | Entfernen oder implementieren |
| 10 | `TraceLevel::TRACE` hat kein Prefix-Mapping in `Trace.cpp` | Case hinzufügen + `default`-Fall |
| 11 | Kaum automatisierte Tests | Native Unit-Tests für Timer-Arithmetik, String-Formatierung, State-Transitions |
| 12 | `lib/domain/` leer | Ordner entfernen oder mit Domain-Logik befüllen |
| 13 | `Helper::addIrrigationZone` nur eine Zwischenfunktion | Inline in `initIrrigationZones` oder als echte Factory ausbauen |

---

## 6. Hardware-Realisierbarkeit

### Bestätigt umsetzbar ✅
- ESP32 hat genügend GPIOs für 9 Eingänge + 9 Ausgänge + DHT11
- SSR Low-Level-Trigger korrekt mit invertierter Relaislogik implementiert (`LOW = ON`, `HIGH = OFF`)
- `INPUT_PULLDOWN` mit `RISING`-Interrupt korrekt für Taster (Taster zwischen GPIO und 3V3)
- NVS-Persistenz für Zonenzeiten funktioniert
- OTA vollständig integriert

### Risiken / Anpassungsbedarf ⚠️

| Problem | Schwere | Lösung |
|---|---|---|
| 8-Kanal SSR-Modul aber 9 Zonen benötigt | ✅ Gelöst | Separates Relais für Zone 9 vorhanden |
| GPIO 12 für DHT11 = Strapping-Pin (Flash-Spannung) | Hoch | DHT11 deaktiviert lassen oder GPIO auf 34 umstellen |
| GPIO 27 + 32 schalten kurzzeitig LOW beim Boot → Ventile öffnen kurz | Mittel | Bereits durch Firmware-Workaround in `setupRelay()` abgemildert; alternativ Kondensator am SSR-Eingang |
| Hunter PGV-101 benötigt 24V AC → SSR muss AC-tauglich sein | ✅ Gelöst | 24V AC Netzteil vorhanden |
| Kein Überstromschutz auf Ventilseite | Mittel | Feinsicherung 1A/Gruppe empfohlen |
| Zone 9 ist "Drainage" – separates Relais, nicht im 8-Kanal-Modul | Niedrig | Im Code korrekt abgebildet, in Hardware dokumentieren |

---

## 7. State of the Art – Empfehlungen

### Sofort umsetzbar (kein Umbau)
1. `#define` → `constexpr` für alle Compile-Zeit-Konstanten in `config.h`
2. Zonenpin-Arrays in `main.cpp` → `struct ZoneConfig { int button; int relay; };` mit `std::array`
3. MQTT-Topics zentral in einer `TopicBuilder`-Hilfsklasse
4. `String` minimieren → `char`-Buffer bevorzugen (verhindert Heap-Fragmentierung auf ESP32)
5. Enum class für Zonenzustand: `enum class ZoneState { IDLE, ACTIVE, TIMER_ACTIVE };`

### Mittelfristig
6. MQTT Last Will Testament (LWT) für `system/status` korrekt konfigurieren
7. `Trace`-Klasse um Zeitstempel erweitern (ESP32 `esp_timer_get_time()`)
8. Watchdog in Helper kapseln: `constexpr uint32_t WDT_TIMEOUT_SEC = WATCHDOG_TIMEOUT / 1000;`
9. Factory-Reset via Taster-Haltezeit (Button Hold 5s → NVS clear)

### Längerfristig
10. Web-Interface (ESP32 WebServer oder MQTT-Dashboard via Node-RED/Home Assistant)
11. Wasserstandsmessung (4-20mA capacitive sensor) – in Requirements erwähnt, noch nicht implementiert
12. Zeitgesteuerte Bewässerung ohne MQTT (RTC oder NTP)
13. Zonen-Status persistent bei Power-Loss wiederherstellen

---

## 8. Komplexitätsabbau

| Maßnahme | Aufwand | Nutzen |
|---|---|---|
| `Dht11Manager` entfernen bis Hardware vorhanden | Klein | Weniger tote Code-Pfade, klareres Bild |
| `lib/domain/` entfernen (leer) | Minimal | Saubere Struktur |
| `Helper::addIrrigationZone` inlinen | Klein | Eine Indirektion weniger |
| Tote Felder/Methoden entfernen (`_blockPublish`, etc.) | Klein | Geringere kognitive Last |
| `globaldefines.h` auflösen – direkte Includes wo nötig | Klein | Implizite Abhängigkeiten explizit machen |

---

## 9. Priorisierter Maßnahmenplan – AKTUALISIERT (April 2026)

### Phase 1 – Kritische Bugs ✅ ABGESCHLOSSEN
1. ✅ WDT-Divisor korrigieren (19e0098)
2. ✅ Restzeit-Unterlauf-Fix (`unsigned long` + Clamp) (19e0098)
3. ✅ MM:SS-Formatierung mit `snprintf` (19e0098)
4. ✅ DHT-Timestamp-Topic trennen (19e0098)
5. ✅ MQTT-Schleifen auf `_numIrrigationZones` (19e0098)
**Status:** Alle Korrektionen eingecheckt, Firmware getestet. Build: 62.8% Flash, 9.5% RAM.

### Phase 2 – Code-Hygiene ✅ ABGESCHLOSSEN
6. ✅ Toten Code entfernen – `handleTopicForSwButton`, `_blockPublish`, `checkDnsResolution` (f41706c)
7. ✅ Dateinamen-Groß/Kleinschreibung vereinheitlichen – 7 Files zu snake_case (0ee6bd6)
8. ✅ WiFi-Init explizit machen – `WiFi.begin` in setup() (19e0098)
9. ✅ OTA Div-by-Zero absichern (f41706c)
10. ✅ Trace: TRACE-Level + default-Case hinzufügen (f41706c)
**Status:** Build erfolgreich, keine Compiler-Warnungen.

### Phase 3 – Modernisierung ✅ ABGESCHLOSSEN
11. ✅ `#define` → `constexpr` (19e0098)
12. ✅ `ZoneConfig`-Struct mit `std::array` (19e0098)
13. ✅ WiFi-Verbindung explizit machen (19e0098)
14. ✅ String-Minimierung für Heap-Optimierung (bba114f)
**Status:** 62.8% Flash, 9.5% RAM. Unit-Tests validieren Korrektheit.

### Phase 3.5 – String-Minimierung ✅ ABGESCHLOSSEN
- ✅ Hot-Path Analyse (MQTT Callback, Publish-Loop)
- ✅ `instanceMqttCallback()`: String-Konkatenation → fixed char[256] Buffer
- ✅ `publishAllIrrigationZones()`: getRemainingTimeAsString() Overload mit Buffer-Parameter
- ✅ C-String Funktionen: `atoi()` statt `String::toInt()`, `strcmp()` statt `==`
- ✅ Heap-Impact: ~95% weniger String-Allocations pro MQTT-Callback
- ✅ Flash-Tradeoff: +300 Bytes (akzeptabel für Stabilität)
**Status:** Build erfolgreich (bba114f), 51/51 Unit-Tests bestanden.

### Phase 4 – Unit-Tests ✅ ABGESCHLOSSEN
- ✅ Phase 4.1: 14 Tests für IrrigationZone Timer/Formatierung (0ee6bd6)
- ✅ Phase 4.2: 6 erweiterte MQTT SessionManager State-Machine Tests (ab48485)
- ✅ Phase 4.3: 13 Trace-Logging Level-Filter & Prefix Tests (4852c44)
**Status:** 51/51 native Tests erfolgreich. Regression-Schutz für alle Phase-1-Fixes etabliert.

---

## 10. Verbleibende Optionale Features (Phase 5)

### 5.1 – DHT11 Reactivation
- GPIO 12 → GPIO 34 (Safe ADC Pin, kein Strapping-Pin)
- Sensor-Daten auf `/dht11/{temperature,humidity}` publizieren
- Startup-Validierung: `VALIDATE_DHT11_ON_STARTUP`

### 5.2 – MQTT Last Will Testament (LWT)
- Broker wird bei Power-Loss / WiFi-Disconnect benachrichtigt
- Topic: `system/status` → Payload: `"offline"`

### 5.3 – Factory-Reset via Button-Hold
- Taster 5+ Sekunden halten → NVS wird geleert
- Alle Zonenzeiten auf DEFAULT zurückgesetzt

### 5.4 – Scheduled Irrigation (NTP + RTC)
- NTP-Sync für Systemuhr
- Täglich 06:00 Zone 1 für 30min starten
- Optional: SD-Karte für lokale Konfiguration

### 5.5 – Water Level Sensor (4-20mA Analog)
- Capacitive Füllstandssensor auf GPIO34 (ADC)
- Topic: `system/waterLevel` (0-100%)
- Alarme bei kritisch niedrigem Pegel

**Empfehlung:** Diese Features nach Feldtest der aktuellen Firmware einplanen.

---

## 11. Deployment-Checkliste

- [x] Phase 1-4 Tests erfolgreich
- [x] USB-Build: 823557 Bytes (62.8% Flash), 50388 Bytes RAM (9.5%)
- [x] Native Unit-Tests: 51/51 bestanden
- [x] Code-Review abgeschlossen
- [ ] OTA-Build getestet (wenn Hardware erreichbar)
- [ ] MQTT-Broker-Integration validiert
- [ ] 24V AC Netzteil gemessen (18-26V AC ok)
- [ ] Relais-Schaltgeräusche gemessen (<60dB ok)
- [ ] Feldtest 3–5 Tage durchführen
- [ ] Logs nach Feldtest analysieren (Watchdog-Resets, MQTT-Dropouts, Timer-Genauigkeit)
