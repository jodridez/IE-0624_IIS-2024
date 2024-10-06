/*  
Universidad de Costa Rica
Escuela de Ingenieria Electrica

IE-0624 Laboratorio de Microcontroladores

main.ino.hex

Autor: Jonathan Rodriguez Hernandez <jonathan.rodriguezhernandez@ucr.ac.cr>.
Carnet: B76490.
Fecha: 6/10/2024.

Descripcion:
  Programa arduino, que utiliza la pantalla LCD PCD8544, para mostrar 4 valores de tensiones AC o DC en un rango de [-24, 24] V.
  Los valores de tensiones analogicas son leidos de los pines A0, A1, A2 y A3.
  Los valores de tensiones analogicas se muestran en la pantalla LCD PCD8544.
*/

#include <PCD8544.h> // Para utilizar la pantalla LCD PCD8544

/////////////////////////////////////////
//Definciones de variables
/////////////////////////////////////////
/*
 Declaracion de puertos analogicos
*/
#define V1_PIN A3 // Puerto analogico para la tension V1
#define V2_PIN A2 // Puerto analogico para la tension V2
#define V3_PIN A1 // Puerto analogico para la tension V3
#define V4_PIN A0 // Puerto analogico para la tension V4

/*
 Definicion de puertos digitales
*/
#define LED1_PIN 0 // Puerto digital para el led 1 
#define LED2_PIN 1 // Puerto digital para el led 2
#define LED3_PIN 2 // Puerto digital para el led 3
#define LED4_PIN 8 // Puerto digital para el led 4

/*
 Declaracion tensiones
*/
float V1; // Valor de la tension V1
float V2; // Valor de la tension V2
float V3; // Valor de la tension V3
float V4; // Valor de la tension V4

/*
Instancia de la pantalla LCD
*/
PCD8544 lcd;

/////////////////////////////////////////
//Funciones de configuracion
/////////////////////////////////////////
//Funcion para evitar mucho copy-paste. genera la estructura para inmprimir en pantalla
void print_V( int modo, int line, String Vstring, float V){
  if(modo == 0){
    lcd.setCursor(0, line);
    lcd.print(Vstring);
    lcd.print(V);
    lcd.print("Vrms");
  } else {
    lcd.setCursor(0, line);
    lcd.print(Vstring);
    lcd.print(V);
    lcd.print("V");
  }
}

// Funcion para configurar la impresion en pantalla LCD
void pantalla_lcd(float V1, float V2, float V3, float V4, int modo) {
  lcd.setPower(HIGH); // Enciende la pantalla LCD
  if(modo == 0){
    lcd.setCursor(0,0);
    lcd.print("AC");
    
    // [V1] Mostrar el valor de la tension V1
    print_V(modo, 1, "V1: ", V1);
    // [V2] Mostrar el valor de la tension V2
    print_V(modo, 2, "V2: ", V2);
    // [V3] Mostrar el valor de la tension V3
    print_V(modo, 3, "V3: ", V3);
    // [V4] Mostrar el valor de la tension V4
    print_V(modo, 4, "V4: ", V4);
  } else {
    lcd.setCursor(0,0);
    lcd.print("DC");

    // [V1] Mostrar el valor de la tension V1
    print_V(modo, 1, "V1: ", V1);
    // [V2] Mostrar el valor de la tension V2
    print_V(modo, 2, "V2: ", V2);
    // [V3] Mostrar el valor de la tension V3
    print_V(modo, 3, "V3: ", V3);
    // [V4] Mostrar el valor de la tension V4
    print_V(modo, 4, "V4: ", V4);
  }
}

// Convierte el valor leido a tension
float tension(int PIN)
{ float valor_leido = analogRead(PIN);
  float tension_condicionada = (valor_leido * 5) / 1023.0;
  float tension = ((tension_condicionada - 1.54) / 3.45) * 48 - 24; 
  return tension;
}

// Gestiona los LEDs
void leds(int LED_PIN, float V) {
  if (V < -20 || V > 20) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void setup() {
  // put your setup code here, to run once:
  lcd.begin();
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Leer las tensiones
  V1 = tension(V1_PIN);
  V2 = 0;
  V3 = 0;
  V4 = 0;

  // Mostrar las tensiones en la pantalla
  pantalla_lcd(V1, V2, V3, V4, 0);

  // Gestionar los LEDs
  leds(LED1_PIN, V1);
  leds(LED2_PIN, V2);
  leds(LED3_PIN, V3);
  leds(LED4_PIN, V4);
}