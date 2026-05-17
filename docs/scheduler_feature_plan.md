# Scheduler Feature Plan

Stand: 2026-05-17

## Zielbild

Der Scheduler ist ein Feature, mit dem pro Bewaesserungszone mehrere
Startzeiten konfiguriert werden koennen. Jeder Eintrag kann fuer bestimmte
Wochentage gelten, eine eigene Laufzeit haben, dauerhaft gespeichert werden
und nach einem Neustart wieder aktiv sein.

Plaene muessen wie bei einer professionellen Bewaesserungssteuerung verwaltet
werden koennen:

- neuen Plan fuer eine Zone hinzufuegen
- bestehenden Plan aendern
- bestehenden Plan loeschen
- Plan aktivieren/deaktivieren, ohne ihn zu loeschen
- mehrere Plaene pro Zone parallel verwalten
- geloeschte Plaene duerfen nach Neustart nicht wieder auftauchen

Beispiel:

- Zone 1: Mittwoch und Freitag um 06:30 fuer 12 Minuten
- Zone 1: Sonntag um 20:00 fuer 5 Minuten
- Zone 3: Montag bis Freitag um 05:45 fuer 8 Minuten

Die Steuerung startet die passende Zone automatisch, wenn lokale NTP-Zeit,
Wochentag und Uhrzeit passen.

## Wichtige Arbeitsregeln

- Nichts committen, bevor der Benutzer manuell getestet und den Commit
  explizit freigegeben hat.
- Keine externen Libraries, Vendor-Dateien oder `test/support`-Stubs anfassen,
  ausser der Benutzer fordert das ausdruecklich.
- PlatformIO-Tests und Builds ausserhalb der Sandbox ausfuehren.
- Bei Firmware-Aenderungen immer beide ESP32-Builds pruefen:
  - `pio run -e az-delivery-devkit-v4-usb`
  - `pio run -e az-delivery-devkit-v4-ota`
- Bei Testfehlern nicht den Scope ausweiten, sondern Fehler lokalisieren und
  melden.
- Keine persoenlichen Daten, Secrets, lokalen IP-Adressen, Zugangsdaten oder
  sonstige sensiblen Informationen in diese oder andere Dokus schreiben.
- Keine Halluzinationen: Nur Fakten, verifizierte Annahmen und real
  umsetzbare Vorschlaege dokumentieren oder empfehlen.
- Jeden sauberen, vom Benutzer manuell getesteten Schritt committen, bevor mit
  dem naechsten Umsetzungsschritt begonnen wird.
- Kein Commit ohne explizite Freigabe nach manuellem Test.
- Den Benutzer immer explizit informieren, wann ein Code-Review sinnvoll ist
  und wann ein Upload-/Hardwaretest durchgefuehrt werden soll.
- Nach erfolgreicher automatischer Validierung stoppen und auf Review,
  Upload-/Hardwaretest und Commit-Freigabe warten.
- Die ioBroker/Home-Assistant-Anleitung
  `docs/scheduler_mqtt_integration_guide.md` bei jedem relevanten
  Scheduler-Schritt mitpflegen, wenn sich Topics, Bedienlogik, Persistenz,
  Retain-Verhalten oder Tests aendern.

## Aktueller vorbereiteter Stand

Bereits vorhanden:

- `IrrigationSchedule` in `src/domain/scheduled_irrigation.h`
  - `enabled`
  - `hour`
  - `minute`
  - `durationMinutes`
  - `daysOfWeek` als Bitmaske, bit0=Montag bis bit6=Sonntag
- `ScheduledIrrigation`
  - verwaltet bis zu `MAX_SCHEDULES_PER_ZONE` feste Plan-Slots pro Zone
  - unterscheidet konfigurierte und nicht konfigurierte Slots
  - prueft Zeit und Wochentag ueber `IWallClock`
  - verhindert Doppeltrigger innerhalb derselben Wanduhren-Minute
  - `consumeDueSchedule()` liefert den faelligen Schedule
- `NtpWallClock` und `IWallClock` sind angebunden.
- `GardenControllerApp::initScheduledIrrigation()` initialisiert pro Zone einen
  Scheduler mit Wall Clock.
- `AutomaticIrrigationController` dient als Sicherheits-/Freigabe-Gate fuer
  automatische Scheduler-Laeufe.
- `ScheduledIrrigationPublisher` publisht Schedule-State fuer ioBroker-artige
  Topics.
- `StorageManager` hat Schedule-Methoden fuer Speichern, Laden, Loeschen und
  configured-Abfrage.

Aktuell vorbereitet:

- `GardenControllerApp::handleScheduledIrrigation()` wertet faellige Schedules
  aus und startet die passende Zone.
