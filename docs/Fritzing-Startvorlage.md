# Fritzing-Startvorlage fuer GardenIrrigationControl

Diese Vorlage beschreibt die Verdrahtung fuer ein erstes Fritzing-Projekt auf Basis des aktuellen Repos und der geplanten Hardwarebelegung.
Sie ist absichtlich so aufgebaut, dass sie sich 1:1 in der Breadboard-Ansicht nachbauen laesst.

## Wichtiger Hinweis zu Pin-Nummern

Beim ESP32 DevKit C V4 ist die auf dem Board aufgedruckte Pin-Beschriftung in vielen Faellen direkt die GPIO-Nummer, aber nicht immer als Begriff klar getrennt.
Fuer die Verdrahtung und fuer den Code gilt deshalb immer:

- `GPIO-Nummer` = die technische Signalnummer, die du im Code verwendest
- `Board-Label` = die Beschriftung am Pinheader des DevKit-Boards

Bei den hier verwendeten Pins ist das Board-Label in der Regel identisch zur GPIO-Nummer, zum Beispiel:

- Board-Label `16` = `GPIO16`
- Board-Label `33` = `GPIO33`
- Board-Label `VP` = `GPIO36`
- Board-Label `VN` = `GPIO39`

Gerade `VP` und `VN` sind wichtig, weil dort auf dem Board nicht einfach `36` oder `39` steht.

## Zielbild

- 1x ESP32 DevKit C V4
- 1x 8-Kanal SSR-Modul, low-level trigger, 5V
- 1x 1-Kanal SSR-/Relaismodul fuer Ventil 9
- 9 Ventile gesamt
- davon 8 Beregnungsventile
- davon 1 Zusatzventil
- 10 Taster gesamt
- davon 9 Ventiltaster
- davon 1 Funktionstaste, z. B. `Abort / Alles aus`
- kein DHT11 in dieser Variante

## Zielarchitektur 9+10

Die geplante Aufteilung ist:

- Ventil 1 bis 8: Beregnungszonen
- Ventil 9: Zusatzventil
- Taste 1 bis 9: direkte Zuordnung zu Ventil 1 bis 9
- Taste 10: Funktionstaste, z. B. `Abort / Alles aus`

## Empfohlene Pinbelegung

Diese Belegung ist fuer direkten Betrieb ohne Expander sinnvoll und moeglichst stabil.

### Ventile

| Ventil | Funktion | GPIO | Board-Label |
|---|---|---:|---|
| Ventil 1 | Beregnung | 16 | `16` |
| Ventil 2 | Beregnung | 17 | `17` |
| Ventil 3 | Beregnung | 18 | `18` |
| Ventil 4 | Beregnung | 19 | `19` |
| Ventil 5 | Beregnung | 21 | `21` |
| Ventil 6 | Beregnung | 22 | `22` |
| Ventil 7 | Beregnung | 23 | `23` |
| Ventil 8 | Beregnung | 25 | `25` |
| Ventil 9 | Zusatzventil | 26 | `26` |

### Taster

| Taste | Funktion | GPIO | Board-Label | Hinweis |
|---|---|---:|---|---|
| Taste 1 | Ventil 1 | 4 | `4` | interner Pulldown nutzbar |
| Taste 2 | Ventil 2 | 13 | `13` | interner Pulldown nutzbar |
| Taste 3 | Ventil 3 | 14 | `14` | interner Pulldown nutzbar |
| Taste 4 | Ventil 4 | 27 | `27` | interner Pulldown nutzbar |
| Taste 5 | Ventil 5 | 32 | `32` | interner Pulldown nutzbar |
| Taste 6 | Ventil 6 | 33 | `33` | interner Pulldown nutzbar |
| Taste 7 | Ventil 7 | 34 | `34` | externer Pulldown noetig |
| Taste 8 | Ventil 8 | 35 | `35` | externer Pulldown noetig |
| Taste 9 | Ventil 9 | 36 | `VP` | externer Pulldown noetig |
| Taste 10 | Funktionstaste / Abort | 39 | `VN` | externer Pulldown noetig |

