
#include <WiFi.h>
#include <NetworkClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DFRobot_ENS160.h>
#include <esp_system.h>
#include <time.h>
#include <math.h>

// =====================================
// INSTELLINGEN - HUIS 1
// =====================================

#define HUIS_NUMMER 1
#define TESTMODUS 0

// WIFI
const char* WIFI_SSID = "jou-wifi-ssid";
const char* WIFI_PASSWORD = "jou-wifi-wachtwoord";

// WEBSITE
const char* API_HOST =
  "esp-sensorstation.alwaysdata.net";

const char* API_URL =
  "https://esp-sensorstation.alwaysdata.net/api.php";

// API KEY HUIS 1
const char* API_KEY =
  "jou-api-sleutel-huis-1-2";

// =====================================
// APPARAAT
// =====================================

#if HUIS_NUMMER == 1
const char* DEVICE_ID = "esp32-1";
#elif HUIS_NUMMER == 2
const char* DEVICE_ID = "esp32-2";
#else
#error "huisnummer: kies 1 of 2"
#endif

// =====================================
// PINNEN
// =====================================

#define SDA_PIN 21
#define SCL_PIN 22
#define TEMP_PIN 4

#define BH1750_ADDR 0x23
#define ENS160_ADDR 0x52

// =====================================
// METINGEN
// =====================================

const unsigned long INTERVAL = 60000;

unsigned long vorigeMeting = 0;
unsigned long teller = 0;

char bootID[17];

// =====================================
// SENSOREN
// =====================================

BH1750 lichtsensor;

OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

DFRobot_ENS160_I2C ens160(&Wire, ENS160_ADDR);

bool tempOK = false;
bool lichtOK = false;
bool ensOK = false;

bool ntpGestart = false;

// =====================================
// I2C
// =====================================

bool i2cGevonden(uint8_t adres) {
  Wire.beginTransmission(adres);
  return Wire.endTransmission() == 0;
}

void scanI2C() {
  Serial.println();
  Serial.println("=== I2C SCANNER ===");

  int gevonden = 0;

  for (int adres = 1; adres < 127; adres++) {
    if (i2cGevonden(adres)) {
      Serial.print("I2C gevonden: 0x");

      if (adres < 16) {
        Serial.print("0");
      }

      Serial.println(adres, HEX);
      gevonden++;
    }
  }

  if (gevonden == 0) {
    Serial.println("Geen I2C-apparaten gevonden");
  }
}

// =====================================
// WIFI VERBINDEN
// =====================================

bool verbindWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.println();
  Serial.println("=== WIFI VERBINDEN ===");

  Serial.print("Netwerk: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - start < 20000
  ) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WIFI MISLUKT");

    Serial.print("Statuscode: ");
    Serial.println((int)WiFi.status());

    return false;
  }

  Serial.println("WIFI VERBONDEN!");

  Serial.print("IP-adres: ");
  Serial.println(WiFi.localIP());

  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());

  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());

  Serial.print("Signaalsterkte: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  return true;
}

// =====================================
// SERVER VERBINDING TESTEN
// =====================================

bool testServer() {
  Serial.println();
  Serial.println("=== SERVERTEST ===");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("FOUT: geen wifi");
    return false;
  }

  IPAddress serverIP;

  Serial.print("DNS opzoeken: ");
  Serial.println(API_HOST);

  if (!WiFi.hostByName(API_HOST, serverIP)) {
    Serial.println("FOUT: DNS werkt niet");
    return false;
  }

  Serial.print("Server IP: ");
  Serial.println(serverIP);

  WiFiClient testClient;

  testClient.setTimeout(5000);

  Serial.println("TCP poort 443 testen...");

  if (testClient.connect(API_HOST, 443, 5000)) {
    Serial.println("TCP 443: VERBONDEN");
    testClient.stop();
    return true;
  }

  Serial.println("TCP 443: MISLUKT");

  Serial.println(
    "Controleer internet, firewall en server."
  );

  return false;
}

// =====================================
// INTERNETKLOK
// =====================================

bool synchroniseerTijd() {
  Serial.println();
  Serial.println("=== INTERNETKLOK ===");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Geen wifi");
    return false;
  }

  if (time(nullptr) >= 1735689600) {
    Serial.println("Internetklok is al ingesteld");
    return true;
  }

  IPAddress ntpIP;

  if (!WiFi.hostByName("pool.ntp.org", ntpIP)) {
    Serial.println("DNS van NTP-server mislukt");
    return false;
  }

  Serial.print("NTP server IP: ");
  Serial.println(ntpIP);

  if (!ntpGestart) {
    configTime(
      0,
      0,
      "pool.ntp.org",
      "time.google.com",
      "time.cloudflare.com"
    );

    ntpGestart = true;
  }

  unsigned long start = millis();

  while (millis() - start < 30000) {
    time_t nu = time(nullptr);

    if (nu >= 1735689600) {
      struct tm tijdInfo;

      gmtime_r(&nu, &tijdInfo);

      Serial.println();
      Serial.println("INTERNETKLOK OK!");

      Serial.printf(
        "UTC: %04d-%02d-%02d %02d:%02d:%02d\n",
        tijdInfo.tm_year + 1900,
        tijdInfo.tm_mon + 1,
        tijdInfo.tm_mday,
        tijdInfo.tm_hour,
        tijdInfo.tm_min,
        tijdInfo.tm_sec
      );

      return true;
    }

    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("INTERNETKLOK MISLUKT");

  return false;
}

