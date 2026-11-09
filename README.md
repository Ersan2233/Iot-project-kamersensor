# IoT Sensorstation & RGB LED Controller

Een Internet of Things (IoT) project aangedreven door een ESP32 microcontroller voor het uitlezen van omgevingsvariabelen (licht, temperatuur en luchtkwaliteit) en het aansturen van een 12V RGB LED-strip via een NLED WS2811 driver. De opgevraagde sensordata worden automatisch doorgestuurd naar een PHP-webapplicatie voor realtime weergave en verwerking.

## 👥 Auteurs

* **Ensar Isci**

* **Ersan Ascioglu**

## 📋 Inhoudsopgave

* [Projectoverzicht](#-projectoverzicht)

* [Hardware Onderdelen](#-hardware-onderdelen)

* [Software & Webapplicatie](#-software--webapplicatie)

* [Aansluitschema](#-aansluitschema)

  * [1. Externe Voeding & Buckconverter](#1-externe-voeding-en-buckconverter)

  * [2. NLED WS2811 MOSFET Driver](#2-nled-ws2811-mosfet-driver)

  * [3. BH1750FVI Lichtsensor](#3-bh1750fvi-lichtsensor)

  * [4. DS18B20 Temperatuursonde](#4-ds18b20-temperatuursonde)

  * [5. ENS160 Luchtkwaliteitssensor](#5-ens160-luchtkwaliteitssensor)

* [ESP32 Pinout Overzicht](#-esp32-pinout-overzicht)

* [⚠️ Belangrijke Veiligheids- en Installatienotities](#️-belangrijke-veiligheids--en-installatienotities)

## Projectoverzicht

Dit project combineert omgevingsmonitoring met dynamische verlichting. Via de ESP32 worden data van drie verschillende sensoren opgevraagd om inzicht te krijgen in omgevingslicht, temperatuur en luchtkwaliteit.

De uitgelezen sensorwaarden worden via een Wi-Fi-verbinding doorgestuurd naar een **PHP-webserver**, waar de data live getoond worden en opgeslagen kunnen worden. Afhankelijk van de sensorwaarden of commando's vanuit de webinterface kan de 12V RGB LED-strip aangestuurd worden.

## Hardware Onderdelen

* **Microcontroller:** ESP32 DevKit Board

* **Lichtsensor:** BH1750FVI (I²C)

* **Temperatuursonde:** DS18B20 (1-Wire)

* **Luchtkwaliteitssensor:** ENS160 (I²C)

* **LED Driver:** NLED WS2811 MOSFET Driver

* **LED Strip:** 12V RGB LED-strip

* **Spanningsconversie:** Buckconverter (Step-down DC-DC, 12V → 5V)

* **Voeding:** Externe 12V DC voeding

## Software & Webapplicatie

* **Backend / Webserver:** Geschreven in **PHP** voor het ontvangen van de sensordata (via HTTP POST/GET requests) en het verwerken van de gegevens.

* **Microcontroller Firmware:** C++ / Arduino IDE met Wi-Fi- en HTTP-clientbibliotheken voor communicatie met het PHP-script.
* ** https://esp-sensorstation.alwaysdata.net/
* ** wachtwoord key website: 8e140f823616154a2ee23a15d0b798758ac5453cc88948a09ce80038dc84d7f2
* ** Huis 1 api: 1ea52b1eb253f68030ccc1f570a6ef397c3489e5f8b20c63dcb6ccbcede6f683
* ** Huis 2 api: 4dbb45c7777294e7c21d6c84c026e069e56cc86342b8324e6ca228a9a8c09082

##  Aansluitschema

>  **Let op:** Gebruik altijd de pin-labels (GPIO-nummers) op je ESP32-bord en niet de fysieke pinstructuur. Zorg dat massa ($GND$) altijd gemeenschappelijk is aangesloten.

### 1. Externe Voeding en Buckconverter

Er wordt gebruikgemaakt van drie gescheiden spanningsniveaus: **12V** voor de NLED/LED-strip, **5V** voor de ESP32-ingang, en **3.3V** vanaf de ESP32 voor de sensoren.

| Vanaf | Naar | Opmerking | 
| ----- | ----- | ----- | 
| **12V Voeding (`+`)** | Buckconverter `IN+` | Hoofdvoeding | 
| **12V Voeding (`-`)** | Buckconverter `IN-` | Hoofdmassa | 
| **12V Voeding (`+`)** | NLED INPUT `VIN` | Voeding driver | 
| **12V Voeding (`-`)** | NLED INPUT `GND/COM` | Massa driver | 
| **Buckconverter `OUT+`** | ESP32 `5V` / `VIN` | **Eerst afstellen op 5.0V!** | 
| **Buckconverter `OUT-`** | ESP32 `GND` | ESP32 massa | 

### 2. NLED WS2811 MOSFET Driver

#### INPUT (Voeding & Besturing)

| NLED Pin | ESP32 / Voeding | 
| ----- | ----- | 
| `VIN` | **+12V** Externe voeding | 
| `DATA IN` | **ESP32 GPIO 25** *(Testopstelling)* | 
| `GND/COM` | **GND** (Gemeenschappelijke massa) | 

#### Vierpolige RGB Uitgang (Naar LED-strip)

| NLED Pin | LED-strip Pin | 
| ----- | ----- | 
| `V+` / `U+` | `+` / `+12V` | 
| `R` | `R` (Rood) | 
| `G` | `G` (Groen) | 
| `B` | `B` (Blauw) | 

### 3. BH1750FVI-lichtsensor

I²C Adres: **`0x23`** (wanneer `ADDR` aan `GND` ligt).

| BH1750 Pin | ESP32 Pin | 
| ----- | ----- | 
| `VCC` | **3V3** | 
| `SCL` | **GPIO 22** | 
| `DAT` / `SDA` | **GPIO 21** | 
| `GND` | **GND** | 
| `ADDR` | **GND** | 

### 4. DS18B20-temperatuursonde

| Draadkleur / Functie | ESP32 Pin | 
| ----- | ----- | 
| Rood (Voeding) | **3V3** | 
| Zwart (Massa) | **GND** | 
| Geel (Data) | **GPIO 4** | 

### 5. ENS160-luchtkwaliteitssensor

I²C Adres: **`0x52`** (wanneer `ADD` aan `GND` ligt).

| ENS160 Pin | ESP32 Pin | Opmerking | 
| ----- | ----- | ----- | 
| `SDA` | **GPIO 21** | Gedeelde I²C Bus | 
| `SCL` | **GPIO 22** | Gedeelde I²C Bus | 
| `GND` | **GND** | Massa | 
| `ADD` | **GND** | Selecteert I²C adres `0x52` | 
| `CS` | **3V3** | Schakelt I²C modus in | 
| `INT` | **GPIO 27** | Optioneel (Interrupt) | 

## ESP32 Pinout Overzicht

| ESP32 Pin | Aangesloten Component / Functie | 
| ----- | ----- | 
| **5V / VIN** | 5.0V Ingang van Buckconverter | 
| **3V3** | Sensorvoeding (BH1750, DS18B20, ENS160) | 
| **GND** | Gemeenschappelijke massa ($GND$) | 
| **GPIO 21** | I²C Data (`SDA`) voor BH1750 & ENS160 | 
| **GPIO 22** | I²C Clock (`SCL`) voor BH1750 & ENS160 | 
| **GPIO 4** | 1-Wire Data voor DS18B20 | 
| **GPIO 25** | NLED `DATA IN` | 
| **GPIO 27** | ENS160 `INT` *(Optioneel)* | 

## Belangrijke Veiligheids- en Installatienotities

1. **Afstellen Buckconverter:** Stel de buckconverter altijd eerst met een multimeter af op exact **5.0V** voordat je de ESP32 aansluit. Sluit **nooit** 12V of niet-gereguleerde spanning direct aan op de `3V3` pin.

2. **USB & Externe voeding:** Gebruik niet tegelijkertijd een USB-kabel en de externe 5V voeding op de ESP32, tenzij je bord hier expliciet beveiliging voor heeft.

3. **DS18B20 Pull-up Weerstand:** Officieel schrijft de fabrikant een $4.7\text{ k}\Omega$ pull-up weerstand voor tussen de datalijn (Geel) en $3.3\text{V}$. Als je geen externe weerstand gebruikt, moet de interne pull-up van de ESP32 softwarematig ingeschakeld worden.

4. **Logic Level Shifter:** De ESP32 werkt met 3.3V logica op GPIO 25, terwijl de WS2811 specificatie in principe 5V datasignalen verwacht. Voor langere kabels of productie wordt een 3.3V-naar-5V levelshifter aanbevolen.