## Gesamttabelle fuer Verdrahtung und Code

Diese Tabelle ist die praktischste Referenz beim Aufbau.

| Kanal | Rolle | Signal | GPIO im Code | Board-Label am ESP32 |
|---|---|---|---:|---|
| 1 | Ventil | Ausgang | 16 | `16` |
| 2 | Ventil | Ausgang | 17 | `17` |
| 3 | Ventil | Ausgang | 18 | `18` |
| 4 | Ventil | Ausgang | 19 | `19` |
| 5 | Ventil | Ausgang | 21 | `21` |
| 6 | Ventil | Ausgang | 22 | `22` |
| 7 | Ventil | Ausgang | 23 | `23` |
| 8 | Ventil | Ausgang | 25 | `25` |
| 9 | Ventil | Ausgang | 26 | `26` |
| 1 | Taste | Eingang | 4 | `4` |
| 2 | Taste | Eingang | 13 | `13` |
| 3 | Taste | Eingang | 14 | `14` |
| 4 | Taste | Eingang | 27 | `27` |
| 5 | Taste | Eingang | 32 | `32` |
| 6 | Taste | Eingang | 33 | `33` |
| 7 | Taste | Eingang | 34 | `34` |
| 8 | Taste | Eingang | 35 | `35` |
| 9 | Taste | Eingang | 36 | `VP` |
| 10 | Taste / Funktion | Eingang | 39 | `VN` |

## Warum diese Belegung sinnvoll ist

- Die Ventile liegen auf gut nutzbaren Ausgangspins.
- Die Taster nutzen die input-only Pins `34`, `35`, `36`, `39` sinnvoll aus.
- Die problematischeren Strapping-Pins `0`, `2`, `5`, `12`, `15` bleiben frei.
- `GPIO1` und `GPIO3` bleiben fuer USB-Serial und Debugging frei.
- `GPIO6` bis `GPIO11` werden nicht verwendet, da sie fuer Flash reserviert sind.

## Wichtiger Hinweis zu Tastern an GPIO34 bis GPIO39

Die Pins `GPIO34`, `GPIO35`, `GPIO36` und `GPIO39` sind nur Eingaenge und besitzen keine internen Pull-up- oder Pull-down-Widerstaende.
Dort brauchst du pro Taste einen externen Pull-down, wenn der Taster gegen `3V3` geschaltet wird.

Empfehlung:

- Taster gegen `3V3`
- je `10k` Pull-down nach `GND` fuer `GPIO34`, `GPIO35`, `GPIO36`, `GPIO39`
- `10k` ist hier der Standardwert und fuer Taster an ESP-Eingaengen eine gute, stabile Wahl
- sinnvoller Bereich waere auch etwa `4.7k` bis `10k`, aber ich wuerde fuer dieses Projekt einfach durchgaengig `10k` nehmen

## Anschluss-Skizze fuer externen Pull-down

So wird ein Taster an einem Pin ohne internen Pull-down angeschlossen, zum Beispiel an `GPIO34`, `GPIO35`, `VP/GPIO36` oder `VN/GPIO39`:

```text
3.3V
 |
 |
[o] Taster
 |
 +-------> zum ESP32-Eingang, z. B. GPIO34
 |
[10k]
 |
 |
GND
```

Bedeutung:

- wenn der Taster offen ist, zieht der `10k` Widerstand den GPIO auf `GND` = `LOW`
- wenn der Taster gedrueckt wird, liegt der GPIO direkt an `3.3V` = `HIGH`

## Verdrahtung des externen Pull-downs Schritt fuer Schritt

Pro Taste an `GPIO34`, `GPIO35`, `GPIO36` oder `GPIO39`:

