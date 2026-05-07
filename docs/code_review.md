# Code Review

Datum: 2026-05-07
Repository: GardenIrrigationControl

## Kurzfazit

Das Projekt ist fuer ein Embedded-/ESP32-Projekt bereits gut strukturiert. Die
Trennung in `app`, `domain`, `infrastructure`, `services`, `contracts` und
Tests ist ein solider Anfang. Besonders positiv sind die Native-Tests, die
Dependency-Injection-Ansaetze und die Dokumentation der Hardware.

Die wichtigsten Verbesserungsfelder liegen bei langlebiger ESP32-Stabilitaet,
MQTT-Verantwortlichkeiten, echter Produktionslogik in Tests und dem Umgang mit
noch unfertigen Features.

## Testergebnis

Native-Testlauf:

```text
pio test -e native
127 test cases: 127 succeeded
```

Hinweis: Der erste Testlauf im Sandbox-Kontext scheiterte an PlatformIO-
Berechtigungen unter `C:\Users\ppfei\.platformio`. Nach Ausfuehrung ausserhalb
der Sandbox liefen alle Native-Tests erfolgreich.

## Findings

### 1. Geplante Bewaesserung ist noch nicht funktionsfaehig

Datei: `src/domain/scheduled_irrigation.cpp`

`ScheduledIrrigation::isScheduleTimeMatch()` gibt aktuell immer `false`
zurueck. Dadurch kann geplante Bewaesserung nie ausloesen, obwohl Klasse und
Tests bereits wie ein vorhandenes Feature wirken.

Empfehlung:

- Entweder das Feature klar als unfertig kennzeichnen und nicht als fertige
  Funktion behandeln.
- Oder einen echten Zeitprovider/NTP-Provider integrieren und
  `isScheduleTimeMatch()` produktionsreif implementieren.
- Tests sollten danach echte Trigger-Faelle gegen Produktionslogik abdecken.

### 2. MQTT-Payloads werden still auf 255 Zeichen gekuerzt

Datei: `src/infrastructure/mqttmanager.cpp`

In `instanceMqttCallback()` wird der Payload in einen festen Buffer kopiert.
Payloads ueber 255 Zeichen werden abgeschnitten, ohne dass der Command
abgelehnt oder als fehlerhaft markiert wird.

Fuer aktuelle Bool- und Duration-Kommandos ist das meist unkritisch. Sobald
spaeter JSON-Kommandos, Schedule-Konfigurationen oder komplexere Payloads per
MQTT kommen, kann daraus aber ein schwer zu findender Fehler entstehen.

Empfehlung:

- Ueberlange Payloads explizit erkennen.
- Command mit Logmeldung ablehnen statt gekuerzt weiterzuverarbeiten.
- Fuer kuenftige JSON-Kommandos feste Maximalgroessen definieren.

### 3. Viele temporaere `String`-Objekte im MQTT-Hotpath

Dateien:

- `src/infrastructure/mqttmanager.cpp`
- `src/infrastructure/mqttmanager.h`

Der MQTT-Callback erzeugt viele temporaere `String`-Objekte fuer Topic-Vergleiche,
Topic-Erzeugung, Logs und Publish-Payloads. Auf ESP32-Systemen kann das bei
langer Laufzeit Heap-Fragmentierung beguenstigen.

Empfehlung:

- MQTT-Topics beim Setup einmal in feste Buffer vorberechnen.
- Eine kleine `MqttTopicRegistry` oder einen `MqttTopicBuilder` einfuehren.
- Im Callback moeglichst mit `strcmp`, Prefix-Matching und festen Buffern
  arbeiten.
- `String` in seltenen Setup-/Logpfaden tolerieren, aber nicht im Hotpath.

### 4. ISR- und Mainloop-Zugriff sind nur mit `volatile` abgesichert

Dateien:

- `src/domain/irrigation_zone.cpp`
- `src/domain/irrigation_zone.h`

`_lastDebounceTime` und `_buttonEventPending` werden aus Interrupt-Service-
Routine und Mainloop genutzt. `volatile` verhindert Optimierungen, ist aber
keine vollstaendige Synchronisation.

Empfehlung:

- ISR minimal halten und nur ein Ereignis-Flag setzen.
- Debounce-Logik vollstaendig in der Mainloop ausfuehren.
- Alternativ kritische Abschnitte fuer gemeinsam genutzte Werte verwenden.
- Ggf. einen kleinen `ButtonDebouncer` als testbare Domain-Komponente
  extrahieren.

### 5. Wasserstand-Prozentwert wird unten begrenzt, aber nicht oben

Datei: `src/domain/waterlevelmanager.cpp`

Der berechnete Prozentwert wird auf mindestens 0 begrenzt, aber nicht auf 100.
Das ist fuer Overflow-Erkennung teilweise sinnvoll, vermischt aber Anzeige- und
Diagnosewert.

Empfehlung:

- Zwei Werte unterscheiden:
  - `rawPercent` fuer Diagnose, Sensorfehler und Overflow-Erkennung.
  - `displayPercent` fuer UI/MQTT-Anzeige, begrenzt auf `0..100`.
