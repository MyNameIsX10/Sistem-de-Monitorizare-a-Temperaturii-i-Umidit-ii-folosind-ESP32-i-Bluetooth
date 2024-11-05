#include <BLEDevice.h> 
#include <BLEUtils.h> 
#include <BLEServer.h> 
#include <DHT.h> 


#define DHTPIN 15       
#define DHTTYPE DHT11  


#define SERVICE_UUID        "12345678-1234-1234-1234-123456789012" 
#define CHARACTERISTIC_UUID "abcdefab-1234-1234-1234-abcdefabcdef" 

DHT dht(DHTPIN, DHTTYPE);
BLECharacteristic *pCharacteristic; 
bool deviceConnected = false; 


class MyServerCallbacks: public BLEServerCallbacks {
      deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);

  
  dht.begin();


  BLEDevice::init("MY_ESP32");  
  BLEServer *pServer = BLEDevice::createServer(); 
  pServer->setCallbacks(new MyServerCallbacks()); 

  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
                    );


  pService->start(); 
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start(); 

  Serial.println("BLE pornit");
}

void loop() {
 
  float temperatura = dht.readTemperature();
  float umiditate = dht.readHumidity();


  if (isnan(temperatura) || isnan(umiditate)) {
    Serial.println("Eroare la citirea de la senzorul DHT11");
    return;
  }

  
  String data = "Temperatura: " + String(temperatura) + " °C, Umiditate: " + String(umiditate) + " %";

  
  Serial.println(data);

 
  if (deviceConnected) {
    pCharacteristic->setValue(data.c_str());  
    pCharacteristic->notify();               
  }

  
  delay(2000);
}