1. Einen Kontakt des Tasters an `3V3`
2. Den anderen Kontakt des Tasters an den gewuenschten GPIO
3. Einen `10k` Widerstand zwischen genau diesem GPIO und `GND`

Also konkret zum Beispiel fuer `Taste 10` an `VN / GPIO39`:

```text
ESP32 3V3 ---- Taster ---- GPIO39 (VN)
                           |
                          10k
                           |
ESP32 GND -----------------+
```

## Was in Fritzing gezeichnet werden sollte

Fuer `GPIO34`, `GPIO35`, `VP/GPIO36` und `VN/GPIO39` jeweils:

- 1x Taster zwischen `3V3` und GPIO
- 1x Widerstand `10k` zwischen GPIO und `GND`

Fuer `GPIO4`, `GPIO13`, `GPIO14`, `GPIO27`, `GPIO32`, `GPIO33` brauchst du diesen externen Pull-down nicht, wenn die Firmware den internen Pulldown verwendet.

## Empfohlene Anordnung der Tastenverdrahtung

Ja, die Taster lassen sich in Fritzing sauberer nebeneinander anordnen.
Wenn die Verdrahtung optimal zu den Pins am ESP32 liegen soll, ist die physische Pin-Nahe wichtiger als die Reihenfolge `T1` bis `T10`.

Beim ESP32 DevKit C V4 liegen fast alle hier genutzten Tasterpins auf derselben Header-Seite:

- `GPIO13`, `GPIO14`, `GPIO27`, `GPIO32`, `GPIO33`, `GPIO34`, `GPIO35`, `VP/GPIO36`, `VN/GPIO39`
- nur `GPIO4` liegt auf der gegenueberliegenden Header-Seite

Darum ist diese Anordnung am saubersten:

```text
          ESP32 DevKit C V4

linke Header-Seite, pin-nah:

VP  -> T9  -> 10k -> GND
VN  -> T10 -> 10k -> GND
34  -> T7  -> 10k -> GND
35  -> T8  -> 10k -> GND
32  -> T5
33  -> T6
27  -> T4
14  -> T3
13  -> T2

rechte Header-Seite:

4   -> T1
```

Praktische Fritzing-Regel:

- `T2` bis `T10` als senkrechte oder leicht versetzte Tasterleiste direkt links neben den ESP32 setzen
- die Reihenfolge der Taster an die Reihenfolge der ESP32-Pins anpassen, nicht an die Tasternummern
- `T1` separat rechts unten neben `GPIO4` setzen
- eine gemeinsame `3V3`-Schiene parallel zur Tasterleiste fuehren
- von `3V3` kurze Leitungen zu jeweils einer Tasterseite legen
- von der anderen Tasterseite kurze Leitungen direkt zum naechsten ESP32-Pin fuehren
- die vier Pull-down-Widerstaende fuer `T7`, `T8`, `T9` und `T10` direkt neben die jeweiligen Signalleitungen setzen
- eine kurze gemeinsame `GND`-Schiene nur fuer diese vier Pull-downs verwenden
- `T10` trotzdem gut beschriften, weil er die Funktionstaste ist und nicht zu Ventil 10 gehoert

Wenn du die Taster unbedingt als horizontale Reihe darstellen moechtest, nimm nicht `T1, T2, T3...` als Reihenfolge.
Nimm stattdessen die Pin-Reihenfolge:

```text
T9   T10  T7   T8   T5   T6   T4   T3   T2        T1 separat rechts
VP   VN   G34  G35  G32  G33  G27  G14  G13       G4
```

So liegen die Taster elektrisch unveraendert, aber die Leitungen bleiben moeglichst kurz und kreuzen sich kaum.

## Bauteile in Fritzing

Nutze nach Moeglichkeit diese Parts oder aehnliche:

