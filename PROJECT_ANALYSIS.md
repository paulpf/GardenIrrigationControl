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

## 9. Priorisierter Maßnahmenplan

### Phase 1 – Kritische Bugs (1–2 Sitzungen)
1. WDT-Divisor korrigieren
2. Restzeit-Unterlauf-Fix (`unsigned long` + Clamp)
3. MM:SS-Formatierung mit `snprintf`
4. DHT-Timestamp-Topic trennen
5. MQTT-Schleifen auf `_numIrrigationZones`

### Phase 2 – Code-Hygiene (1 Sitzung)
6. Toten Code entfernen
7. Dateinamen-Groß/Kleinschreibung vereinheitlichen
8. WiFi-Init explizit machen
9. OTA Div-by-Zero absichern

### Phase 3 – Modernisierung (2–3 Sitzungen)
10. `#define` → `constexpr`
11. `ZoneConfig`-Struct
12. Heap-String-Minimierung
13. `Trace`-Zeitstempel

### Phase 4 – Features (nach Bedarf)
14. Web-Interface
15. Wasserstandssensor
16. Zeitgesteuerte Bewässerung
