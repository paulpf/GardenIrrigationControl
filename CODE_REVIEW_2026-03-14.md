# Code Review - GardenIrrigationControl

Datum: 2026-03-14
Scope: src, platformio.ini, scripts/setup_secrets.ps1
Hinweis: Read-only Review, keine produktiven Codeaenderungen vorgenommen.

## Kurzfazit
Das Projekt ist buildbar, aber es gibt mehrere funktionale Risiken in Zeit-/Zustandslogik und MQTT-Pfaden. Die wichtigsten Punkte betreffen Laufzeitverhalten (Timer, Watchdog, MQTT Topic-Nutzung), nicht die Build-Pipeline.

## Findings (priorisiert)

### Hoch

1. Falsche Umrechnung fuer Watchdog-Timeout
- Stelle: [src/main.cpp](src/main.cpp#L123)
- Befund: `WATCHDOG_TIMEOUT / 5000` wird als "ms -> s" kommentiert. Korrekt waere `/1000`.
- Risiko: Effektiver WDT-Timeout ist 5x kuerzer als konfiguriert, potenzielle unerwartete Resets.
- Vorschlag: Timeout-Berechnung korrigieren und zentral kapseln (z. B. helper `msToSecondsRounded`).

2. Restzeitberechnung hat Typ-/Overflow-Risiko
- Stelle: [src/irrigationZone.cpp](src/irrigationZone.cpp#L97)
- Befund: `int - unsigned long` kann bei Ablauf unterlaufen/wrappen.
- Risiko: Negative/inkonsistente Restzeiten, sporadisch falsches Abschaltverhalten.
- Vorschlag: Mit `unsigned long elapsed = millis() - _startTime` arbeiten, dann clampen auf 0.

3. Zeitformatierung MM:SS ist falsch fuer einstellige Sekunden
- Stellen: [src/irrigationZone.cpp](src/irrigationZone.cpp#L114), [src/irrigationZone.cpp](src/irrigationZone.cpp#L119)
- Befund: Bei `seconds < 10` wird `"0"` angehaengt statt vorangestellt.
- Risiko: Anzeige wird z. B. `5:30` statt `5:03`.
- Vorschlag: `snprintf("%02d:%02d", minutes, seconds)` oder String-Build mit Prefix-0.

4. DHT Timestamp wird auf Status-Topic publiziert
- Stelle: [src/mqttmanager.cpp](src/mqttmanager.cpp#L271)
- Befund: Timestamp schreibt erneut auf Status-Topic statt eigenes Topic zu nutzen.
- Risiko: Status `online/offline` wird ueberschrieben, UI/Automation koennen Zustand falsch interpretieren.
- Vorschlag: Eigenes Timestamp-Topic einfuehren, z. B. `/dht11/timestamp`.

5. MQTT-Logik iteriert ueber MAX statt tatsaechlich registrierte Zonen
- Stellen: [src/mqttmanager.cpp](src/mqttmanager.cpp#L34), [src/mqttmanager.cpp](src/mqttmanager.cpp#L71), [src/mqttmanager.cpp](src/mqttmanager.cpp#L183), [src/mqttmanager.cpp](src/mqttmanager.cpp#L228)
- Befund: Schleifen laufen bis `MAX_IRRIGATION_ZONES`, unabhaengig von `_numIrrigationZones`.
- Risiko: Nullpointer-Zugriffe, sobald Initialisierungspfad/Zone-Anzahl variiert.
- Vorschlag: Ausschliesslich `_numIrrigationZones` als Schleifengrenze verwenden.

### Mittel

6. OTA-Fortschrittsberechnung kann durch 0 teilen
- Stelle: [src/otamanager.cpp](src/otamanager.cpp#L130)
- Befund: `progress / (total / 100)` kann bei kleinen `total` problematisch sein.
- Risiko: Undefiniertes Verhalten/Fehler in seltenen OTA-Faellen.
- Vorschlag: `percentage = (total > 0) ? (progress * 100U) / total : 0;`.

7. Inkonsistente Dateinamen-Gross/Kleinschreibung in Includes
- Stellen: [src/irrigationZone.h](src/irrigationZone.h#L4), [src/wifimanager.h](src/wifimanager.h#L5), [src/globaldefines.h](src/globaldefines.h#L13), [src/irrigationZone.cpp](src/irrigationZone.cpp#L1), [src/otamanager.cpp](src/otamanager.cpp#L2)
- Befund: Mischung aus `globaldefines.h/globalDefines.h`, `Trace.h/trace.h`, `irrigationZone.h/IrrigationZone.h`.
- Risiko: Build-Fehler auf case-sensitive Systemen (Linux CI, Container).
- Vorschlag: Ein konsistentes Naming-Schema festlegen und Includes vereinheitlichen.

8. WLAN-Connect-Pfad ist event-getrieben ohne explizites `WiFi.begin` in setup
- Stellen: [src/wifimanager.cpp](src/wifimanager.cpp#L11), [src/wifimanager.cpp](src/wifimanager.cpp#L35)
- Befund: Initialer Connect startet indirekt ueber Event `SYSTEM_EVENT_STA_START`.
- Risiko: Fragiles Verhalten bei Event-Reihenfolge oder API-Aenderungen.
- Vorschlag: Initiales `WiFi.begin` nach Setup explizit triggern und State-Machine klar halten.

### Niedrig

9. Tote/ungenutzte API und Felder
- Stellen: [src/mqttmanager.h](src/mqttmanager.h#L40), [src/mqttmanager.h](src/mqttmanager.h#L43), [src/wifimanager.h](src/wifimanager.h#L14), [src/wifimanager.h](src/wifimanager.h#L30), [src/wifimanager.h](src/wifimanager.h#L32)
- Befund: Deklarierte, aber nicht genutzte Elemente (`handleTopicForSwButton`, `_blockPublish`, `checkDnsResolution`, Counters).
- Risiko: Hoehere kognitive Last, schwerere Wartbarkeit.
- Vorschlag: Entfernen oder implementieren; Compiler-Warnungen fuer ungenutzte Symbole schaerfen.

10. Logging-Level `TRACE` hat kein Prefix-Mapping
- Stelle: [src/Trace.cpp](src/Trace.cpp#L13)
- Befund: `switch(level)` behandelt INFO/DEBUG/ERROR, aber nicht TRACE.
- Risiko: Inkonsistente Logs bei feinerem Tracing.
- Vorschlag: TRACE-Prefix plus `default`-Fall einfuehren.

11. Sehr wenig automatisierte Tests
- Stelle: [test/README](test/README)
- Befund: Aktuell praktisch keine Unit-Tests im Projekt.
- Risiko: Refactoring ohne Sicherheitsnetz, Regressionen in Timing- und MQTT-Logik.
- Vorschlag: Native Unit-Tests fuer Formatierung/Parser/State-Transitions und Host-Simulation fuer MQTT-Callbacks.

## Strategischer Plan fuer Refactoring/Aenderungen

### Phase 1 - Stabilisierung kritischer Laufzeitlogik (kurzfristig)
Ziel: Laufzeitfehler und falsche Zustandsmeldungen eliminieren.

Massnahmen:
1. Watchdog-Timeout-Berechnung korrigieren und mit constexpr-Helfer absichern.
2. Restzeitberechnung in `IrrigationZone` auf sichere unsigned-Arithmetik mit Clamping umstellen.
3. `MM:SS`-Formatierung robust via `snprintf` umsetzen.
4. DHT-Topics trennen: `status` vs `timestamp`.
5. MQTT-Schleifen auf `_numIrrigationZones` umstellen.

Abnahmekriterien:
- Keine funktionalen Regressionen im manuellen Smoke-Test.
- Korrekte Restzeitanzeige fuer Werte < 10 Sekunden.
- Status-Topic wird nicht mehr durch Timestamp ueberschrieben.

### Phase 2 - Robustheit und Portabilitaet (kurzfristig bis mittelfristig)
Ziel: Plattformunabhaengigkeit und resiliente Verbindungslogik.

Massnahmen:
1. Include-Casing im gesamten Projekt vereinheitlichen.
2. OTA-Prozentberechnung gegen Division durch 0 haerten.
3. WLAN-Verbindungsablauf als klare State-Machine mit explizitem initialem Connect umbauen.
4. Veraltete ESP32 Event-Konstanten pruefen und auf aktuelle APIs migrieren (falls noetig).

Abnahmekriterien:
- Build erfolgreich auf Windows und case-sensitive Umgebung.
- Keine instabilen Reconnect-Loops im Langzeittest.

### Phase 3 - Code Health und Entkopplung (mittelfristig)
Ziel: Wartbarkeit verbessern, Risiken bei Weiterentwicklung reduzieren.

Massnahmen:
1. Tote Felder/Methoden entfernen oder vollstaendig implementieren.
2. MQTT-Topic-Building und Publish/Subscribe-Pfade kapseln (kleine Topic-Utility).
3. Logging konsistent machen (TRACE/default).
4. Konfigurationswerte und Grenzen zentralisieren (z. B. valider Bereich fuer Duration).

Abnahmekriterien:
- Geringere zyklomatische Komplexitaet in `MqttManager` und `IrrigationZone`.
- Keine Compiler-Hinweise zu ungenutzten Symbolen.

### Phase 4 - Teststrategie aufbauen (mittelfristig)
Ziel: Sichere Iteration bei kuenftigen Features.

Massnahmen:
1. Native Unit-Tests fuer:
   - Restzeitberechnung und Zeitformatierung
   - Duration-Parsing/Validierung aus MQTT-Payload
   - Topic-Mapping
2. Component-Tests mit Fakes fuer `PubSubClient`, `WiFi`, `Preferences`.
3. Regressions-Checkliste fuer OTA/WiFi/MQTT-Szenarien in README dokumentieren.

Abnahmekriterien:
- Mindestens 10 zielgerichtete Tests fuer Kernlogik.
- Reproduzierbare Testausfuehrung lokal und in CI.

## Empfohlene Reihenfolge der Umsetzung
1. Phase 1 komplett
2. Phase 2 Punkte 1 und 2
3. Phase 3 Punkt 1
4. Phase 4 Basis-Testpaket
5. Rest aus Phase 2 und 3

## Offene Fragen fuer die Umsetzung
1. Soll die Anzahl Zonen zur Laufzeit konfigurierbar werden oder fest bei 9 bleiben?
2. Soll DHT11 produktiv aktiviert werden oder vorerst optional/deaktiviert bleiben?
3. Ist Linux-CI kurzfristig geplant? Dann hat Include-Casing hohe Prioritaet.

## MQTT auf Event-basiert umstellen

### Ist-Zustand
Der aktuelle Stand ist ein Hybrid:
1. Polling fuer Verbindung/Keepalive via `mqttManager.loop()`.
2. Event-Callback fuer eingehende Nachrichten via statischem MQTT-Callback.
3. Zyklisches Publishing in kurzen Intervallen, auch bei unveraenderten Werten.

Bewertung: stabil genug fuer kleine Last, aber nicht optimal bei Skalierung und fuer klare Verantwortlichkeiten.

### Zielarchitektur (Event-First)
Kernidee: Verbindungs- und Topic-Verarbeitung werden event-getrieben. Das Hauptloop bleibt leichtgewichtig und dient nur als Scheduler/Watchdog-Futter.

Bausteine:
1. `ConnectivityService`
- Reagiert auf WiFi-Events (connected/disconnected/ip).
- Triggert MQTT connect/disconnect ohne periodisches Polling.

2. `MqttSessionManager`
- Verwaltet Sessionzustand (`DISCONNECTED`, `CONNECTING`, `CONNECTED`, `BACKOFF`).
- Fuehrt Backoff-Strategie mit Jitter aus.
- Publiziert Last-Will/Online-Status konsistent.

3. `TopicRouter`
- Map von Topic-Praefix -> Handlerfunktion.
- Keine linearen Vollscans ueber alle Zonen fuer jedes eingehende Paket.

4. `PublishCoordinator`
- Event-getriebene Publish-Ausloeser (`zone_state_changed`, `duration_changed`, `telemetry_tick`).
- Change-only-Publishing fuer statusartige Topics.
- Rate-Limit fuer Telemetrie.

5. `Domain Events`
- Beispiele: `ZoneButtonPressed`, `ZoneTimerExpired`, `DurationUpdated`, `SensorUpdated`.
- MQTT ist nur ein Adapter, die Fachlogik bleibt in Domain-Schicht.

### Zustandsmodell (MqttSessionManager)
Zustaende:
1. `DISCONNECTED`
2. `CONNECTING`
3. `CONNECTED`
4. `BACKOFF`

Transitionen:
1. `DISCONNECTED` -> `CONNECTING` bei WiFi `GOT_IP`.
2. `CONNECTING` -> `CONNECTED` bei erfolgreichem MQTT-Connect.
3. `CONNECTING` -> `BACKOFF` bei Connect-Fehler.
4. `BACKOFF` -> `CONNECTING` nach Ablauf Backoff-Timer.
5. `CONNECTED` -> `DISCONNECTED` bei WiFi-Verlust.
6. `CONNECTED` -> `BACKOFF` bei MQTT-Disconnect.

Regeln:
1. Subscriptions nur beim Eintritt in `CONNECTED`.
2. Initial Publish nur beim Eintritt in `CONNECTED`.
3. Kein Publish in allen anderen Zustaenden.

### Vor- und Nachteile

Vorteile:
1. Weniger unnoetige CPU-Arbeit durch reduzierte Polling-Last.
2. Schnellere Reaktion auf Verbindungswechsel.
3. Bessere Trennbarkeit: Transportzustand vs Fachlogik.
4. Einfachere Skalierung bei mehr Zonen/Topics.
5. Leichter testbar, weil Events und Zustandsuebergaenge explizit sind.

Nachteile:
1. Hoehere konzeptionelle Komplexitaet.
2. Mehr Sorgfalt bei Reihenfolge/Race Conditions noetig.
3. Migrationsaufwand (insb. Topic-Routing und Publish-Entkopplung).
4. Bei aktueller Bibliothek bleibt ein minimaler `loop()`-Pump-Pfad meist weiterhin notwendig.

### Empfohlener Migrationspfad (inkrementell, ohne Big-Bang)

#### Schritt 1 - Sessionzustand isolieren
1. `MqttSessionManager` als kleine Klasse einfuehren.
2. Bestehendes Reconnect inkl. Retry-Intervall dorthin verschieben.
3. Verhalten unveraendert halten (funktionale Aequivalenz).

Abnahme:
1. Build gruen.
2. Connect/Disconnect-Verhalten unveraendert im Log.

#### Schritt 2 - Event-Trigger von WiFi koppeln
1. WiFi-Event `GOT_IP` triggert `requestConnect()`.
2. WiFi-Event `DISCONNECTED` triggert `forceDisconnect()`.
3. Polling in `main.cpp` reduzieren auf minimale Pump-Funktion.

Abnahme:
1. Reconnect erfolgt ohne zyklische Vollpruefung.
2. Kein MQTT-Connectversuch ohne WiFi.

#### Schritt 3 - TopicRouter einfuehren
1. Topic-zu-Handler Registry aufbauen.
2. Lineare Schleifen in Callback ersetzen.
3. Fuer Zonen dynamische Registrierung bei `addIrrigationZone`.

Abnahme:
1. SwButton/Duration Topics funktionieren unveraendert.
2. Weniger String-Vergleiche pro Nachricht.

#### Schritt 4 - PublishCoordinator und Change-only Publishing
1. Zustandsaenderungen als Events melden.
2. Periodisches Voll-Publishing reduzieren.
3. Heartbeat und Systemstatus getrennt takten.

Abnahme:
1. MQTT-Traffic sinkt messbar.
2. UI bleibt konsistent und aktuell.

#### Schritt 5 - Offline-Verhalten und LWT finalisieren
1. Last-Will Topic/Feldformat final festlegen.
2. Optional kleine Outbox fuer wichtige retained Updates.
3. Fehlerszenarien (Router down, WLAN flap) testen.

Abnahme:
1. Sauberer online/offline Status im Broker.
2. Keine inkonsistenten Zustandsbilder nach Reconnect.

### Konkrete Quick Wins ohne Architekturbruch
1. In MQTT-Schleifen `_numIrrigationZones` statt `MAX_IRRIGATION_ZONES` nutzen.
2. DHT Timestamp auf eigenes Topic legen.
3. Publish nur bei Wertaenderung fuer Relay/SwButton.
4. Retry-Intervall mit Exponential Backoff statt fix 5s.

### KPI fuer den Erfolg der Umstellung
1. Mittlere Reconnect-Zeit nach WLAN-Rueckkehr.
2. Anzahl Publish-Nachrichten pro Minute im Idle.
3. CPU-Zeit im Mainloop (oder Loop-Latenz).
4. Anzahl inkonsistenter Statusmeldungen nach Reconnect (soll 0 sein).