- `IrrigationZone::startForDuration(int durationTime)` startet eine Zone fuer
  eine temporaere aktive Timer-Laufzeit, ohne die manuelle Zonendauer
  `_durationTime` zu veraendern oder als neue Default-Dauer zu speichern.
- Test in `test/test_native_irrigation_zone_relay_output/test_main.cpp`, dass
  `startForDuration()` die Zone startet und nicht persistiert.
- `.github/copilot-instructions.md` wurde um die Arbeitsregeln ergaenzt.
- Nicht konfigurierte Schedule-Slots bleiben als Locher erhalten; ein Plan in
  `schedule2` wird nach Loeschen von `schedule1` nicht zu `schedule1`.
- `GardenControllerApp::initScheduledIrrigation()` laedt konfigurierte
  Schedule-Slots aus `StorageManager`.

Zuletzt validiert, ausserhalb der Sandbox:

- `pio test -e native`: 143 Tests bestanden
- `pio run -e az-delivery-devkit-v4-usb`: erfolgreich
- `pio run -e az-delivery-devkit-v4-ota`: erfolgreich

## Fehlende Teile fuer das vollstaendige Feature

1. Persistenz der Schedule-Konfiguration
   - Schedules werden in NVS/Preferences gespeichert.
   - Nach Neustart werden konfigurierte Slots in `initScheduledIrrigation()`
     geladen.
   - Geloeschte Schedules werden als nicht konfiguriert markiert.

2. MQTT-/ioBroker-Konfiguration
   - Schedule-Werte sind pro Zone und Schedule-Slot per MQTT setzbar.
   - Schedules koennen per MQTT hinzugefuegt und geloescht werden.
   - Ungueltige Payloads duerfen bestehende Werte nicht veraendern.
   - Nach Aenderung soll der aktuelle State publiziert werden.

3. Saubere Verantwortlichkeiten
   - `ScheduledIrrigation` bleibt Domain-Logik ohne MQTT/Storage.
   - Storage kommt ueber `StorageManager` oder einen kleinen Schedule-Store.
   - MQTT-Routing sollte `MqttManager` nicht weiter unkontrolliert aufblaehen.

## Vorgeschlagene Topic-Struktur

State Topics:

```text
<device>/irrigationZone1/schedule1/configured
<device>/irrigationZone1/schedule1/enabled
<device>/irrigationZone1/schedule1/hour
<device>/irrigationZone1/schedule1/minute
<device>/irrigationZone1/schedule1/durationMinutes
<device>/irrigationZone1/schedule1/daysOfWeek
```

Command Topics:

```text
<device>/irrigationZone1/schedule1/configured/set
<device>/irrigationZone1/schedule1/enabled/set
<device>/irrigationZone1/schedule1/hour/set
<device>/irrigationZone1/schedule1/minute/set
<device>/irrigationZone1/schedule1/durationMinutes/set
<device>/irrigationZone1/schedule1/daysOfWeek/set
<device>/irrigationZone1/schedule1/delete/set
```

Semantik:

- `configured/set=true` legt einen Plan im Slot an, falls er noch nicht
  existiert, und speichert Default-Werte.
- `configured/set=false` loescht den Plan im Slot wie `delete/set=true`.
- `delete/set=true` loescht den Plan im Slot, entfernt ihn aus der Persistenz
  und publisht den Slot als nicht konfiguriert.
- `enabled/set=false` deaktiviert den Plan nur; er bleibt gespeichert.
- Feld-Updates auf nicht konfigurierte Slots legen den Slot automatisch mit
  Default-Werten an und aktualisieren danach das angefragte Feld.

`daysOfWeek` bleibt zunaechst eine Bitmaske:

- Montag: `1`
- Dienstag: `2`
- Mittwoch: `4`
- Donnerstag: `8`
- Freitag: `16`
- Samstag: `32`
- Sonntag: `64`
- Mittwoch + Freitag: `20`
- Montag bis Freitag: `31`

## Design-Entscheidungen

### Schedule-Dauer ist nicht die Zonendefault-Dauer

Ein Schedule-Lauf darf die manuell konfigurierte Default-Dauer einer Zone nicht
ueberschreiben. Deshalb ist ein transienter Start sinnvoll:

```cpp
_irrigationZones[i].startForDuration(durationMs);
```

Diese Methode setzt die Laufzeit nur fuer den aktuellen Lauf und speichert sie
nicht als neue Default-Dauer. Technisch nutzt die Zone dafuer eine separate
aktive Timer-Dauer. Der MQTT-State `durationTime` bleibt die manuelle
Zonendauer und wird durch Schedule-Laeufe nicht veraendert.

### `ScheduledIrrigation::loop()` sollte nicht die Zone starten

