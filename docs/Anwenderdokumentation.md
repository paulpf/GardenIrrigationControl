# Anwenderdokumentation

## Zweck des Systems

`GardenIrrigationControl` ist eine Steuerung fuer eine Gartenbewaesserung auf Basis eines ESP32.
Das System kann mehrere Bewaesserungszonen schalten, den Wasserstand ueberwachen,
Statusmeldungen per MQTT senden und per OTA aktualisiert werden.

Die Anwendung ist dafuer gedacht, im normalen Betrieb weitgehend automatisch zu laufen.

## Was das System kann

- Bis zu 9 Zonen bzw. Ausgaenge schalten
- Zonen per physischem Taster bedienen
- Zonen per MQTT steuern
- Laufzeiten fuer Zonen speichern
- Wasserstand der Zisterne ueberwachen
- Bewaesserung bei zu niedrigem Wasserstand sperren
- Geraetestatus per MQTT melden
- Firmware per WLAN aktualisieren

## Grundprinzip im Betrieb

Nach dem Einschalten startet der Controller automatisch.

Dabei passiert in dieser Reihenfolge:

1. Interner Speicher wird initialisiert
2. Ein Geraetename wird vorbereitet
3. WLAN wird verbunden
4. MQTT, OTA, Wasserstandslogik und Zonen werden gestartet
5. Der Watchdog wird aktiviert

Anschliessend laeuft das System dauerhaft in einer Schleife weiter und bearbeitet in kurzen Intervallen:

- Tastereingaben
- MQTT-Nachrichten
- Zonensteuerung
- Wasserstand
- Statusmeldungen

## Bewaesserungszonen

Jede Zone hat:

- einen physischen Eingang/Taster
- ein Relais zum Schalten des Ventils
- eine konfigurierbare Laufzeit
- einen Status fuer Ein/Aus

Wenn eine Zone gestartet wird, schaltet das Relais ein und ein Timer laeuft ab.
Nach Ablauf der eingestellten Dauer wird die Zone automatisch wieder ausgeschaltet.

## Wasserstand und Schutzfunktionen

Das System misst den Wasserstand in der Zisterne.

Es gibt drei wichtige Faelle:

1. **Normalbetrieb**
   Der Wasserstand ist ausreichend. Bewaesserung ist erlaubt.

2. **Niedriger Wasserstand**
   Das System aktiviert eine Sperre. Neue Bewaesserungsvorgaenge werden verhindert,
   damit die Pumpe oder Anlage nicht trocken laeuft.

3. **Ueberlauf / kritischer Ueberlauf**
   Das System erkennt zu hohe Fuellstaende und meldet diese per MQTT.

## Bedienung

### Physische Taster

Jede Zone kann lokal ueber einen Taster bedient werden.

### MQTT

Das System kann ueber MQTT mit einer Hausautomation oder anderen Clients verbunden werden.

Typische Anwendungsfaelle:

- Zone ein-/ausschalten
- Laufzeit setzen
- Status empfangen
- Wasserstand auswerten

## OTA-Updates

Das System unterstuetzt OTA-Updates ueber WLAN.

Waehrend eines OTA-Updates werden normale Hintergrundaufgaben angehalten,
damit das Update stabil und ohne Stoerungen durchlaeuft.

## Typische Fehlersituationen

### Kein WLAN

Wenn kein WLAN verfuegbar ist, kann keine MQTT-Kommunikation stattfinden.
Lokale Funktionen wie Taster und Relaislogik bleiben grundsaetzlich weiter moeglich,
soweit sie nicht von einer Schutzsperre blockiert werden.

### Kein MQTT

Wenn WLAN vorhanden ist, aber MQTT nicht erreichbar ist, versucht das System die Verbindung erneut aufzubauen.

### Niedriger Wasserstand

Wenn eine Wassermangelsperre aktiv ist, startet keine neue Bewaesserung.
Zuerst muss sich der Wasserstand wieder erholen.

## Was fuer den Anwender wichtig ist

- Das Geraet benoetigt eine stabile Spannungsversorgung
- WLAN und MQTT sind fuer die Fernsteuerung erforderlich
- OTA-Updates sollten nur bei stabiler WLAN-Verbindung durchgefuehrt werden
- Der Wasserstandsschutz ist eine Sicherheitsfunktion und sollte nicht umgangen werden

## Zusammenfassung

Fuer Anwender ist das System vor allem ein automatischer Bewaesserungscontroller.
Er schaltet Ventile, achtet auf Schutzbedingungen, meldet seinen Zustand und kann remote gepflegt werden.
