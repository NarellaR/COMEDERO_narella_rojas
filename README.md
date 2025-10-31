# COMEDERO_narella_rojas
# 🐱 Comedero Inteligente y Automático para Gatos (Arduino)

Este proyecto es un comedero automático para mascotas desarrollado en Arduino UNO, diseñado para solucionar el problema de múltiples mascotas con diferentes dietas.

El sistema utiliza un módulo de reloj (RTC) para dispensar comida en horarios fijos para todas las mascotas, y un lector RFID para controlar qué mascota específica puede acceder a la comida, abriendo una tapa protectora.


## 📋 Funcionalidades Principales

* **Dispensado por Horario:** El servo del dispensador ('comedero') se activa automáticamente a las 8:00 AM, 2:00 PM y 8:00 PM para soltar una porción de comida.
* **Control de Acceso por RFID:** Un lector RFID (MFRC522) escanea el tag del collar de la mascota.
* **Tapa de Acceso Selectivo:** Un servomotor ('tapa') abre el plato solo si se detecta un tag RFID autorizado ('idPermitido').
* **Detección de Presencia:** Un sensor ultrasónico (HC-SR04) detecta si la mascota está cerca del plato.
* **Cierre Automático:** Si la tapa está abierta y el sensor deja de detectar a la mascota, la tapa se cierra automáticamente después de un período de gracia de 5 segundos.
* **Feedback Visual:** Una pantalla LCD I2C (16x2) muestra la hora actual, el estado del sistema ("Esperando", "Gato detectado", "Acceso Permitido/Denegado") y las alertas.

## ⚙️ Componentes (Hardware)

* **Microcontrolador:** Arduino UNO
* **Sensores:**
    * Sensor Ultrasónico HC-SR04
    * Lector RFID MFRC522
    * Módulo de Reloj en Tiempo Real (RTC) DS3231
* **Actuadores:**
    * 1x Servomotor (para la 'tapa')
    * 1x Servomotor (para el dispensador 'comedero')
* **Interfaz:**
    * Pantalla LCD 16x2 con módulo I2C
    * Tags RFID
* **Otros:**
    * Protoboard y Jumpers (cables)
    * **Importante:** Una fuente de alimentación externa (5V 2A) para alimentar los servomotores, evitando sobrecargar el Arduino.

## 🛠️ Lógica de Funcionamiento

El 'loop()' principal opera como una máquina de estados no bloqueante (usando 'millis()') para asegurar que el reloj, el sensor y el lector RFID funcionen simultáneamente.

1.  **Estado de Reposo:**
    * La tapa está cerrada ('POS_CERRADO').
    * El LCD muestra la fecha y hora actual obtenidas del RTC.
    * El sistema comprueba dos cosas constantemente:
        1.  ¿Es la hora de dispensar comida? (Lógica RTC)
        2.  ¿Hay un gato cerca? (Lógica Sensor)

2.  **Lógica de Dispensado (RTC)**
    * Si la hora es 8:00, 14:00 o 20:00 (y el minuto es 0), el servo 'comedero' gira para soltar la comida en el plato.
    * Esto ocurre independientemente de si hay un gato presente. La tapa permanece cerrada.

3.  **Lógica de Acceso (Sensor + RFID)**
    * **Paso 1 (Detección):** El HC-SR04 detecta una presencia a menos de 20 cm ('gatoPresente = true').
    * **Paso 2 (Verificación):** Si el gato está presente Y la tapa está cerrada ('!platoAbierto'), el LCD pide "Acerque el tag...".
    * **Paso 3 (Decisión):**
        * Si se escanea el 'idPermitido', el servo `tapa` gira a 90° ('POS_ABIERTO'), el LCD muestra "Acceso Permitido" y se activa 'platoAbierto = true'.
        * Si se escanea el 'idDenegado' (u otro tag), el LCD muestra "Acceso Denegado" y la tapa permanece cerrada.
    * **Paso 4 (Comiendo):** Mientras 'gatoPresente' sea 'true' y 'platoAbierto' sea 'true', el LCD muestra "Gato comiendo..." y el temporizador de cierre se reinicia.
    * **Paso 5 (Cierre):** Si el gato se va ('gatoPresente = false') y la tapa estaba abierta ('platoAbierto = true'), se inicia el temporizador de 5 segundos.
        * El LCD muestra "Cerrando en 5s...".
        * Si el gato no vuelve en esos 5 segundos, la 'tapa' vuelve a 0° ('POS_CERRADO') y 'platoAbierto' se pone en 'false'.


## 🧪 Simulación (Wokwi)

Este proyecto fue prototipado en Wokwi. Dado que Wokwi no simula el MFRC522, la lógica RFID fue reemplazada por dos botones (Pines 11 y 12) que actúan como "Tag Permitido" y "Tag Denegado" para probar el flujo del programa.

(En proceso).
