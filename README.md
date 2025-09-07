# Projektbeschreibung: Arduino-Bildanzeige auf TFT-Display

## Überblick
Dieses Arduino-Projekt zeigt 16-Bit-Bilder an.
Die Daten sind auf einer SD-Karte gespeichert und werden auf einem ST7735-TFT-Display angezeigt.
Es zeigt Bilder von einer SD-Karte. Die Bilder sind im BMP-Format.
Die Bilder erscheinen nacheinander im Querformat auf dem Display. Jedes Bild bleibt 3 Sekunden.
Das nächste Bild wird angezeigt, bevor das vorherige ganz verschwindet.
Dazu gibt es ein Python-Skript, das JPG-Bilder in das
Das erforderliche BMP-Format wurde optimiert, um die Kompatibilität zu gewährleisten.
Der Arduino-Code gewährleistet das.

## Hardware
- **Mikrocontroller**: Arduino-kompatibles Board (z. B. Arduino Uno)
- **Display**: ST7735 TFT-Display (1,8 Zoll, 128x160 Pixel, Querformat: 160x128)
- **SD-Kartenmodul**: SPI-kompatibles SD-Kartenmodul zur Speicherung der BMP-Bilder
- **Verkabelung**:
  - TFT_CS: Pin 10
  - TFT_DC: Pin 8
  - TFT_RST: Pin 9
  - SD_CS: Pin 4

## Software
- **Arduino-Code**:
  - **Bibliotheken**:
    - `SPI.h`: Für die Kommunikation mit dem SD-Kartenmodul und dem Display.
    - `SD.h`: Für den Zugriff auf die SD-Karte.
    - `Adafruit_GFX.h` und `Adafruit_ST7735.h`: Für die Steuerung des ST7735 TFT-Displays.
  - **Funktionalität**:
    - **Initialisierung**: Das Display wird im Querformat initialisiert, und die SD-Karte wird geprüft. Bei einem Fehler in der SD-Karteninitialisierung wird das Programm gestoppt.
    - **Hauptschleife**: Das Programm durchläuft kontinuierlich die Dateien auf der SD-Karte, filtert nach BMP-Dateien (Dateiendung `.bmp`) und zeigt diese an.
    - **BMP-Anzeige**: Unterstützt ausschließlich 16-Bit BMP-Dateien mit BI_BITFIELDS-Kompression. Die Bilddaten werden zeilenweise eingelesen, in einem Puffer gespeichert und effizient auf das Display übertragen.
    - **Fehlerbehandlung**: Überprüft die Gültigkeit des BMP-Headers und meldet Fehler, falls das Bildformat nicht unterstützt wird oder die Datei nicht geöffnet werden kann.

- **Python-Skript zur Bildoptimierung**:
  - **Bibliotheken**: `PIL` (Pillow) für Bildverarbeitung, `os` und `glob` für Dateiverwaltung.
  - **Funktionalität**:
    - **Eingabe**: Verarbeitet JPG- und JPG-Dateien (Groß-/Kleinschreibung) aus dem aktuellen Verzeichnis.
    - **Verarbeitung**:
      - Resiziert Bilder auf 160x128 Pixel (passend zum Display im Querformat).
      - Konvertiert die Pixel in RGB565-Format (16-Bit-Farbtiefe), das für das ST7735-Display optimiert ist.
      - Erstellt einen spezifischen BMP-Header mit BI_BITFIELDS-Kompression (Wert 3), inklusive Bitmasks für Rot (0xF800), Grün (0x07E0) und Blau (0x001F).
      - Speichert die Pixel-Daten in umgekehrter Reihenfolge (bottom-to-top, wie im BMP-Format üblich) mit Padding pro Zeile auf Vielfaches von 4 Bytes.
    - **Ausgabe**: Speichert die konvertierten BMP-Dateien in einem Ordner namens `bmp_out`. Der Dateiname basiert auf dem Original ohne Erweiterung, ergänzt um `.bmp`.
    - **Beispiel-Ausführung**: Das Skript durchläuft alle JPG-Dateien und gibt eine Bestätigungsnachricht aus, z. B. "Konvertiert input.jpg -> bmp_out/input.bmp".
  - **Hinweise**:
    - Das Skript ist nur unter Linux getestet. Es sollte unter anderen Betriebssystemen (z. B. Windows, macOS) ähnlich funktionieren, da es auf standardmäßigen Python-Bibliotheken basiert, aber eine Überprüfung empfohlen wird.
    - Voraussetzungen: Python mit installiertem Pillow (`pip install Pillow`).
    - Die Konvertierung stellt sicher, dass die BMP-Dateien exakt dem Format entsprechen, das der Arduino-Code verarbeitet, und vermeidet Kompatibilitätsprobleme.

