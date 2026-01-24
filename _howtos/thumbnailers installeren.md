# THUMBNAILERS INSTALLEREN

Absoluut. Met deze opdracht installeer je de meest robuuste thumbnailers en de bijbehorende plugins om de gaten in je huidige lijst op te vullen.

Kopieer en plak het volgende commando in je terminal:

```bash
sudo apt update && sudo apt install -y \
  tumbler-plugins-extra \
  font-manager \
  libraw-bin \
  ghostscript \
  poppler-utils \
  webp-pixbuf-loader \
  libheif-examples

```

---

### Wat installeer je hiermee precies?

Hier is een kort overzicht van wat deze pakketten toevoegen aan je Thunar-ervaring:

* **`tumbler-plugins-extra`**: De belangrijkste uitbreiding voor Thunar. Het voegt ondersteuning toe voor o.a. mappen-previews en geavanceerde video-codecs.
* **`font-manager`**: Hiermee krijg je de krachtigere thumbnailer voor lettertypen (inclusief betere WOFF2-ondersteuning).
* **`libraw-bin`**: Zorgt ervoor dat Thunar miniaturen kan maken van RAW-bestanden van digitale camera's (zoals .CR2, .ARW, .NEF).
* **`ghostscript` & `poppler-utils**`: Verbeteren de weergave van PDF-, PostScript- en EPS-bestanden.
* **`webp-pixbuf-loader`**: Garandeert dat `.webp` afbeeldingen (die je veel op internet vindt) altijd direct zichtbaar zijn.
* **`libheif-examples`**: Voegt ondersteuning toe voor HEIC-bestanden (het standaard foto-formaat van iPhones).

---

### De "Finishing Touch" (Belangrijk!)

Nadat de installatie is voltooid, moet je de cache van Thunar geforceerd vernieuwen om de nieuwe thumbnailers aan het werk te zetten. Voer deze drie regels uit:

```bash
# 1. Sluit Thunar volledig af
thunar -q

# 2. Verwijder de oude (lege) thumbnails cache
rm -rf ~/.cache/thumbnails/*

# 3. Herstart de thumbnail-service
/usr/lib/x86_64-linux-gnu/tumbler-1/tumblerd &

```

*(Opmerking: Als je op een 32-bits systeem zit of een andere architectuur, kan het pad naar `tumblerd` iets afwijken, maar meestal pikt het systeem dit na een herstart van je pc automatisch op.)*

Zal ik je ook laten zien hoe je in Thunar de **grootte** van de thumbnails aanpast, zodat je de fonts nog beter kunt beoordelen zonder ze te openen?

---

# ONDERSTEUNING VOOR WOFF2 UITBEREIDEN

> GEMINI ->

Omdat `font.thumbnailer` bij jou ontbreekt, gebruikt het systeem de thumbnailer van de GNOME Font Viewer. We gaan deze aanpassen zodat hij ook `.woff` en `.woff2` accepteert.

### Zo pas je het aan:

1. **Open het bestand:**
```bash
sudo nano /usr/share/thumbnailers/gnome-font-viewer.thumbnailer

```


2. **Bewerk de MimeType regel:**
Zoek de regel die begint met `MimeType=`. Voeg daar de WOFF-types aan toe. De regel moet er ongeveer zo uit komen te zien (alles op één regel, gescheiden door puntkomma's):
```text
MimeType=application/x-font-ttf;application/x-font-otf;application/font-woff;application/font-woff2;application/x-font-woff;application/x-font-woff2;

```


3. **Opslaan en afsluiten:**
Druk op `Ctrl+O`, `Enter`, en daarna `Ctrl+X`.

---

### Belangrijke extra stap: Controleer de MIME-types

Thunar kijkt eerst naar wat voor *soort* bestand het is volgens het systeem. Als het systeem denkt dat een `.woff2` bestand gewoon "binary data" is in plaats van een "font", zal de thumbnailer nooit gestart worden.

Check dit door in de terminal te typen:

```bash
mimetype pad/naar/jouw/lettertype.woff2

```

* **Krijg je `font/woff2` of `application/x-font-woff2`?** Dan zit je goed.
* **Krijg je `application/octet-stream`?** Dan herkent Linux het bestandstype niet.

**Als hij het niet herkent**, moet je een lokaal MIME-type aanmaken:

1. Maak de map aan: `mkdir -p ~/.local/share/mime/packages`
2. Maak een nieuw bestand: `nano ~/.local/share/mime/packages/woff2.xml`
3. Plak dit erin:

```xml
<?xml version="1.0" encoding="utf-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="font/woff2">
    <comment>WOFF2 font</comment>
    <glob pattern="*.woff2"/>
  </mime-type>
</mime-info>

```

4. Update de database en wis de cache:

```bash
update-mime-database ~/.local/share/mime
rm -rf ~/.cache/thumbnails/*
thunar -q

```

Zodra je Thunar nu heropent en naar de map met fonts gaat, zou hij de thumbnails moeten gaan genereren. Zie je nu wel kleine lettertjes verschijnen in de iconen?