// =====================================
// SENSOREN STARTEN
// =====================================

void initSensoren() {

#if TESTMODUS == 0

  if (!tempOK) {
    // Alleen interne pull-up: testopstelling.
    // Externe 4,7 kOhm blijft aanbevolen.
    pinMode(TEMP_PIN, INPUT_PULLUP);

    tempSensor.begin();
    tempSensor.setResolution(12);

    tempOK = tempSensor.getDeviceCount() > 0;
  }

  if (!lichtOK) {
    uint8_t adres = 0;

    if (i2cGevonden(0x23)) {
      adres = 0x23;
    } else if (i2cGevonden(0x5C)) {
      adres = 0x5C;
    }

    if (adres != 0) {
      lichtOK = lichtsensor.begin(
        BH1750::CONTINUOUS_HIGH_RES_MODE,
        adres,
        &Wire
      );
    }
  }

  if (!ensOK && i2cGevonden(ENS160_ADDR)) {
    if (ens160.begin() == 0) {
      ens160.setPWRMode(ENS160_STANDARD_MODE);
      ensOK = true;
    }
  }

  Serial.println();
  Serial.println("=== SENSOREN ===");

  Serial.print("DS18B20: ");
  Serial.println(tempOK ? "OK" : "FOUT");

  Serial.print("BH1750: ");
  Serial.println(lichtOK ? "OK" : "FOUT");

  Serial.print("ENS160: ");
  Serial.println(ensOK ? "OK" : "FOUT");

#endif
}

// =====================================
// METING
// =====================================