## Funktionsweise
1. **Bildvorbereitung (Python)**:
   - JPG-Bilder werden mit dem Python-Skript in optimierte 16-Bit BMP-Dateien umgewandelt und auf die SD-Karte kopiert.
2. **Setup (Arduino)**:
   - Initialisiert die serielle Kommunikation (9600 Baud).
   - Konfiguriert das TFT-Display (Schwarz als Hintergrund, Querformat).
   - Prüft die SD-Karte und öffnet das Root-Verzeichnis.
3. **Loop (Arduino)**:
   - Durchsucht das Root-Verzeichnis der SD-Karte nach Dateien.
   - Erkennt BMP-Dateien anhand der Dateiendung `.bmp`.
   - Zeigt jedes Bild für 3 Sekunden an, bevor es die nächste Datei verarbeitet.
4. **BMP-Verarbeitung (Arduino)**:
   - Liest den BMP-Header, um Bildgröße, Bit-Tiefe und Kompression zu prüfen.
   - Verarbeitet nur 16-Bit BMPs mit BI_BITFIELDS-Kompression.
   - Zeichnet das Bild zeilenweise auf das Display, wobei die Bildgröße an die Displayauflösung (128x160 Pixel) angepasst wird.
   - Berücksichtigt Padding in den Bilddaten und unterstützt sowohl normale als auch vertikal gespiegelte BMPs.

## Optimierungen
- **Effiziente Darstellung (Arduino)**: Verwendet einen Zeilenpuffer und die Funktion `setAddrWindow` des Displays, um Pixel zeilenweise zu schreiben, was die Performance verbessert.
- **Speicherbedarf (Arduino)**: Nutzt einen dynamischen Puffer, der nur die Breite des Bildes abdeckt, um Speicher zu sparen.
- **Bildoptimierung (Python)**: Automatische Resisierung und Farbraumkonvertierung minimiert Dateigröße und stellt Kompatibilität sicher.
- **Fehlerbehandlung**: Robuste Prüfung von Dateien und Headern verhindert Abstürze bei ungültigen oder inkompatiblen BMP-Dateien.

## Anwendung
Das Projekt eignet sich für Anwendungen wie digitale Bilderrahmen, Diashows oder die Anzeige von statischen Grafiken in Arduino-Projekten. Es ist besonders nützlich, wenn Bilder offline auf einer SD-Karte gespeichert und ohne Internetverbindung angezeigt werden sollen. Das Python-Skript erleichtert die Vorbereitung der Bilder für Nicht-Programmierer.

## Einschränkungen
- **Arduino**: Unterstützt nur 16-Bit BMP-Dateien mit BI_BITFIELDS-Kompression. Die Bildgröße ist auf die Displayauflösung (128x160 Pixel) beschränkt. Keine Unterstützung für Verzeichnisstrukturen; Bilder müssen im Root-Verzeichnis der SD-Karte liegen.
- **Python**: Nur JPG-Eingaben; erweitert für andere Formate möglich. Getestet nur unter Linux – Kompatibilität auf anderen OS sollte überprüft werden. Keine Unterstützung für Transparenz oder Alpha-Kanäle.

## Erweiterungsmöglichkeiten
- Unterstützung weiterer BMP-Formate (z. B. 24-Bit) im Arduino-Code.
- Implementierung einer Benutzeroberfläche zur Bildauswahl.
- Hinzufügen von Animationen oder Übergangseffekten zwischen Bildern.
- Erweiterung des Python-Skripts: Unterstützung für weitere Bildformate (PNG, etc.), Batch-Verarbeitung oder GUI-Integration.
- Plattformübergreifende Tests für das Python-Skript (z. B. unter Windows/macOS).
