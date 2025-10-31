/*
 Resumen del codigo:
  1. Detecta presencia a < 20 cm con el sensor ultrasonico.
  2. Si hay una presencia, intenta leer un tag.
  3. Si el tag es "Permitido", abre el servo de la tapa a 90°.
  4. Si el tag es "Denegado" o desconocido, el servo de la tapa se mantiene en 0°.
  5. Si el plato está abierto y el sensor deja de detectar presencia,
  espera 5 segundos y cierra el servo de la tapa a 0°.
  6. El servo del comedero si activara a las 8AM, 2PM y 8PM.
 */

// --- LIBRERIAS ---
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "RTClib.h"

// --- OBJETOS ---
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
Servo tapa;
Servo comedero;
MFRC522 rfid(10, 9);

// --- PINES ---
#define SERVO_TAPA 4
#define SERVO_COM 7
#define POS_CERRADO 0
#define POS_ABIERTO 90

int TRIG_PIN = 2;
int ECHO_PIN = 3;
int DISTANCIA_DETECCION = 20;

#define TIEMPO_ESPERA_CIERRE 5000 /
#define INTERVALO_SENSOR 100     
#define INTERVALO_LCD 1000       

// --- IDs ---
byte idPermitido[4] = {0xB5, 0x35, 0xA7, 0x00};
byte idDenegado[4]  = {0xA8, 0xD7, 0xD9, 0x12};

// --- VARIABLES ---
char daysOfTheWeek[7][12] = { "Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab" };

long duracion;
int distancia;

bool platoAbierto = false;
bool comidaDispensada = false;

unsigned long ultimaDeteccion = 0;
unsigned long tiempoSensorAnterior = 0;
unsigned long tiempoLCDAnterior = 0;


void setup() {
  Serial.begin(9600);

  // --- RELOJ ---
  if (!rtc.begin()) {
    Serial.println(F("No se encuentra el RTC"));
    lcd.print("Error: RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // --- PANTALLA ---
  lcd.init();
  lcd.backlight();

  // --- SERVOS ---
  tapa.attach(SERVO_TAPA);
  tapa.write(POS_CERRADO);
  comedero.attach(SERVO_COM);
  comedero.write(0); 

  platoAbierto = false;
  
  // --- RFID ---
  SPI.begin();
  rfid.PCD_Init();

  // --- SENSOR ---
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.setCursor(0, 0);
  lcd.print("Comedero listo");
  lcd.setCursor(0, 1);
  lcd.print("Esperando...");
  delay(1500); 
  lcd.clear();
}

void loop() {
  
  DateTime now = rtc.now();
  unsigned long tiempoActual = millis();

  // --- DISPENSAR COMIDA (RELOJ) ---
  if ((now.hour() == 8 || now.hour() == 14 || now.hour() == 20) && now.minute() == 0) {
    if (!comidaDispensada) {
      Serial.println("HORA DE COMER: Dispensando...");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dispensando");
      lcd.setCursor(0, 1);
      lcd.print("comida...");
      
      comedero.write(90);
      delay(1000);
      
      comidaDispensada = true; 
      delay(1500); 
      lcd.clear();
    }
  }
  
  if (now.minute() == 1 && comidaDispensada == true) {
    Serial.println("Comida Dispensada");
    comidaDispensada = false;
  }


  // --- SENSOR ULTRASÓNICO ---
  if (tiempoActual - tiempoSensorAnterior > INTERVALO_SENSOR) {
    tiempoSensorAnterior = tiempoActual;

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(1000);
    digitalWrite(TRIG_PIN, LOW);

    duracion = pulseIn(ECHO_PIN, HIGH);

    // Calcular la distancia en centímetros
    distancia = duracion * 0.0343 / 2;

    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");
  }

  // --- DETECCIÓN DE MASCOTA (SENSOR ULTRASÓNICO) ---
  
  bool gatoPresente = (distancia < DISTANCIA_DETECCION && distancia > 0);

  if (gatoPresente) {
    ultimaDeteccion = tiempoActual; 

    if (!platoAbierto) {  // Si hay un gato presente y el plato esta cerrado, lee el Tag
      lcd.setCursor(0, 0);
      lcd.print("Gato detectado");
      lcd.setCursor(0, 1);
      lcd.print("Leyendo tag...");

      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        
        bool aceptado = true;
        for (byte i = 0; i < rfid.uid.size; i++) {
          if (rfid.uid.uidByte[i] != idPermitido[i]) {
            aceptado = false;
            break; 
          }
        }

        if (aceptado == true) {
          Serial.println("Tag Permitido. Abriendo tapa.");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Acceso Permitido");
          tapa.write(POS_ABIERTO);
          platoAbierto = true;
          delay(1500); 
          
        } 
        else {
          Serial.println("Tag Denegado.");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Acceso Denegado");
          delay(1500); 
        }
        
        rfid.PICC_HaltA(); 
        rfid.PCD_StopCrypto1();
      }
      
    } else {   // Si hay un gato presente y el plato esta abierto, esta comiendo
      lcd.setCursor(0, 0);
      lcd.print("Gato comiendo..."); 
    }
    
  }
  else {   // Si NO hay un gato presente
    if (platoAbierto) {   // Si NO hay un gato presente y el plato esta abierto, cierra la tapa
      
      if (tiempoActual - ultimaDeteccion > TIEMPO_ESPERA_CIERRE) {// Cerrando el plato
        Serial.println("El gato se fue. Cerrando tapa.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Cerrando tapa...");
        tapa.write(POS_CERRADO);
        platoAbierto = false;
        delay(1000); 
        
      }
      else {   // Aviso de cierre
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("El gato se fue");
        lcd.setCursor(0, 1);
        lcd.print("Cerrando en 5s");
      }
      
    }
    
    else {   // Mensaje de reposo
      
      if (tiempoActual - tiempoLCDAnterior > INTERVALO_LCD) {
        tiempoLCDAnterior = tiempoActual;
        
        lcd.setCursor(0, 0);
        lcd.print(now.day(), DEC);
        lcd.print('/');
        lcd.print(now.month(), DEC);
        lcd.print('/');
        lcd.print(now.year(), DEC);
        lcd.print(" (");
        lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
        lcd.print(") ");
        
        lcd.setCursor(0, 1);
        if (now.hour() < 10) lcd.print('0');
        lcd.print(now.hour(), DEC);
        lcd.print(':');
        if (now.minute() < 10) lcd.print('0');
        lcd.print(now.minute(), DEC);
        lcd.print(':');
        if (now.second() < 10) lcd.print('0');
        lcd.print(now.second(), DEC);
        lcd.print("       "); 
      }
    }
  }
}