- MQTT-Topics entsprechend klar benennen oder dokumentieren.

### 6. OTA ist als Default-Environment aktiviert

Datei: `platformio.ini`

`default_envs` enthaelt USB und OTA. Gleichzeitig steht im OTA-Environment eine
konkrete IP-Adresse sowie ein Platzhalter fuer das Auth-Flag.

Risiko:

- Versehentliche OTA-Builds oder Uploads.
- Lokale IP-Adresse im versionierten Projekt.
- Auth-Konfiguration wirkt wie ein Platzhalter, ist aber Teil der aktiven
  Projektkonfiguration.

Empfehlung:

- OTA aus `default_envs` entfernen.
- OTA-Upload nur bewusst mit `pio run -e az-delivery-devkit-v4-ota -t upload`
  starten.
- IP/Auth aus lokaler Konfiguration oder Secrets beziehen.
- Beispielwerte in eine dokumentierte Template-Datei verschieben.

## Design-Tipps

### MQTT-Verantwortlichkeiten aufteilen

`MqttManager` uebernimmt derzeit viele Aufgaben:

- MQTT-Verbindung
- Reconnect-Logik
- Topic-Erzeugung
- Command-Parsing
- Factory-Reset
- Duration-Reset
- State-Publishing
- LWT/Systemstatus

Empfohlene Aufteilung:

- `MqttConnectionManager` fuer Verbindung und Reconnect.
- `MqttTopicBuilder` oder `MqttTopicRegistry` fuer Topics.
- `IrrigationCommandHandler` fuer Zonen-Kommandos.
- `SystemCommandHandler` fuer Reset/Systembefehle.
- `MqttStatePublisher` fuer Status- und Sensor-Publishes.

Das macht die Klasse kleiner, reduziert Kopplung und erleichtert gezielte Tests.

### Domain weiter von Infrastruktur trennen

Die Richtung mit `contracts/` ist gut. Einige Domain-Klassen greifen aber noch
direkt auf Infrastruktur zu, zum Beispiel `IrrigationZone` auf
`StorageManager::getInstance()`.

Empfehlung:

- Interface wie `IZoneSettingsStore` einfuehren.
- Storage ueber Konstruktor oder Setup injizieren.
- Domain-Logik ohne NVS/Preferences testbar machen.

### Tests naeher an Produktionslogik bringen

Einige Tests kopieren Produktionslogik oder testen eher Checklisten als echtes
Verhalten. Dadurch koennen Tests gruen sein, obwohl sich die echte
Implementierung anders verhaelt.

Empfehlung:

- Reine Funktionen aus Produktionscode extrahieren und direkt testen.
- Kopierte Test-Hilfsfunktionen schrittweise entfernen.
- Checklist-/Dokumentations-Tests in echte Verhaltens-Tests umwandeln.
- Scheduled-Irrigation-Tests erst dann als Feature-Test werten, wenn die
  Zeitlogik wirklich implementiert ist.

### Einheitliche Sprache und Benennung

Im Code sind Begriffe wie `relais` und `relay` gemischt. Dazu kommen deutsche
und englische Kommentare bzw. Bezeichner.

Empfehlung:

- Code-Bezeichner einheitlich auf Englisch halten.
- Hardware-Dokumentation im README kann weiterhin Deutsch bleiben.
- Begriffe wie `durationTime` praezisieren, zum Beispiel `durationMs` oder
  `durationSeconds`.

### Konfiguration klarer schichten

Aktuell liegen Defaults, Hardware-Pins, Wasserlevel-Konfiguration, OTA-Schalter
und Timing-Werte eng zusammen.

Empfehlung:

- Hardware-Pins in `hardwareconfig`.
- Laufzeit-/Loop-Parameter in `systemconfig`.
- Sensorwerte in `waterlevelconfig`.
- MQTT-/OTA-/WiFi-Werte konsequent ueber lokale Secrets oder Templates.
- README mit einer kurzen Konfigurationsmatrix ergaenzen.

## Positives

- Gute Native-Testbasis mit erfolgreichem Testlauf.
- Klare Projektstruktur mit begonnener Schichtenarchitektur.
- Hardware-Dokumentation ist ausfuehrlich und praxisnah.
- Relais-Startzustand wird fuer LOW-active Ausgaenge bewusst abgesichert.
- MQTT-LWT und Systemstatus zeigen, dass Produktionsbetrieb mitgedacht wurde.
- Wasserstand-Logik nutzt Hysterese und trennt Sensorlesen, Transitionen und
  Publishing schon recht sauber.

## Priorisierte naechste Schritte

1. `ScheduledIrrigation` entweder fertig implementieren oder klar deaktivieren.
2. MQTT-Payload-Laengen validieren und ueberlange Nachrichten ablehnen.
3. OTA aus den Default-Environments entfernen.
4. ISR-/Button-Debounce robuster gestalten.
5. MQTT-Topic-Erzeugung aus dem Hotpath herausziehen.
6. Tests bereinigen, die Produktionslogik kopieren statt sie zu testen.
7. Storage-Abhaengigkeit aus der Domain ueber ein Interface entkoppeln.
