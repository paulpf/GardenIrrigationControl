# Scheduler MQTT Integration Guide

Stand: 2026-05-17

Diese Anleitung beschreibt, wie ioBroker und Home Assistant fuer das kommende
Scheduler-Feature vorbereitet werden koennen. Sie enthaelt bewusst keine
persoenlichen Daten, lokalen IP-Adressen, Zugangsdaten oder Secrets.

Diese Anleitung darf nur Fakten, verifizierte Annahmen und real umsetzbare
Vorschlaege enthalten. Wenn sich Firmware-Topics, Bedienlogik, Persistenz,
Retain-Verhalten oder Tests aendern, muss diese Datei bei Bedarf mit
aktualisiert werden.

## Inhaltsverzeichnis

<!-- TOC -->

- [Ziel](#ziel)
- [MQTT-Grundprinzip](#mqtt-grundprinzip)
- [Empfohlene Basisstruktur](#empfohlene-basisstruktur)
- [Wochentage](#wochentage)
- [Bedienlogik](#bedienlogik)
  - [Plan anlegen](#plan-anlegen)
  - [Plan deaktivieren](#plan-deaktivieren)
  - [Plan wieder aktivieren](#plan-wieder-aktivieren)
  - [Plan loeschen](#plan-loeschen)
- [Vorbereitung in ioBroker](#vorbereitung-in-iobroker)
  - [Variante A: MQTT-Adapter erzeugt States automatisch](#variante-a-mqtt-adapter-erzeugt-states-automatisch)
  - [Variante B: Eigene ioBroker-Aliase vorbereiten](#variante-b-eigene-iobroker-aliase-vorbereiten)
  - [ioBroker-Visualisierung vorbereiten](#iobroker-visualisierung-vorbereiten)
  - [VIS-Konfiguration in ioBroker](#vis-konfiguration-in-iobroker)
- [Vorbereitung in Home Assistant](#vorbereitung-in-home-assistant)
  - [Manuelle YAML-Vorbereitung](#manuelle-yaml-vorbereitung)
  - [Home Assistant Discovery als Ziel](#home-assistant-discovery-als-ziel)
- [Retain-Verhalten](#retain-verhalten)
- [Validierung mit MQTT-Client](#validierung-mit-mqtt-client)
- [Empfohlene erste Ausbaustufe](#empfohlene-erste-ausbaustufe)
- [Quellen](#quellen)

<!-- /TOC -->

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

Damit ein gespeicherter und aktivierter Plan wirklich automatisch startet,
muessen zwei Bedingungen erfuellt sein:

- Das Scheduler-Feature ist in der Firmware aktiviert.
- Der globale Automatik-Schalter ist aktiv:
  `<device>/irrigation/automatic/enabled = true`.

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

Globale Automatik-Freigabe:

```text
Status: <device>/irrigation/automatic/enabled
Befehl: <device>/irrigation/automatic/enabled/set
```

Gueltige Payloads fuer den Befehl sind:

```text
true
false
1
0
on
off
```

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
<device>/irrigation/automatic/enabled/set = true
<device>/irrigationZone1/schedule1/configured/set = true
<device>/irrigationZone1/schedule1/hour/set = 6
<device>/irrigationZone1/schedule1/minute/set = 30
<device>/irrigationZone1/schedule1/durationMinutes/set = 12
<device>/irrigationZone1/schedule1/daysOfWeek/set = 20
<device>/irrigationZone1/schedule1/enabled/set = true
```

Das Beispiel bedeutet: Zone 1, Plan 1, Mittwoch und Freitag um 06:30 fuer
12 Minuten.

Der erste Befehl aktiviert die globale Automatik-Freigabe. Ohne diese Freigabe
bleibt der Plan gespeichert und aktiv, startet aber keinen Bewaesserungslauf.

Ein Feld-Update auf einen noch nicht konfigurierten Slot legt den Slot
automatisch mit Default-Werten an und aktualisiert danach das Feld. Dadurch kann
ioBroker z. B. direkt `hour/set` schreiben, ohne vorher zwingend
`configured/set=true` zu senden.

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

Alternativ:

```text
<device>/irrigationZone1/schedule1/configured/set = false
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
Garten.Bewaesserung.Automatik.enabled
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

- `Automatik.enabled`: Boolean/Switch
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

Empfohlene VIS-Struktur:

- Die Hauptseite bleibt schlank und zeigt pro Zone nur eine kompakte
  Schedule-Zusammenfassung.
- Die eigentliche Konfiguration erfolgt ueber ein gemeinsames Popup.
- Das Popup wird fuer jede Zone wiederverwendet und bekommt nur die gewaehlte
  Zone als Kontext.
- Die Firmware bleibt bei stabilen Einzelwert-Topics. Anzeige-Strings fuer VIS
  werden in ioBroker aus den vorhandenen States berechnet.

Beispiel fuer die Hauptseite:

```text
Erdbeeren          Mi,Fr 06:30 12m | So 20:00 5m
Himbeeren          -
Blumenbeete        Mo-Fr 05:45 8m
```

Beispiel fuer ein gemeinsames Popup:

```text
Zone: Erdbeeren

[Automatik aktiv]

Plan 1  [aktiv]  [06] [:] [30]  [12 min]  [Mo] [Di] [Mi] [Do] [Fr] [Sa] [So]  [Loeschen]
Plan 2  [aktiv]  [20] [:] [00]  [ 5 min]  [Mo] [Di] [Mi] [Do] [Fr] [Sa] [So]  [Loeschen]

[+ Plan]
```

Die Checkboxen fuer Montag bis Sonntag schreiben nicht direkt eigene
Firmware-Topics. Sie berechnen in ioBroker die `daysOfWeek`-Bitmaske und
schreiben den Summenwert nach:

```text
<device>/irrigationZoneX/scheduleY/daysOfWeek/set
```

Fuer eine erste Version ist auch ein Select- oder Number-Widget fuer
`daysOfWeek` ausreichend. Komfortabler ist die Checkbox-Variante im Popup.

#### ioBroker-seitige Summary-States

Fuer die Hauptseite sollte ioBroker pro Zone einen zusaetzlichen Anzeige-State
erzeugen, z. B. als Alias oder eigener JavaScript-State:

```text
Garten.Bewaesserung.Zone1.scheduleSummary
Garten.Bewaesserung.Zone2.scheduleSummary
...
```

Dieser State ist nur Darstellung und wird aus den MQTT-Status-States berechnet.
Er wird nicht an den ESP32 zurueckgeschrieben.

Beispiele fuer den String:

```text
Mi,Fr 06:30 12m
Mi,Fr 06:30 12m | So 20:00 5m
-
```

Empfohlene Regeln fuer den Summary-String:

- Nur `configured=true` Plaene anzeigen.
- `enabled=false` Plaene entweder ausblenden oder mit `aus` markieren.
- Zeit immer zweistellig darstellen, z. B. `06:05`.
- Wochentage lesbar darstellen: `Mo`, `Di`, `Mi`, `Do`, `Fr`, `Sa`, `So`.
- Bei vielen Plaenen den Text kurz halten, damit die Hauptseite nicht
  ueberlaedt.

Optional kann ioBroker zusaetzlich einen JSON-State pro Zone erzeugen:

```json
[
  {"slot":1,"enabled":true,"time":"06:30","days":"Mi,Fr","duration":12},
  {"slot":2,"enabled":true,"time":"20:00","days":"So","duration":5}
]
```

Der JSON-State ist optional. Er ist nuetzlich, wenn spaeter statt eines
einfachen Texts eine kleine Tabelle oder dynamische Liste in VIS gebaut werden
soll. Fuer die Firmware ist dieser State nicht erforderlich.

### VIS-Konfiguration in ioBroker

Dieser Abschnitt beschreibt eine einfache, real umsetzbare VIS-Variante mit
den aktuell geplanten MQTT-Topics. Die genaue Widget-Auswahl haengt von der
installierten VIS-Version und den verfuegbaren Widget-Sets ab. Das Grundprinzip
bleibt gleich: Anzeigen lesen Status-States, Bedienelemente schreiben auf
`/set` States.

#### Empfohlene Darstellung pro Schedule-Slot im Popup

Eine Zeile pro Schedule-Slot im gemeinsamen Popup:

```text
[aktiv] [Stunde] [Minute] [Dauer] [Wochentage] [Loeschen]
```

Beispiel:

```text
Zone 1 / Plan 1 | aktiv: true | 06:30 | 12 min | 20 | Loeschen
```

#### Datenpunkte pro Zeile

Status lesen:

```text
mqtt.0.<device>.irrigation.automatic.enabled
mqtt.0.<device>.irrigationZone1.schedule1.configured
mqtt.0.<device>.irrigationZone1.schedule1.enabled
mqtt.0.<device>.irrigationZone1.schedule1.hour
mqtt.0.<device>.irrigationZone1.schedule1.minute
mqtt.0.<device>.irrigationZone1.schedule1.durationMinutes
mqtt.0.<device>.irrigationZone1.schedule1.daysOfWeek
```

Befehle schreiben:

```text
mqtt.0.<device>.irrigation.automatic.enabled.set
mqtt.0.<device>.irrigationZone1.schedule1.configured.set
mqtt.0.<device>.irrigationZone1.schedule1.enabled.set
mqtt.0.<device>.irrigationZone1.schedule1.hour.set
mqtt.0.<device>.irrigationZone1.schedule1.minute.set
mqtt.0.<device>.irrigationZone1.schedule1.durationMinutes.set
mqtt.0.<device>.irrigationZone1.schedule1.daysOfWeek.set
mqtt.0.<device>.irrigationZone1.schedule1.delete.set
```

`<device>` ist ein Platzhalter fuer den echten Controller-Namen. Keine echten
lokalen Werte in diese Doku schreiben.

Beim gemeinsamen Popup muss die VIS-Konfiguration die Zone dynamisch auswaehlen
oder pro Zone dieselben Widgets mit anderem Prefix verwenden. Technisch sind
beide Varianten umsetzbar:

- Einfacher Aufbau: ein Popup pro Zone kopieren und nur die Datenpunkt-Prefixe
  austauschen.
- Wartbarer Aufbau: ein gemeinsames Popup mit einer ioBroker-Variable
  `selectedZone`, aus der die Widget-Datenpunkte oder ein Skript die Zieltopics
  ableiten.

Welche Variante besser ist, haengt von der vorhandenen VIS-Version und den
verfuegbaren Widgets ab. Die Firmware-Topics bleiben in beiden Faellen gleich.

#### Widget-Vorschlag

| Feld | VIS-Widget-Typ | Lesen von | Schreiben nach |
| --- | --- | --- | --- |
| automatic enabled | Checkbox / Switch | `irrigation.automatic.enabled` | `irrigation.automatic.enabled.set` |
| configured | Anzeige / optional Checkbox | `configured` | `configured.set` |
| enabled | Checkbox / Switch | `enabled` | `enabled.set` |
| hour | Number Input / Select | `hour` | `hour.set` |
| minute | Number Input / Select | `minute` | `minute.set` |
| durationMinutes | Number Input / Slider | `durationMinutes` | `durationMinutes.set` |
| daysOfWeek | Number Input / Select | `daysOfWeek` | `daysOfWeek.set` |
| delete | Button | optional `configured` | `delete.set` |

Empfohlene Wertebereiche:

| Feld | Min | Max | Schritt |
| --- | ---: | ---: | ---: |
| hour | 0 | 23 | 1 |
| minute | 0 | 59 | 1 |
| durationMinutes | 1 | 60 | 1 |
| daysOfWeek | 1 | 127 | 1 |

#### Bedienlogik in VIS

Plan anlegen:

1. Global `irrigation.automatic.enabled.set=true` schreiben, wenn automatische
   Laeufe erlaubt sein sollen.
2. Optional `configured.set=true` schreiben.
3. `hour.set`, `minute.set`, `durationMinutes.set`, `daysOfWeek.set`
   schreiben.
4. `enabled.set=true` schreiben.

Da die Firmware Feld-Updates auf nicht konfigurierte Slots automatisch anlegt,
kann VIS auch direkt mit `hour.set` oder einem anderen Feld starten.

Plan deaktivieren:

```text
enabled.set=false
```

Plan wieder aktivieren:

```text
enabled.set=true
```

Plan loeschen:

```text
delete.set=true
```

oder:

```text
configured.set=false
```

#### Wochentage in VIS

Ein einfacher erster Ansatz ist ein Select- oder Number-Widget fuer
`daysOfWeek`.

Nuetzliche Werte:

| Anzeige | Wert |
| --- | ---: |
| Montag bis Freitag | 31 |
| Wochenende | 96 |
| Jeden Tag | 127 |
| Mittwoch und Freitag | 20 |

Fuer eine komfortablere VIS koennen spaeter sieben Checkboxen verwendet werden.
Diese muessen die Bitmaske berechnen:

```text
Montag     = 1
Dienstag   = 2
Mittwoch   = 4
Donnerstag = 8
Freitag    = 16
Samstag    = 32
Sonntag    = 64
```

Beispiel:

```text
Mittwoch + Freitag = 4 + 16 = 20
```

Wichtig: Wenn sieben Checkboxen verwendet werden, muss die VIS-Logik beim
Aendern einer Checkbox den neuen Summenwert nach `daysOfWeek.set` schreiben.
Die Firmware erwartet aktuell die Bitmaske als Zahl.

Empfehlung fuer das Popup:

- Sieben Checkboxen anzeigen: Mo, Di, Mi, Do, Fr, Sa, So.
- Beim Oeffnen des Popups den aktuellen `daysOfWeek`-Wert in Checkboxen
  zerlegen.
- Bei jeder Aenderung die Summe neu berechnen.
- Die Summe nach `daysOfWeek.set` schreiben.

Beispiel:

```text
Mo=false, Di=false, Mi=true, Do=false, Fr=true, Sa=false, So=false
daysOfWeek = 4 + 16 = 20
```

#### Sichtbarkeit und Bedienkomfort

Empfehlung:

- Einen globalen Switch `Automatik` anzeigen, der
  `irrigation.automatic.enabled.set` schreibt.
- In der Hauptseite pro Zone nur `scheduleSummary` oder eine sehr kurze
  Mini-Tabelle anzeigen.
- Die Bearbeitung von Stunde, Minute, Dauer, Wochentagen, Aktivierung und
  Loeschen in ein Popup verlagern.
- Wenn `configured=false`, die Zeile ausgegraut darstellen.
- Einen Button `Plan anlegen` anzeigen, der `configured.set=true` schreibt.
- Einen Button `Loeschen` anzeigen, der `delete.set=true` schreibt.
- `enabled` nur als Aktiv/Inaktiv-Schalter verwenden, nicht als Loeschfunktion.
- Nach dem Schreiben eines Werts immer den Status-State beobachten, nicht den
  `/set` State als Wahrheit verwenden.

#### VIS-Testablauf

1. `irrigation.automatic.enabled.set=true` schreiben.
2. Pruefen, ob `irrigation.automatic.enabled=true` zurueckkommt.
3. Einen freien Schedule-Slot auswaehlen.
4. `configured.set=true` oder direkt `hour.set` schreiben.
5. Pruefen, ob `configured=true` zurueckkommt.
6. Stunde, Minute, Dauer und Wochentage setzen.
7. Pruefen, ob alle Status-States mit den gesetzten Werten zurueckkommen.
8. `enabled.set=true` schreiben.
9. `enabled=true` im Status pruefen.
10. Fuer einen echten Hardwaretest eine Startzeit kurz in die Zukunft setzen
    und pruefen, ob die Zone zur erwarteten lokalen Zeit startet.
11. `enabled.set=false` schreiben und pruefen, dass der Plan erhalten bleibt.
12. `delete.set=true` schreiben.
13. Pruefen, dass `configured=false` zurueckkommt und andere Schedule-Slots
    ihre Nummer behalten.

## Vorbereitung in Home Assistant

Home Assistant kann MQTT-Entitaeten entweder manuell per YAML oder automatisch
ueber MQTT Discovery bekommen. Fuer das Firmware-Feature ist Discovery
langfristig die angenehmere Loesung.

### Manuelle YAML-Vorbereitung

Beispiel fuer `enabled`:

```yaml
mqtt:
  switch:
    - name: "Irrigation Automatic Enabled"
      unique_id: "irrigation_automatic_enabled"
      state_topic: "<device>/irrigation/automatic/enabled"
      command_topic: "<device>/irrigation/automatic/enabled/set"
      payload_on: "true"
      payload_off: "false"
      state_on: "true"
      state_off: "false"
```

Beispiel fuer Schedule `enabled`:

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
mosquitto_sub -h <broker-host> -t '<device>/irrigation/automatic/#' -v
```

Plan anlegen:

```bash
mosquitto_pub -h <broker-host> -t '<device>/irrigation/automatic/enabled/set' -m 'true'
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
