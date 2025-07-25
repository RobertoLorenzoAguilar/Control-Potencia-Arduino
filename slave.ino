/**
  ******************************************************************************
  * @alumno         : Roberto Aguilar
  * @materia        : Cómputo Autónomo
  * @version        : 1.0
  * @date           : 14/05/25
  * @details        : Este código permite controlar dos motores brushless usando ESCs
  *                   manipulados mediante potenciómetros conectados a un microcontrolador.
  *                   También incorpora comunicación serial con un módulo XBee para
  *                   recibir alertas que detienen los motores.
  *
  *                   - Control de ESCs mediante señales PWM
  *                   - Lectura de potenciómetros vía ADC
  *                   - Comunicación serial por software con módulo XBee
  *                   - Gestión de estado de alerta y reinicio
  ******************************************************************************
  */
/* USER CODE END Header */

#include <Servo.h>              // Librería para controlar servos o ESCs
#include <SoftwareSerial.h>     // Librería para comunicación serial por software

// Definición del puerto serial por software para comunicar con XBee
SoftwareSerial XBee(10, 11);    // RX = pin 10, TX = pin 11

// Pines donde están conectados los ESCs
const int esc1Pin = 5;          // Pin del ESC 1
const int esc2Pin = 6;          // Pin del ESC 2

// Pines analógicos donde están conectados los potenciómetros
const int pot1Pin = A0;         // Potenciómetro para ESC 1
const int pot2Pin = A5;         // Potenciómetro para ESC 2

// Objetos Servo que representan los ESCs
Servo esc1;
Servo esc2;

// Variable para controlar el estado de alerta
bool enAlerta = false;

void setup() {
  Serial.begin(115200);         // Inicializa la consola serial
  XBee.begin(115200);           // Inicializa el puerto serial para XBee

  esc1.attach(esc1Pin);         // Asocia el objeto esc1 al pin definido
  esc2.attach(esc2Pin);         // Asocia el objeto esc2 al pin definido

  // Enviar señal mínima para calibrar ESCs (modo seguro)
  esc1.writeMicroseconds(1000);
  esc2.writeMicroseconds(1000);

  Serial.println("🔧 Control de ESC con potenciómetros y XBee");
}

void loop() {
  // Revisión de mensajes entrantes por XBee
  if (XBee.available()) {
    String recibido = XBee.readStringUntil('\n');  // Lee hasta salto de línea
    recibido.trim();                               // Elimina espacios extras

    if (recibido == "ALERTA") {                    // Si se recibe una alerta
      enAlerta = true;                             // Cambia a modo de alerta
      detenerMotores();                            // Detiene los motores
      Serial.println("ALERTA recibida. Motores detenidos.");
    }
  }

  // Permite liberar la alerta desde la consola serial
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');     // Lee comando ingresado
    cmd.trim();

    if (cmd == "RESET") {                          // Si el usuario escribe "RESET"
      enAlerta = false;                            // Se reestablece el control
      Serial.println("Alerta liberada. Control reactivado.");
    }
  }

  // Si no hay alerta, se leen los potenciómetros y se controla la velocidad
  if (!enAlerta) {
    int pot1Value = analogRead(pot1Pin);           // Lee valor del potenciómetro 1
    int pot2Value = analogRead(pot2Pin);           // Lee valor del potenciómetro 2

    // Mapea el valor de 0–1023 a 1000–2000 microsegundos (señal PWM)
    int esc1Signal = map(pot1Value, 0, 1023, 1000, 2000);
    int esc2Signal = map(pot2Value, 0, 1023, 1000, 2000);

    esc1.writeMicroseconds(esc1Signal);            // Aplica señal PWM a ESC 1
    esc2.writeMicroseconds(esc2Signal);            // Aplica señal PWM a ESC 2

    // Imprime los valores actuales de PWM para monitoreo
    Serial.print("ESC1: ");
    Serial.print(esc1Signal);
    Serial.print(" | ESC2: ");
    Serial.println(esc2Signal);
  }

  delay(20); // Control a 50 Hz (20 ms por ciclo)
}

// Función para detener ambos motores (PWM en 1000 µs)
void detenerMotores() {
  esc1.writeMicroseconds(1000);
  esc2.writeMicroseconds(1000);
}
