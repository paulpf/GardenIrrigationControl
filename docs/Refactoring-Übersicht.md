# Refactoring-Uebersicht

Stand: 2026-04-24

## Ziel der Refactorings
- Firmware robuster, wartbarer und einfacher erweiterbar machen.
- Verantwortlichkeiten sauber trennen (insbesondere in `main.cpp`).
- Fachlogik testbarer machen (weniger Infrastruktur-Kopplung).
- Schrittweise modernisieren, ohne die lauffaehige Funktionalitaet zu verlieren.

## Aktueller Stand (Ist)

### Bereits umgesetzt
- `main.cpp` wurde deutlich verschlankt und auf Orchestrierung fokussiert.
- Water-Level-Logik wurde objektorientiert in eine eigene Klasse ausgelagert:
  - `src/waterlevelmanager.h`
  - `src/waterlevelmanager.cpp`
- Dependency Inversion fuer Publishing wurde umgesetzt:
  - `src/imessagepublisher.h` eingefuehrt
  - `MqttManager` implementiert die Abstraktion
  - `WaterLevelManager` ist von der konkreten MQTT-Implementierung entkoppelt
- Zustandslogik und Seiteneffekte wurden getrennt:
  - Transition-Berechnung ueber `updateStateTransitions()`
  - Effekte (Log/MQTT/Sperre) ueber `applyTransitionEffects()`
- Ueberlauf-/Sicherheitslogik ist enthalten:
  - 100% = voll
  - >100% = Ueberlauf
  - kritischer Ueberlauf wird in Litern bewertet
  - Safety-Lock mit Sperre/Freigabe vorhanden
- Native-Testabdeckung wurde erweitert:
  - `test/test_native_waterlevel_manager/test_main.cpp`
  - Hysterese- und Zustandswechsel fuer Lockout/Overflow/Alarm/Safety-Lock
  - Safety-Lock bleibt aktiv bei partieller Freigabe (11 Tests, alle gruen)
- Sensorzugriff wurde abstrahiert (Schritt 2):
  - `src/iwaterlevelsensorreader.h` Interface eingefuehrt
  - `src/esp32waterlevelsensor.h/.cpp` konkrete ESP32-Implementierung
  - `WaterLevelManager` bekommt `IWaterLevelSensorReader` per Konstruktor (DI)
  - `test/support/fakes/FakeWaterLevelSensor.h` fuer kuenftige direkte Tests
  - 3 neue Testfaelle: Safety-Lock Partial-Release, Multi-Event-Zyklus, Hysterese ueber mehrere Zyklen
- Transition-Events wurden typisiert (Schritt 4):
  - Bool-Flags durch typisierte Event-Liste (`Type` + `active`) ersetzt
  - Effekte werden event-getrieben in einer zentralen Schleife verarbeitet
  - Tests auf typed Events umgestellt und um Event-Reihenfolge erweitert
- Publish-Semantik fuer Bool-Topics wurde abgesichert:
  - neuer nativer Manager-Test mit Fake-Sensor + Fake-Publisher
  - Bool-State-Topics werden nur noch bei echten Zustandswechseln publiziert
  - periodische Datenpunkte (`percent`, `liters`, `raw`, `status`) bleiben zyklisch aktiv
  - gemeinsame Host-Bindings fuer WaterLevelManager-Tests reduzieren Stub-Duplikate
  - Disconnected-Fall ist ebenfalls abgedeckt (keine Publishes ohne Verbindung)
- Connectivity-Steuerung wurde fuer App-Level-Testbarkeit separiert:
  - `ConnectivityCoordinator` kapselt WiFi-Event-Verbrauch und MQTT-Connect-Steuerung
  - `WifiManager` und `MqttManager` exponieren dafuer schlanke Interfaces
  - nativer Test deckt Disconnect-, Connect- und Ensure-Connect-Verhalten ab
- Build und Tests waren bei den letzten Schritten erfolgreich (91/91).

### Letzte relevante Commits
- `76213cb` test: add native coverage for WaterLevelManager transitions
- `b685a99` refactor: separate water level state transitions from side effects
- `f6d2f7b` refactor: apply DIP for WaterLevelManager publishing
- `a58b1c2` refactor: extract water level logic into WaterLevelManager
- `46301fa` feat: refine overflow safety logic and refactor water level flow
- `ab188fa` Phase 5.5: Water level sensor + low-water lockout
- `e7d7435` Phase 5.4: scheduled irrigation framework

### Aktuelle lokale Abweichung
- Keine uncommitteten Dateien.
- Branch-Status: `main` ist aktuell 1 Commit vor `origin/main` (`76213cb`).

## Was am Design schon gut ist
- Trennung von App-Orchestrierung (`main.cpp`) und Domaenenlogik (`WaterLevelManager`).
- Klarere Verantwortlichkeit fuer Wasserstand, Alarm, Sperrlogik und Publikation.
- Schrittweises Refactoring ohne grossen Big-Bang-Umbau.

## Naechste sinnvolle Refactoring-Schritte

### 1) Konfiguration logisch gruppieren (mittlere Prioritaet)
Problem:
- Viele Schwellwerte liegen verteilt in `config.h`.

Ziel:
- Water-Level-Konfiguration als strukturierte Einheit (z. B. `WaterLevelConfig`).

Nutzen:
- Uebersichtlicher.
- Weniger Fehler bei neuen Parametern.

### 2) Sensorzugriff abstrahieren ✅ ERLEDIGT
- `IWaterLevelSensorReader` Interface eingefuehrt
- `Esp32WaterLevelSensor` isoliert den Hardware-Zugriff
- `WaterLevelManager` per DI entkoppelt
- `FakeWaterLevelSensor` verfuegbar fuer Tests

### 3) Fokus-Tests fuer WaterLevelManager weiter ausbauen (mittlere Prioritaet)

### 4) Transition-Events als typisierte Events modellieren ✅ ERLEDIGT
- Bool-Flags wurden durch typisierte Event-Liste ersetzt
- Reihenfolge und Duplikat-Verhalten sind im nativen Test abgedeckt

## Risiko-/Aufwands-Einschaetzung
- Schritt 1 (Konfig-Gruppierung): niedriges Risiko, mittlerer Nutzen.
- Schritt 2 (Sensor-Interface): niedriges bis mittleres Risiko, hoher Nutzen.
- Schritt 3 (weiterer Testausbau): niedriges Risiko, hoher Nutzen.
- Schritt 4 (typisierte Events): mittleres Risiko, mittlerer bis hoher Nutzen.

## Empfohlene Reihenfolge
1. Konfigurationsmodell gruppieren
2. Sensorzugriff abstrahieren
3. WaterLevelManager-Tests weiter ausbauen
4. Transition-Events typisieren

## Definition of Done (Refactoring-Etappe)
- Build erfolgreich fuer ESP32-Umgebungen.
- Native-Tests gruen.
- Neue/angepasste Tests fuer geaenderte Logik vorhanden.
- Keine Verhaltensaenderung ausser explizit beabsichtigt.
- Kurze Doku-Notiz in dieser Datei aktualisiert.
