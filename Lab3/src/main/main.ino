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
#define V1_pin A3 // Puerto analogico para la tension V1
#define V2_pin A2 // Puerto analogico para la tension V2
#define V3_pin A1 // Puerto analogico para la tension V3
#define V4_pin A0 // Puerto analogico para la tension V4

/*
 Declaracion de constantes
*/
#define V1_MAX 20 // Valor maximo de la tension V1
#define V2_MAX 20 // Valor maximo de la tension V2
#define V3_MAX 20 // Valor maximo de la tension V3
#define V4_MAX 20 // Valor maximo de la tension V4

#define V1_MIN -20 // Valor minimo de la tension V1
#define V2_MIN -20 // Valor minimo de la tension V2
#define V3_MIN -20 // Valor minimo de la tension V3
#define V4_MIN -20 // Valor minimo de la tension V4

/*
 Declaracion tensiones
*/
float V1; // Valor de la tension V1
float V2; // Valor de la tension V2
float V3; // Valor de la tension V3
float V4; // Valor de la tension V4

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

void setup() {
  // put your setup code here, to run once:
  lcd.begin();
  

}

void loop() {
  // put your main code here, to run repeatedly:
  V1 = analogRead(V1_pin);
  V2 = 0;
  V3 = 0;
  V4 = 0;

  pantalla_lcd(V1, V2, V3, V4, 0);

}