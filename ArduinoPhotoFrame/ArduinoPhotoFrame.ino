#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define TFT_CS   10
#define TFT_DC   8
#define TFT_RST  9
#define SD_CS    4

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

File root;

void setup() {
  Serial.begin(9600);
  tft.initR(INITR_BLACKTAB);
  //tft.initR(INITR_REDTAB);
  //tft.initR(INITR_GREENTAB);
  
  tft.setRotation(1);       // ⬅️ Display im Querformat
  tft.fillScreen(ST77XX_BLACK);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD-Karte konnte nicht initialisiert werden.");
    while (1);
  }
  Serial.println("SD-Karte bereit.");

  root = SD.open("/");
}

void loop() {
  root.rewindDirectory();

  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      // keine weiteren Dateien → Neustart
      root.rewindDirectory();
      continue;
    }

    if (!entry.isDirectory()) {
      String fname = entry.name();
      fname.toLowerCase();
      if (fname.endsWith(".bmp")) {
        entry.close();  // Schließen, damit showBMP sie öffnen kann

        Serial.print("Zeige Bild: ");
        Serial.println(fname);

        if (showBMP(fname.c_str())) {
          delay(3000);  // 3 Sekunden warten
        } else {
          Serial.println("Fehler beim Anzeigen des Bildes.");
        }
      } else {
        entry.close();
      }
    } else {
      entry.close();
    }
  }
}

// BMP-Datei anzeigen (optimiert)
bool showBMP(const char *filename) {
  File bmpFile = SD.open(filename);
  if (!bmpFile) {
    Serial.println("Bild konnte nicht geöffnet werden.");
    return false;
  }

  // BMP-Header prüfen
  if (read16(bmpFile) != 0x4D42) {
    Serial.println("Kein gültiger BMP-Header!");
    bmpFile.close();
    return false;
  }

  (void)read32(bmpFile); // Dateigröße ignorieren
  (void)read32(bmpFile); // reserviert
  uint32_t bmpImageoffset = read32(bmpFile); // Start der Bilddaten

  (void)read32(bmpFile); // Headergröße
  int32_t bmpWidth  = (int32_t)read32(bmpFile);
  int32_t bmpHeight = (int32_t)read32(bmpFile);
  (void)read16(bmpFile); // Planes
  uint16_t bitDepth = read16(bmpFile);
  uint32_t compression = read32(bmpFile);

  // Nur 16 Bit BMP mit BI_BITFIELDS erlaubt
  if (bitDepth != 16 || compression != 3) {
    Serial.print("Nur 16-Bit BMP mit BI_BITFIELDS unterstützt. bitDepth=");
    Serial.print(bitDepth);
    Serial.print(", compression=");
    Serial.println(compression);
    bmpFile.close();
    return false;
  }

  bool flip = true;
  if (bmpHeight < 0) {
    bmpHeight = -bmpHeight;
    flip = false;
  }

  // Displaygrenzen
  int16_t drawWidth  = min(bmpWidth,  tft.width());
  int16_t drawHeight = min(bmpHeight, tft.height());

  Serial.print("BMP-Größe: ");
  Serial.print(bmpWidth); Serial.print(" x "); Serial.println(bmpHeight);

  uint32_t rowSize = ((bmpWidth * 2 + 3) & ~3);  // auf 4 Byte ausgerichtet

  // Zeilenpuffer
  uint16_t lineBuffer[drawWidth];

  // Bilddaten lesen und zeichnen
  for (int row = 0; row < drawHeight; row++) {
    // Zeilenposition berechnen
    uint32_t pos = bmpImageoffset + (flip ? (bmpHeight - 1 - row) * rowSize : row * rowSize);
    bmpFile.seek(pos);

    // Zeile einlesen
    for (int col = 0; col < drawWidth; col++) {
      lineBuffer[col] = read16(bmpFile);
    }

    // Padding überspringen
    uint32_t padding = rowSize - (bmpWidth * 2);
    if (padding > 0) {
      bmpFile.seek(bmpFile.position() + padding);
    }

    // Zeile effizient zeichnen
    tft.startWrite();
    tft.setAddrWindow(0, row, drawWidth, 1); // Zeile setzen

    for (int col = 0; col < drawWidth; col++) {
      tft.writeColor(lineBuffer[col], 1); // 1 Pixel schreiben
    }

    tft.endWrite();
  }

  bmpFile.close();
  return true;
}

// --- Helferfunktionen ---
uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
