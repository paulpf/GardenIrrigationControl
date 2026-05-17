# Scheduler MQTT Integration Guide

Stand: 2026-05-17

Diese Anleitung beschreibt, wie ioBroker und Home Assistant fuer das kommende
Scheduler-Feature vorbereitet werden koennen. Sie enthaelt bewusst keine
persoenlichen Daten, lokalen IP-Adressen, Zugangsdaten oder Secrets.

Diese Anleitung darf nur Fakten, verifizierte Annahmen und real umsetzbare
Vorschlaege enthalten. Wenn sich Firmware-Topics, Bedienlogik, Persistenz,
Retain-Verhalten oder Tests aendern, muss diese Datei bei Bedarf mit
aktualisiert werden.

## Ziel

Der ESP32 soll pro Bewaesserungszone mehrere Zeitplaene per MQTT anbieten.
ioBroker und Home Assistant sollen diese Plaene anzeigen, aendern, aktivieren,
deaktivieren und loeschen koennen.

Ein Plan besteht aus:

- `configured`: Plan existiert
- `enabled`: Plan ist aktiv
- `hour`: Startstunde, `0..23`
- `minute`: Startminute, `0..59`
- `durationMinutes`: Laufzeit in Minuten
- `daysOfWeek`: Wochentage als Bitmaske

## MQTT-Grundprinzip

Es gibt getrennte Topics fuer Status und Befehle.

Status:

```text
<device>/irrigationZone1/schedule1/enabled
```

Befehl:

```text
<device>/irrigationZone1/schedule1/enabled/set
```

Das Geraet publisht Status-Topics. ioBroker oder Home Assistant schreiben auf
die `/set` Topics. Danach publisht das Geraet den aktualisierten Status zurueck.

## Empfohlene Basisstruktur

Platzhalter:

- `<device>`: MQTT-Basisname des Controllers
- `irrigationZone1`: Zone 1
- `schedule1`: erster Plan-Slot der Zone

Beispiel fuer Zone 1, Plan 1:

```text
<device>/irrigationZone1/schedule1/configured
<device>/irrigationZone1/schedule1/enabled
<device>/irrigationZone1/schedule1/hour
<device>/irrigationZone1/schedule1/minute
<device>/irrigationZone1/schedule1/durationMinutes
<device>/irrigationZone1/schedule1/daysOfWeek
```

Befehle:

```text
<device>/irrigationZone1/schedule1/configured/set
<device>/irrigationZone1/schedule1/enabled/set
<device>/irrigationZone1/schedule1/hour/set
<device>/irrigationZone1/schedule1/minute/set
<device>/irrigationZone1/schedule1/durationMinutes/set
<device>/irrigationZone1/schedule1/daysOfWeek/set
<device>/irrigationZone1/schedule1/delete/set
```

## Wochentage

Intern wird eine Bitmaske verwendet:

| Tag | Wert |
| --- | ---: |
| Montag | 1 |
| Dienstag | 2 |
| Mittwoch | 4 |
| Donnerstag | 8 |
| Freitag | 16 |
| Samstag | 32 |
| Sonntag | 64 |

Beispiele:

| Bedeutung | Wert |
| --- | ---: |
| Mittwoch und Freitag | 20 |
| Montag bis Freitag | 31 |
| Samstag und Sonntag | 96 |
| Jeden Tag | 127 |

## Bedienlogik

### Plan anlegen

Minimaler Ablauf:

```text
<device>/irrigationZone1/schedule1/configured/set = true
<device>/irrigationZone1/schedule1/hour/set = 6
<device>/irrigationZone1/schedule1/minute/set = 30
<device>/irrigationZone1/schedule1/durationMinutes/set = 12
<device>/irrigationZone1/schedule1/daysOfWeek/set = 20
<device>/irrigationZone1/schedule1/enabled/set = true
```

Das Beispiel bedeutet: Zone 1, Plan 1, Mittwoch und Freitag um 06:30 fuer
12 Minuten.

### Plan deaktivieren

```text
<device>/irrigationZone1/schedule1/enabled/set = false
```

Der Plan bleibt gespeichert, startet aber nicht.

### Plan wieder aktivieren