Die Domain-Klasse kennt keine Zone und keine MQTT-Publisher. Der Start gehoert
in `GardenControllerApp::handleScheduledIrrigation()`, weil dort Scheduler,
Zone, Config, MQTT-Events und Safety-Gates zusammenkommen.

### Persistenz zuerst, MQTT danach

Ohne Persistenz ist das Feature nicht vollstaendig. Deshalb zuerst Storage und
Load-on-setup bauen, danach MQTT-Commands.

## Umsetzungsplan

### Phase 1: Aktuellen uncommitted Stand pruefen und bereinigen

- Diff anschauen:
  - `.github/copilot-instructions.md`
  - `src/app/gardencontrollerapp.cpp`
  - `src/domain/irrigation_zone.cpp`
  - `src/domain/irrigation_zone.h`
  - `test/test_native_irrigation_zone_relay_output/test_main.cpp`
- Sicherstellen, dass `startForDuration()` keine Persistenz ausloest.
- Sicherstellen, dass faellige Schedules nur starten, wenn:
  - `ENABLE_SCHEDULED_IRRIGATION == true`
  - `AutomaticIrrigationController::canRunSchedules() == true`
  - Dauer > 0 und <= `maxDurationMs`

### Phase 2: Schedule-Persistenz

- `StorageManager` um Schedule-Methoden erweitern: erledigt
  - `saveSchedule(int zoneIndex, int scheduleIndex, const IrrigationSchedule &schedule)`
  - `loadSchedule(int zoneIndex, int scheduleIndex, IrrigationSchedule &schedule)`
  - `clearSchedule(int zoneIndex, int scheduleIndex)`
  - `isScheduleConfigured(int zoneIndex, int scheduleIndex)`
- Pro Schedule speichern:
  - configured
  - enabled
  - hour
  - minute
  - durationMinutes
  - daysOfWeek
- `GardenControllerApp::initScheduledIrrigation()` laedt alle gespeicherten
  Schedules pro Zone: erledigt
- Geloeschte oder nicht konfigurierte Slots werden nicht in
  `ScheduledIrrigation` aufgenommen: erledigt
- Native-Tests fuer feste Slot-Semantik und Publisher-Locher sind ergaenzt.

### Phase 3: MQTT Command Handling

- `ScheduledIrrigationCommandHandler` eingefuehrt, damit `MqttManager` nicht
  weiter mit Schedule-Details waechst.
- `/set` Topics werden beim MQTT-Reconnect subscribt.
- Payloads werden validiert:
  - `configured`: bool
  - `delete`: bool, nur `true` loest Loeschung aus
  - `enabled`: bool (`true/false`, `1/0`, `on/off`)
  - `hour`: 0..23
  - `minute`: 0..59
  - `durationMinutes`: 1..maxDurationMs/60000
  - `daysOfWeek`: 1..127
- Nach erfolgreicher Aenderung:
  - Schedule im Domain-Objekt aktualisieren
  - Schedule speichern
  - Schedule-State publizieren
- Nach erfolgreicher Loeschung:
  - Schedule aus Domain-Objekt entfernen oder Slot als unconfigured behandeln
  - Schedule aus Storage entfernen
  - Schedule-State als unconfigured/default publizieren

### Phase 4: State Publishing vervollstaendigen

- `ScheduledIrrigationPublisher` pruefen:
  - publisht alle Slots
  - publisht unconfigured Slots als Defaults
  - throttling bleibt erhalten
- Bei Command-Aenderungen unmittelbares Publish ermoeglichen.

### Phase 5: Integration und manuelle Testvorbereitung

- Native Tests ausserhalb der Sandbox:
  - `pio test -e native`
- Firmware Builds ausserhalb der Sandbox:
  - `pio run -e az-delivery-devkit-v4-usb`
  - `pio run -e az-delivery-devkit-v4-ota`
- Pruefen, ob `docs/scheduler_mqtt_integration_guide.md` aktualisiert werden
  muss.
- Benutzer zum Code-Review auffordern.
- Benutzer explizit informieren, ob ein Upload-/Hardwaretest sinnvoll oder
  notwendig ist.
- Benutzer testet manuell auf Hardware/ioBroker.
- Nach erfolgreichem manuellem Test und expliziter Freigabe committen.
- Erst danach mit dem naechsten Umsetzungsschritt beginnen.

## Naechster Einstiegspunkt

Beim naechsten Weiterarbeiten hier starten:

1. `git status --short` pruefen.
2. Diese Datei lesen.
3. Aktuellen Diff gegen die Phase-1-Liste pruefen.
4. Wenn der aktuelle uncommitted Stand noch gewollt ist, mit Phase 2
   `Schedule-Persistenz` beginnen.
