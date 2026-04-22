# Refactoring-Uebersicht

Stand: 2026-04-22

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
- Ueberlauf-/Sicherheitslogik ist enthalten:
  - 100% = voll
  - >100% = Ueberlauf
  - kritischer Ueberlauf wird in Litern bewertet
  - Safety-Lock mit Sperre/Freigabe vorhanden
- Native-Tests und Firmware-Build waren bei den letzten Schritten erfolgreich.

### Letzte relevante Commits
- `a58b1c2` refactor: extract water level logic into WaterLevelManager
- `46301fa` feat: refine overflow safety logic and refactor water level flow
- `ab188fa` Phase 5.5: Water level sensor + low-water lockout
- `e7d7435` Phase 5.4: scheduled irrigation framework

### Aktuelle lokale Abweichung
- `src/waterlevelmanager.cpp` ist lokal geaendert (aktuell nur Formatierungs-/Layout-Aenderungen).

## Was am Design schon gut ist
- Trennung von App-Orchestrierung (`main.cpp`) und Domaenenlogik (`WaterLevelManager`).
- Klarere Verantwortlichkeit fuer Wasserstand, Alarm, Sperrlogik und Publikation.
- Schrittweises Refactoring ohne grossen Big-Bang-Umbau.

## Naechste sinnvolle Refactoring-Schritte

### 1) Dependency Inversion fuer Publishing (hohe Prioritaet)
Problem:
- `WaterLevelManager` haengt direkt von `MqttManager` ab.

Ziel:
- `WaterLevelManager` arbeitet nur noch gegen ein kleines Interface (z. B. `IMessagePublisher`).

Nutzen:
- Bessere Testbarkeit.
- Geringere Kopplung an konkrete Infrastruktur.
- Leichtere Wiederverwendung.

Vorschlag:
- Neues Interface mit minimalen Methoden:
  - `bool isConnected() const`
  - `void publish(const char* topic, const char* payload)`
- `MqttManager` implementiert dieses Interface.
- `WaterLevelManager` bekommt nur dieses Interface injiziert.

### 2) Seiteneffekte von Zustandslogik trennen (mittlere Prioritaet)
Problem:
- In derselben Methode wird Zustand geaendert und direkt publiziert/geloggt.

Ziel:
- Zustandstransitionen als reine Logik (deterministisch).
- Events ausgeben (z. B. AlarmAn, AlarmAus, LockAn, LockAus).
- Log/MQTT als separater Effekt-Layer.

Nutzen:
- Deutlich besser unit-testbar.
- Fachlogik leichter nachvollziehbar.

### 3) Konfiguration logisch gruppieren (mittlere Prioritaet)
Problem:
- Viele Schwellwerte liegen verteilt in `config.h`.

Ziel:
- Water-Level-Konfiguration als strukturierte Einheit (z. B. `WaterLevelConfig`).

Nutzen:
- Uebersichtlicher.
- Weniger Fehler bei neuen Parametern.

### 4) Fokus-Tests fuer WaterLevelManager ergaenzen (hohe Prioritaet)
Fehlende gezielte Testfaelle:
- Hysterese Low-Water Lockout
- Ueberlauf in Litern und kritischer Ueberlauf
- Safety-Lock Aktivierung/Freigabe
- Keine Flattereffekte an Schwellen

Nutzen:
- Regressionen bei weiteren Refactorings werden frueh gefunden.

## Risiko-/Aufwands-Einschaetzung
- Schritt 1 (DIP): niedriges Risiko, hoher Nutzen.
- Schritt 2 (Event-basierte Trennung): mittleres Risiko, sehr hoher langfristiger Nutzen.
- Schritt 3 (Konfig-Gruppierung): niedriges Risiko, mittlerer Nutzen.
- Schritt 4 (gezielte Tests): niedriges Risiko, hoher Nutzen.

## Empfohlene Reihenfolge
1. Dependency Inversion fuer Publishing
2. WaterLevelManager-Tests ausbauen
3. Zustandstransitionen/Effekte trennen
4. Konfigurationsmodell gruppieren

## Definition of Done (Refactoring-Etappe)
- Build erfolgreich fuer ESP32-Umgebungen.
- Native-Tests gruen.
- Neue/angepasste Tests fuer geaenderte Logik vorhanden.
- Keine Verhaltensaenderung ausser explizit beabsichtigt.
- Kurze Doku-Notiz in dieser Datei aktualisiert.