```text
<device>/irrigationZone1/schedule1/enabled/set = true
```

### Plan loeschen

```text
<device>/irrigationZone1/schedule1/delete/set = true
```

Erwartetes Verhalten:

- Der Plan wird aus dem ESP32-Speicher geloescht.
- Der Plan wird aus der Persistenz geloescht oder als nicht konfiguriert
  markiert.
- `configured` wird auf `false` publiziert.
- Die restlichen Felder werden als Defaults oder leerer Slot publiziert.
- Andere Schedule-Slots behalten ihre Nummer. Wenn `schedule1` geloescht wird,
  bleibt `schedule2` weiterhin `schedule2`.

## Vorbereitung in ioBroker

### Variante A: MQTT-Adapter erzeugt States automatisch

Das ist fuer den Start am einfachsten.

1. MQTT-Adapter in ioBroker als Client oder Broker passend zur vorhandenen
   Infrastruktur einrichten.
2. Subscribe-Pattern auf den Controller-Topicbaum setzen, zum Beispiel:

   ```text
   <device>/#
   ```

3. Controller starten oder Status neu publizieren lassen.
4. ioBroker sollte die empfangenen Topics als States anlegen.
5. Fuer schreibbare Werte die passenden `/set` Topics verwenden.

Empfohlene ioBroker-Objekte oder Alias-Struktur:

```text
Garten.Bewaesserung.Zone1.Schedule1.configured
Garten.Bewaesserung.Zone1.Schedule1.enabled
Garten.Bewaesserung.Zone1.Schedule1.hour
Garten.Bewaesserung.Zone1.Schedule1.minute
Garten.Bewaesserung.Zone1.Schedule1.durationMinutes
Garten.Bewaesserung.Zone1.Schedule1.daysOfWeek
Garten.Bewaesserung.Zone1.Schedule1.delete
```

Diese Namen sind nur ein Vorschlag fuer Aliase. Die echten MQTT-Topics bleiben
technisch stabil.

### Variante B: Eigene ioBroker-Aliase vorbereiten

Fuer eine schoenere Visualisierung koennen Aliase angelegt werden:

- `enabled`: Boolean/Switch
- `hour`: Number, Min 0, Max 23, Step 1
- `minute`: Number, Min 0, Max 59, Step 1
- `durationMinutes`: Number, Min 1, Max 60, Step 1
- `daysOfWeek`: Number, Min 1, Max 127, Step 1
- `delete`: Button oder Boolean, der nach Ausfuehrung wieder zurueckgesetzt
  wird

Wichtig:

- Anzeigen sollten Status-Topics lesen.
- Bedienelemente sollten `/set` Topics schreiben.
- Nicht direkt auf Status-Topics schreiben.

### ioBroker-Visualisierung vorbereiten

Pro Schedule-Slot bietet sich eine Zeile oder Karte an:

```text
Zone 1 | Plan 1 | aktiv | 06:30 | 12 min | Mi+Fr | Loeschen
```

Fuer die erste Version reicht `daysOfWeek` als Zahl. Komfortabler waere spaeter
eine ioBroker-Logik, die Checkboxen fuer Montag bis Sonntag in die Bitmaske
umrechnet.

## Vorbereitung in Home Assistant

Home Assistant kann MQTT-Entitaeten entweder manuell per YAML oder automatisch
ueber MQTT Discovery bekommen. Fuer das Firmware-Feature ist Discovery
langfristig die angenehmere Loesung.

### Manuelle YAML-Vorbereitung

Beispiel fuer `enabled`:

```yaml
mqtt:
  switch:
    - name: "Irrigation Zone 1 Schedule 1 Enabled"
      unique_id: "irrigation_zone1_schedule1_enabled"
      state_topic: "<device>/irrigationZone1/schedule1/enabled"
      command_topic: "<device>/irrigationZone1/schedule1/enabled/set"
      payload_on: "true"
      payload_off: "false"
      state_on: "true"
      state_off: "false"
```

Beispiel fuer Stunde:

```yaml
mqtt:
  number:
    - name: "Irrigation Zone 1 Schedule 1 Hour"
      unique_id: "irrigation_zone1_schedule1_hour"
      state_topic: "<device>/irrigationZone1/schedule1/hour"
      command_topic: "<device>/irrigationZone1/schedule1/hour/set"
      min: 0
      max: 23
      step: 1
```

Weitere `number` Entitaeten:

- `minute`, Min 0, Max 59
- `durationMinutes`, Min 1, Max 60
- `daysOfWeek`, Min 1, Max 127

Beispiel fuer Delete als Button:

```yaml
mqtt:
  button:
    - name: "Irrigation Zone 1 Schedule 1 Delete"
      unique_id: "irrigation_zone1_schedule1_delete"
      command_topic: "<device>/irrigationZone1/schedule1/delete/set"
      payload_press: "true"
```

### Home Assistant Discovery als Ziel

Spaeter kann der ESP32 fuer jeden Schedule-Slot Discovery-Konfigurationen
publizieren. Dann muss Home Assistant nicht manuell per YAML gepflegt werden.

Discovery-Entitaeten:

- Switch: `enabled`
- Number: `hour`
- Number: `minute`
- Number: `durationMinutes`
- Number: `daysOfWeek`
- Button: `delete`

Discovery sollte stabile `unique_id` Werte verwenden, zum Beispiel:

```text
<device>_zone1_schedule1_enabled
<device>_zone1_schedule1_hour
```

Keine Seriennummern, IP-Adressen oder Secrets in Discovery-Payloads schreiben.

## Retain-Verhalten

Empfehlung:

- Status-Topics retained publishen.
- Command-Topics nicht retained verwenden.
- Leere retained Command-Topics vermeiden oder gezielt nach Verarbeitung
  loeschen.

Warum:

- ioBroker und Home Assistant sehen nach Neustart sofort den letzten Status.
- Alte Befehle werden nach Reconnect nicht versehentlich erneut ausgefuehrt.

## Validierung mit MQTT-Client

Vor ioBroker/Home Assistant kann das Verhalten mit einem generischen MQTT-Client
geprueft werden.

Subscribe:

```bash
mosquitto_sub -h <broker-host> -t '<device>/irrigationZone1/schedule1/#' -v
```

Plan anlegen:

```bash
mosquitto_pub -h <broker-host> -t '<device>/irrigationZone1/schedule1/configured/set' -m 'true'
mosquitto_pub -h <broker-host> -t '<device>/irrigationZone1/schedule1/hour/set' -m '6'
mosquitto_pub -h <broker-host> -t '<device>/irrigationZone1/schedule1/minute/set' -m '30'
mosquitto_pub -h <broker-host> -t '<device>/irrigationZone1/schedule1/durationMinutes/set' -m '12'
mosquitto_pub -h <broker-host> -t '<device>/irrigationZone1/schedule1/daysOfWeek/set' -m '20'
mosquitto_pub -h <broker-host> -t '<device>/irrigationZone1/schedule1/enabled/set' -m 'true'
```

Plan loeschen:

```bash
mosquitto_pub -h <broker-host> -t '<device>/irrigationZone1/schedule1/delete/set' -m 'true'
```

`<broker-host>` und `<device>` sind Platzhalter und muessen lokal ersetzt
werden. Keine echten Werte in die Doku eintragen.

## Empfohlene erste Ausbaustufe

Fuer die erste stabile Version:

- 4 Schedule-Slots pro Zone
- Einzelwert-Topics wie oben
- `daysOfWeek` als Bitmaske
- Persistenz im ESP32
- Retained Status-Topics
- Nicht-retained Command-Topics
- ioBroker kompatibel ohne Discovery
- Home Assistant kompatibel ueber manuelle YAML-Konfiguration

Danach als Komfortausbau:

- Home Assistant MQTT Discovery
- zusaetzliche Wochentag-Boolean-Topics
- lesbarer `daysText` Status, z. B. `Mi,Fr`
- optional ein JSON-Summary pro Zone fuer Visualisierungen

## Quellen

- Home Assistant MQTT Integration:
  <https://www.home-assistant.io/integrations/mqtt/>
- ioBroker MQTT Adapter:
  <https://github.com/ioBroker/ioBroker.mqtt>