- ESP32 DevKit C / ESP32 Dev Board
- 8 Channel Relay Module oder 8 Channel SSR Module
- 1 Channel Relay Module oder 1 einzelner SSR-Kanal
- Pushbutton
- Widerstand `10k`
- Schraubklemme / Terminal Block fuer externe Anschluesse
- DC supply / Power connector

Hinweis:
Falls dein exaktes AZ-Delivery-Board oder SSR-Modul in Fritzing fehlt, nimm fuer den Start ein generisches ESP32-Devboard und generische Relais-/SSR-Module. Die Pinbelegung ist fuer die Dokumentation wichtiger als die exakte Geometrie.

## Spannungen

- `VIN_5V`: Versorgung fuer SSR-Module
- `3V3`: Versorgung fuer die Tasterlogik
- `GND`: gemeinsame Masse zwischen ESP32 und SSR-Modulen

## Verdrahtung ESP32 zu Tastern

Alle Taster werden zwischen GPIO und `3V3` angeschlossen.

### Taster mit internem Pull-down

- Taste 1 an GPIO4, Board-Label `4`
- Taste 2 an GPIO13, Board-Label `13`
- Taste 3 an GPIO14, Board-Label `14`
- Taste 4 an GPIO27, Board-Label `27`
- Taste 5 an GPIO32, Board-Label `32`
- Taste 6 an GPIO33, Board-Label `33`

### Taster mit externem Pull-down

- Taste 7 an GPIO34, Board-Label `34`
- Taste 8 an GPIO35, Board-Label `35`
- Taste 9 an GPIO36, Board-Label `VP`
- Taste 10 an GPIO39, Board-Label `VN`

Fuer diese vier Taster gilt jeweils:

- eine Seite des Tasters an `3V3`
- andere Seite an GPIO
- zusaetzlich `10k` Pull-down von GPIO nach `GND`

## Verdrahtung ESP32 zu SSR

Die Ausgaenge sind low-level-triggered.
Das bedeutet:

- `LOW` = Relais aktiv
- `HIGH` = Relais aus

### 8-Kanal SSR fuer Beregnungsventile

- GPIO16, Board-Label `16`, an `IN1`
- GPIO17, Board-Label `17`, an `IN2`
- GPIO18, Board-Label `18`, an `IN3`
- GPIO19, Board-Label `19`, an `IN4`
- GPIO21, Board-Label `21`, an `IN5`
- GPIO22, Board-Label `22`, an `IN6`
- GPIO23, Board-Label `23`, an `IN7`
- GPIO25, Board-Label `25`, an `IN8`

### Einzelkanal fuer Ventil 9

- GPIO26, Board-Label `26`, an `IN9`

## Empfohlene Baugruppen im Diagramm

Ordne die Teile in Fritzing in vier Bloecken an:

1. Links oben: Versorgung
2. Mitte: ESP32
3. Rechts: 8-Kanal SSR-Modul plus Einzelkanal fuer Ventil 9
4. Unten: Tasterleiste mit Funktionstaste

## Minimale Verbindungen

### Versorgung

- ESP32 `5V/VIN` an `VIN_5V`
- 8-Kanal SSR `VCC` an `VIN_5V`
- Einzel-SSR `VCC` an `VIN_5V`
- ESP32 `GND` an `GND`
- 8-Kanal SSR `GND` an `GND`
- Einzel-SSR `GND` an `GND`

### Ventile

- Ventil 1 bis 8 auf dem 8-Kanal-SSR-Modul
- Ventil 9 auf dem Einzelkanal-Modul

### Funktionstaste

Taste 10 ist nicht einem Ventil zugeordnet.
Sie ist als allgemeine Funktionstaste vorgesehen, zum Beispiel fuer:

- `Abort / Alles aus`
- `Modus wechseln`
- `Reset laufender Bewaesserung`

## Darstellungshinweise fuer Fritzing

