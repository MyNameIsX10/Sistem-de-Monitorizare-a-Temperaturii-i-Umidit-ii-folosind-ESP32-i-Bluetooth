#include <BLEDevice.h> // Initializeaza esp32 ca dispozitiv BLE
#include <BLEUtils.h> // Oferă funcții pentru conversia UUID-urilor din format binar în format text sau invers
#include <BLEServer.h> // îți permite să creezi servicii BLE pe care le definești în cod.
#include <DHT.h> // Libraria pt senzorul "DHT"

// Definim pinii și tipul de senzor
#define DHTPIN 15       // Pinul unde este conectat senzorul DHT11
#define DHTTYPE DHT11   // Tipul de senzor (DHT11)

// Definim UUID-urile pentru BLE
//Un UUID unic care identifică serviciul BLE. Un serviciu poate grupa mai multe caracteristici, fiecare corespunzând unui set de date.
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789012" // Un UUID unic care identifică serviciul BLE. Un serviciu poate grupa mai multe caracteristici, fiecare corespunzând unui set de date.
#define CHARACTERISTIC_UUID "abcdefab-1234-1234-1234-abcdefabcdef" // Un UUID pentru caracteristica specifică din cadrul serviciului BLE, care va stoca și trimite datele senzorului (temperatură și umiditate).

DHT dht(DHTPIN, DHTTYPE); // Creăm un obiect dht care inițializează senzorul 
BLECharacteristic *pCharacteristic; //Un pointer care va fi utilizat pentru a accesa și manipula caracteristica BLE ce va stoca datele senzorului.
bool deviceConnected = false; // Variabila pt a urmari daca un dispozitiv este conectat

// Definim un callback pentru conexiunile BLE
// Aceste callback-uri ne ajută să gestionăm comportamentul BLE atunci când dispozitivele mobile se conectează sau deconectează.
class MyServerCallbacks: public BLEServerCallbacks { // Aceasta este o clasă care moștenește BLEServerCallbacks și definește două funcții
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
// Configurarea BLE și inițializarea senzorului în setup()
void setup() {
  Serial.begin(115200);

  // Inițializăm senzorul DHT
  dht.begin();

  // Inițializăm BLE
  BLEDevice::init("MY_ESP32");   // Numele dispozitivului BLE
  BLEServer *pServer = BLEDevice::createServer(); // Creează un server BLE pe ESP32
  pServer->setCallbacks(new MyServerCallbacks()); // Adaugă callback-urile pentru a detecta când un dispozitiv se conectează sau se deconectează

  // Definim un serviciu și o caracteristică BLE
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Pornim serviciul și începem publicarea (advertising)
  pService->start(); // Pornește serviciul BLE.
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising(); // Creează o caracteristică în cadrul serviciului BLE. Aceasta caracteristică poate fi citită (PROPERTY_READ) și poate trimite notificări (PROPERTY_NOTIFY).
  pAdvertising->start(); // Începe să publice (advertising) faptul că ESP32 este disponibil pentru conexiuni BLE.

  Serial.println("BLE a pornit și este gata să trimită date.");
}

void loop() {
  // Citim datele de la senzor
  float temperatura = dht.readTemperature();
  float umiditate = dht.readHumidity();

  // Verificăm dacă datele sunt valide
  if (isnan(temperatura) || isnan(umiditate)) {
    Serial.println("Eroare la citirea de la senzorul DHT11");
    return;
  }

  // Pregătim datele pentru trimitere
  String data = "Temperatura: " + String(temperatura) + " °C, Umiditate: " + String(umiditate) + " %";

  // Afișăm datele pe monitorul serial (opțional)
  Serial.println(data);

  // Actualizăm caracteristica BLE doar dacă dispozitivul este conectat
  if (deviceConnected) {
    pCharacteristic->setValue(data.c_str());  // Convertim datele în șir de caractere
    pCharacteristic->notify();                // Trimitere prin BLE (notificare)
  }

  // Pauză între citiri
  delay(2000);
}
