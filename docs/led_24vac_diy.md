# 24 V AC Status-LED

Diese Schaltung betreibt eine einzelne 5-mm-LED direkt an 24 V AC. Der Vorwiderstand begrenzt den LED-Strom, die antiparallele Diode schuetzt die LED in der negativen Halbwelle.

## Schaltplan-Grafik

[Schaltplan als SVG oeffnen](led_24vac_schematic.svg)

<figure>
  <img src="led_24vac_schematic.svg" alt="Schaltplan fuer eine 24 V AC Status-LED mit 6,8 kOhm Vorwiderstand und antiparalleler Schutzdiode" width="100%">
  <figcaption>24 V AC Status-LED mit Vorwiderstand R1 und antiparalleler Schutzdiode D1.</figcaption>
</figure>

## Stueckliste

| Menge | Bauteil | Wert / Typ | Hinweis |
|---:|---|---|---|
| 1 | LED | 5 mm, Farbe nach Wunsch | Lange Anschlussfahne ist meist Anode |
| 1 | Widerstand | 6,8 kOhm / 0,5 W | 10 kOhm geht, wenn es dezenter leuchten soll |
| 1 | Diode | 1N4148 oder 1N4007 | Ring/Strich markiert die Kathode |
| 1 | LED-Halter | 5-mm-Frontclip | passend zur Gehaeusefront |
| 1 | Schrumpfschlauch | passend | alle blanken Kontakte isolieren |
| optional | Litze | 0,14-0,25 mm2 | fuer Anschluss an Klemme/Platine |

## Verdrahtung

1. Widerstand R1 in Reihe zur LED einbauen.
2. Schutzdiode D1 direkt antiparallel zur LED anschliessen.
3. Dioden-Kathode, also die Seite mit Ring/Strich, an die LED-Anode anschliessen.
4. Dioden-Anode an die LED-Kathode anschliessen.
5. Alle blanken Anschluesse mit Schrumpfschlauch isolieren.

## Dimensionierung

24 V AC hat eine Spitzenspannung von ungefaehr 34 V.

```text
24 V * 1,414 = ca. 34 V
```

Mit 6,8 kOhm liegt der LED-Spitzenstrom grob bei 4-5 mA. Fuer moderne LEDs reicht das als Statusanzeige meistens aus.

```text
I = (34 V - ca. 2 V) / 6800 Ohm
I = ca. 4,7 mA
```

Die mittlere Verlustleistung im Widerstand liegt deutlich unter 0,25 W. Ein 0,5-W-Widerstand ist trotzdem empfehlenswert, weil er kuehler bleibt und mechanisch robuster ist.

## Fritzing-Hinweis

In Fritzing kannst du die Schaltung mit Standardbauteilen nachbauen:

- AC-Eingang: 2-poliger Schraubklemmenblock oder 2 Pins
- R1: Widerstand, Wert 6,8 kOhm
- LED1: 5-mm-LED
- D1: Diode, Typ 1N4148 oder 1N4007

Wichtig ist die antiparallele Ausrichtung von LED1 und D1.