void meetEnVerstuur() {
  float temperatuur = NAN;
  float lux = NAN;

  int eco2 = -1;
  int ensStatus = -1;

  unsigned long metingNummer = teller++;

  Serial.println();
  Serial.println("==========================");
  Serial.print("NIEUWE METING - HUIS ");
  Serial.println(HUIS_NUMMER);
  Serial.println("==========================");

#if TESTMODUS == 1

  float fase = metingNummer * 0.20f;

  temperatuur =
    20.0f +
    HUIS_NUMMER * 1.5f +
    sinf(fase) * 1.5f;

  lux =
    150.0f +
    HUIS_NUMMER * 60.0f +
    sinf(fase * 0.8f) * 60.0f;

  eco2 =
    550 +
    HUIS_NUMMER * 100 +
    (int)(sinf(fase * 0.6f) * 120.0f);

  Serial.println("TESTWAARDEN ACTIEF");

#else

  if (!tempOK || !lichtOK || !ensOK) {
    initSensoren();
  }

  // DS18B20
  if (tempOK) {
    pinMode(TEMP_PIN, INPUT_PULLUP);

    tempSensor.requestTemperatures();

    float waarde = tempSensor.getTempCByIndex(0);

    if (
      isfinite(waarde) &&
      waarde != DEVICE_DISCONNECTED_C &&
      waarde >= -55 &&
      waarde <= 125
    ) {
      temperatuur = waarde;
    } else {
      tempOK = false;
    }
  }

  // BH1750
  if (lichtOK) {
    if (i2cGevonden(0x23) || i2cGevonden(0x5C)) {
      float waarde = lichtsensor.readLightLevel();

      if (
        isfinite(waarde) &&
        waarde >= 0 &&
        waarde <= 200000
      ) {
        lux = waarde;
      } else {
        lichtOK = false;
      }
    } else {
      lichtOK = false;
    }
  }

  // ENS160
  if (ensOK) {
    if (i2cGevonden(ENS160_ADDR)) {
      ensStatus = ens160.getENS160Status();

      if (ensStatus >= 0 && ensStatus <= 2) {
        int waarde = ens160.getECO2();

        if (waarde >= 400 && waarde <= 65000) {
          eco2 = waarde;
        }
      }
    } else {
      ensOK = false;
    }
  }

#endif

  // ===================================
  // WAARDEN TONEN
  // ===================================

  Serial.print("Temperatuur: ");

  if (isfinite(temperatuur)) {
    Serial.print(temperatuur, 2);
    Serial.println(" C");
  } else {
    Serial.println("GEEN WAARDE");
  }

  Serial.print("Lichtsterkte: ");

  if (isfinite(lux)) {
    Serial.print(lux, 1);
    Serial.println(" lux");
  } else {
    Serial.println("GEEN WAARDE");
  }

  Serial.print("eCO2: ");

  if (eco2 >= 400) {
    Serial.print(eco2);
    Serial.println(" ppm");
  } else {
    Serial.println("GEEN WAARDE");
  }

  Serial.print("ENS160 status: ");
  Serial.println(ensStatus);

  // ===================================
  // WIFI
  // ===================================

  if (!verbindWiFi()) {
    Serial.println("UPLOAD GESTOPT: geen wifi");
    return;
  }

  // ===================================
  // SERVERTEST
  // ===================================

  if (!testServer()) {
    Serial.println(
      "UPLOAD GESTOPT: server niet bereikbaar"
    );
    return;
  }

  // ===================================
  // INTERNETKLOK
  // ===================================

  if (!synchroniseerTijd()) {
    Serial.println(
      "UPLOAD GESTOPT: internetklok ongeldig"
    );
    return;
  }

  // ===================================
  // DATA OPBOUWEN
  // ===================================

  String sampleID =
    String(bootID) + "-" +
    String(metingNummer);

  String data =
    "device_id=" + String(DEVICE_ID) +
    "&sample_id=" + sampleID;

  data += "&temperature=";

  if (isfinite(temperatuur)) {
    data += String(temperatuur, 2);
  }

  data += "&light=";

  if (isfinite(lux)) {
    data += String(lux, 1);
  }

  data += "&eco2=";

  if (eco2 >= 400) {
    data += String(eco2);
  }

  data += "&is_test=";
  data += String(TESTMODUS);

  data += "&ens_status=";

  if (ensStatus >= 0 && ensStatus <= 3) {
    data += String(ensStatus);
  }

  // ===================================
  // HTTPS VERBINDING
  // ===================================

  NetworkClientSecure client;

  // Vereist een ESP32-core die deze
  // ingebouwde certificatenbundel ondersteunt.
  client.useBuiltinCACertBundle();

  client.setHandshakeTimeout(10);

  HTTPClient http;

  http.setConnectTimeout(5000);
  http.setTimeout(10000);

  http.setFollowRedirects(
    HTTPC_DISABLE_FOLLOW_REDIRECTS
  );

  if (!http.begin(client, API_URL)) {
    Serial.println(
      "HTTPS client kan niet starten"
    );
    return;
  }

  http.addHeader(
    "Content-Type",
    "application/x-www-form-urlencoded"
  );

  http.addHeader("X-API-Key", API_KEY);

  Serial.println();
  Serial.println("Versturen naar alwaysdata...");

  int httpCode = http.POST(data);

  Serial.print("HTTP code: ");
  Serial.println(httpCode);

  // ===================================
  // SERVERANTWOORD
  // ===================================

  if (httpCode > 0) {
    String antwoord = http.getString();

    Serial.print("Server antwoord: ");
    Serial.println(antwoord);

    bool opgeslagen =
      (httpCode == 200 || httpCode == 201) &&
      antwoord.indexOf("\"ok\":true") >= 0 &&
      antwoord.indexOf(
        "\"device_id\":\"" +
        String(DEVICE_ID) + "\""
      ) >= 0 &&
      antwoord.indexOf(
        "\"sample_id\":\"" +
        sampleID + "\""
      ) >= 0;

    if (opgeslagen) {
      Serial.println("METING OPGESLAGEN!");
    } else {
      Serial.println("UPLOAD NIET BEVESTIGD!");
    }

    if (httpCode == 401) {
      Serial.println("Verkeerde API-sleutel");
    } else if (httpCode == 404) {
      Serial.println(
        "api.php niet gevonden op alwaysdata"
      );
    } else if (httpCode == 500) {
      Serial.println(
        "PHP- of databasefout op server"
      );
    } else if (httpCode == 429) {
      Serial.println(
        "Te snel verzonden: wacht 60 seconden"
      );
    }

  } else {
    Serial.print("HTTP fout: ");
    Serial.println(
      HTTPClient::errorToString(httpCode)
    );

    if (httpCode == -1) {
      Serial.println(
        "Controleer TLS, DNS, netwerk en server"
      );
    }
  }

  http.end();
}

// =====================================
// SETUP
// =====================================

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("==========================");
  Serial.println("ESP32 SENSORSTATION");
  Serial.println("==========================");

  Serial.print("Huis: ");
  Serial.println(HUIS_NUMMER);

  Serial.print("Apparaat: ");
  Serial.println(DEVICE_ID);

  snprintf(
    bootID,
    sizeof(bootID),
    "%08lx%08lx",
    (unsigned long)esp_random(),
    (unsigned long)esp_random()
  );

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  scanI2C();
  initSensoren();

  if (verbindWiFi()) {
    testServer();
    synchroniseerTijd();
  }

  // Eerste meting onmiddellijk
  vorigeMeting = millis() - INTERVAL;
}

// =====================================
// LOOP
// =====================================

void loop() {
  if (millis() - vorigeMeting >= INTERVAL) {
    vorigeMeting = millis();
    meetEnVerstuur();
  }

  delay(10);
}