- Versorgungsleitungen rot fuer `5V`, orange fuer `3V3`, schwarz fuer `GND`
- Tasterleitungen kurz und direkt nach unten fuehren
- SSR-Steuerleitungen als gebuendelte Signalleitungen rechts herausfuehren
- Ventil 1 bis 8 als zusammengehoerigen Beregnungsblock darstellen
- Ventil 9 als separaten Zusatzkanal darstellen
- Taste 10 optisch absetzen und als Funktionstaste beschriften
- Pull-down-Widerstaende fuer `GPIO34`, `GPIO35`, `GPIO36`, `GPIO39` sichtbar einzeichnen

## Pull-up oder Pull-down?

Fuer dein aktuelles Projekt wuerde ich bei den Tastern **Pull-down** verwenden.
Das passt zu der hier beschriebenen Verdrahtung:

- Taster zwischen `3V3` und GPIO
- Widerstand von GPIO nach `GND`
- ungedrueckt = `LOW`
- gedrueckt = `HIGH`

### Funktionsweise Pull-down

Beim Pull-down haelt der Widerstand den Eingang im Ruhezustand auf `0V` beziehungsweise `LOW`.
Wenn du den Taster drueckst, wird der GPIO direkt mit `3.3V` verbunden und liest `HIGH`.

```text
3.3V
 |
[o] Taster
 |
 +-------> GPIO
 |
[10k]
 |
GND
```

### Funktionsweise Pull-up

Beim Pull-up ist es genau umgekehrt:

- Widerstand von GPIO nach `3V3`
- Taster zwischen GPIO und `GND`
- ungedrueckt = `HIGH`
- gedrueckt = `LOW`

```text
3.3V
 |
[10k]
 |
 +-------> GPIO
 |
[o] Taster
 |
GND
```

### Unterschied in der Praxis

`Pull-down`:

- Ruhezustand ist `LOW`
- Tastendruck ergibt `HIGH`
- passt gut zu deiner bisherigen Denkweise `Taste gedrueckt = HIGH`

`Pull-up`:

- Ruhezustand ist `HIGH`
- Tastendruck ergibt `LOW`
- ist in vielen Mikrocontroller-Projekten ebenfalls sehr ueblich

Elektrisch sind beide Varianten voellig in Ordnung, solange Hardware und Software zusammenpassen.

### Was ist hier die beste Wahl?

Fuer diese Dokumentation und die vorgeschlagene Belegung empfehle ich:

- fuer `GPIO4`, `GPIO13`, `GPIO14`, `GPIO27`, `GPIO32`, `GPIO33`: internen **Pull-down**, wenn die Firmware das so nutzt
- fuer `GPIO34`, `GPIO35`, `GPIO36`, `GPIO39`: externen **Pull-down** mit `10k`

Der Hauptgrund ist Konsistenz:

- alle Taster verhalten sich gleich
- ungedrueckt = `LOW`
- gedrueckt = `HIGH`

### Wann waere Pull-up sinnvoll?

Pull-up waere dann sinnvoll, wenn du die Firmware bewusst auf `INPUT_PULLUP` auslegst und alle Taster nach `GND` schaltest.
Das ist ebenfalls robust, aber dann muessten Logik und Doku entsprechend umgestellt werden.

Fuer den aktuellen Vorschlag hier gilt deshalb klar:

- **verwende Pull-down**
- **Widerstandswert: 10k**

## Abgleich mit der aktuellen Firmware

Diese Vorlage bildet die empfohlene Hardwarebelegung ab.
Die aktuelle Firmware im Repo verwendet derzeit noch andere Pins.

Das ist hier so gewollt, weil in diesem Schritt nur Diagramm und Doku angepasst werden sollen, nicht die Firmware.

## Verknuepfte Grafiken

Die passenden Diagramme liegen hier:

- [_assets/_images/irrigation-wiring-9valves-10buttons.svg](../_assets/_images/irrigation-wiring-9valves-10buttons.svg)
- [docs/Verdrahtung.drawio](Verdrahtung.drawio)

