/*******************************************************************************
  * @alumno         : Roberto Aguilar
 * @maestro        : Dr. Jesús Pacheco
 * @materia        : Cómputo Autónomo
  * @version        : 1.0
  * @date           : 08/05/25
  * @details        : Este código implementa un sistema de monitoreo y control de
  *                   posición basado en un encoder de 8 bits y un sistema de
  *                   alertas para mantener la barra en una posición deseada.
  *
  *                   - Lectura de señales de un encoder rotatorio (8 pines de entrada)
  *                   - Cálculo de ángulo de posición basado en una tabla predefinida
  *                   - Establecimiento de límites de ángulo mínimo y máximo
  *                   - Comunicación serial para recibir comandos de actualización de límites
  *                   - Envío de alertas cuando la posición se sale de los límites
  *                   - Uso de SoftwareSerial para comunicación con dispositivo esclavo
  *
  ******************************************************************************/
  
#include <SoftwareSerial.h>  // Incluye la librería para comunicación serial por SoftwareSerial

const int pins[8] = {2, 3, 4, 5, 6, 7, 8, 9};  // Definición de los pines de entrada para el encoder
const float ANGLE_RES = 360.0 / 128.0;  // Resolución del ángulo (360° dividido por 128 posiciones)

float ANGLE_MIN = -20.0;  // Ángulo mínimo predeterminado
float ANGLE_MAX = 20.0;   // Ángulo máximo predeterminado

SoftwareSerial alertaSerial(10, 11);  // Configura el puerto serial para la comunicación con un dispositivo esclavo (RX, TX)

const int acetab[128] = {  // Tabla predefinida con códigos de posición del encoder
  127, 63, 62, 58, 56, 184, 152, 24, 8, 72, 73, 77, 79, 15, 47, 175,
  191, 159, 31, 29, 28, 92, 76, 12, 4, 36, 164, 166, 167, 135, 151, 215,
  223, 207, 143, 142, 14, 46, 38, 6, 2, 18, 82, 83, 211, 195, 203, 235,
  239, 231, 199, 71, 7, 23, 19, 3, 1, 9, 41, 169, 233, 225, 229, 245,
  247, 243, 227, 163, 131, 139, 137, 129, 128, 132, 148, 212, 244, 240, 242, 250,
  251, 249, 241, 209, 193, 197, 196, 192, 64, 66, 74, 106, 122, 120, 121, 125,
  253, 252, 248, 232, 224, 226, 98, 96, 32, 33, 37, 53, 61, 60, 188, 190,
  254, 126, 124, 116, 112, 113, 49, 48, 16, 144, 146, 154, 158, 30, 94, 95
};

void setup() {
  Serial.begin(115200);  // Inicializa la comunicación serial a 115200 baudios
  alertaSerial.begin(9600);  // Inicializa la comunicación serial con el dispositivo esclavo a 9600 baudios
  for (int i = 0; i < 8; i++) pinMode(pins[i], INPUT);  // Configura los pines de entrada para el encoder
  Serial.println("Sistema iniciado");  // Muestra mensaje de inicio en el monitor serial
}

void loop() {
  // Procesar comandos por Serial
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');  // Lee el comando del monitor serial
    input.trim();  // Elimina espacios en blanco al inicio y final
    procesarComando(input);  // Procesa el comando recibido
  }

  // Leer encoder y calcular ángulo
  uint8_t code = readEncoderCode();  // Lee el código del encoder
  int pos = findPosition(code);  // Busca la posición correspondiente al código del encoder
  if (pos == -1) return;  // Si no se encuentra la posición, se termina la función

  float angle = pos * ANGLE_RES;  // Calcula el ángulo en función de la posición
  if (angle > 180.0) angle -= 360.0;  // Ajusta el ángulo si es mayor a 180° (para mantenerlo entre -180° y 180°)

  // Imprime el ángulo calculado y los límites en el monitor serial
  Serial.print("ANG: ");
  Serial.print(angle, 2);
  Serial.print("°, Límite: ");
  Serial.print(ANGLE_MIN);
  Serial.print(" a ");
  Serial.println(ANGLE_MAX);

  // Verifica si el ángulo está fuera de los límites y envía alerta si es necesario
  if (angle < ANGLE_MIN || angle > ANGLE_MAX) {
    alertaSerial.println("ALERTA");  // Envia alerta al dispositivo esclavo
    Serial.println("ALERTA");  // Muestra alerta en el monitor serial
  }

  delay(100);  // Pausa de 100 ms antes de la siguiente lectura
}

void procesarComando(String comando) {
  // Procesa los comandos para actualizar los valores de ángulo mínimo y máximo
  if (comando.startsWith("MIN:")) {
    ANGLE_MIN = comando.substring(4).toFloat();  // Actualiza el ángulo mínimo
    Serial.print("Ángulo mínimo actualizado: ");
    Serial.println(ANGLE_MIN);
  } else if (comando.startsWith("MAX:")) {
    ANGLE_MAX = comando.substring(4).toFloat();  // Actualiza el ángulo máximo
    Serial.print("Ángulo máximo actualizado: ");
    Serial.println(ANGLE_MAX);
  } else {
    Serial.println("Comando inválido. Usa MIN:valor o MAX:valor");  // Muestra un mensaje de error si el comando no es válido
  }
}

uint8_t readEncoderCode() {
  uint8_t code = 0;
  for (int i = 0; i < 8; i++) {  // Lee el estado de los pines del encoder
    if (digitalRead(pins[i]) == HIGH) {  // Si el pin está alto, ajusta el código
      code |= (1 << i);  // Suma el valor del pin al código
    }
  }
  return code;  // Devuelve el código del encoder
}

int findPosition(uint8_t code) {
  for (int i = 0; i < 128; i++) {  // Busca la posición en la tabla acetab
    if (acetab[i] == code) return i;  // Si se encuentra el código, devuelve la posición
  }
  return -1;  // Si no se encuentra el código, devuelve -1
}
