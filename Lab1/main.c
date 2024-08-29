/*  
Universidad de Costa Rica
Escuela de Ingenieria Electrica

IE-0624 Laboratorio de Microcontroladores

main.c

Autor: Jonathan Rodriguez Hernandez <jonathan.rodriguezhernandez@ucr.ac.cr>.
Carnet: B76490.
Fecha: 28/8/2024.

Descripcion:
*/


#include <pic14/pic12f683.h>

/*
Configuracion:
  _WDTE_OFF: Desactiva el watchdog Timer para evitar interrupciones inesperadas.
*/
typedef unsigned int word;
word __at 0x2007 __CONFIG = (_WDTE_OFF);

#define DATA_PIN   GP0 // Pin de datos (conectado a DS del 74HC595)
#define CLOCK_PIN  GP4 // Pin del reloj de desplazamiento (conectado a SH_CP del 74HC595 )
#define LATCH_PIN  GP5 // Pin del reloj de almacenamiento (conectado a  ST_CP del 74HC595)
#define DISPLAY1_PIN GP1 // Control del primer display
#define DISPLAY2_PIN GP2 // Control del segundo display
#define BUTTON_PIN GP3 // Pin del botón

// Definición de los segmentos para los números 0-9
const char segmentos[10] = {
    0b00111111, // 0 -> a,b,c,d,e,f
    0b00000110, // 1 -> b,c
    0b01011011, // 2 -> a,b,d,e,g
    0b01001111, // 3 -> a,b,c,d,g
    0b01100110, // 4 -> b,c,f,g
    0b01101101, // 5 -> a,c,d,f,g
    0b01111101, // 6 -> a,c,d,e,f,g
    0b00000111, // 7 -> a,b,c
    0b01111111, // 8 -> a,b,c,d,e,f,g
    0b01101111  // 9 -> a,b,c,d,f,g
};

void enviarDato(unsigned char dato) {
    for (int i = 0; i < 8; i++) {
        DATA_PIN = (dato & 0x80) >> 7; // Enviar el bit más significativo primero
        CLOCK_PIN = 1; // Pulso en el reloj de desplazamiento
        dato <<= 1;
        CLOCK_PIN = 0;
    }
}

void mostrarNumero(unsigned char numero, char display) {
    if (display == 1) {
        DISPLAY1_PIN = 1; // Activar el primer display
        DISPLAY2_PIN = 0; // Desactivar el segundo display
    } 
    if (display==2) {
        DISPLAY1_PIN = 0; // Desactivar el primer display
        DISPLAY2_PIN = 1; // Activar el segundo display
    }
    if (display== 0){
        DISPLAY1_PIN = 0; // Desactivar el primer display
        DISPLAY2_PIN = 0; // Activar el segundo display
    }

    LATCH_PIN = 0; // Latch bajo para preparar el almacenamiento
    enviarDato(segmentos[numero]); // Enviar el dato correspondiente al número
    LATCH_PIN = 1; // Latch alto para actualizar el display
}

// Delay: Genera retrasos
void delay(unsigned int tiempo) {
    unsigned int i;
    unsigned int j;
    for(i = 0; i < tiempo; i++)
        for(j = 0; j < 1275; j++);
}

// Función para parpadear el número 99 tres veces
void parpadear99() {
    for (int i = 0; i < 3; i++) {
        mostrarNumero(9, 1); // Mostrar 9 en el primer display
        delay(10);
        mostrarNumero(9, 2); // Mostrar 9 en el segundo display
        delay(10); // Mantener el número en pantalla durante medio segundo

        mostrarNumero(0, 0); // Apagar el primer display
        delay(50);
    }
}

unsigned int numerosSorteados[10] = {0}; // Arreglo para registrar números sorteados
unsigned int cantidadSorteados = 0;
unsigned int resultado = 0;
unsigned int unidad = 0;
unsigned int decena = 0;

void main() {
    // Configuración de los pines como salida
    TRISIO = 0b00000000; // Poner todos los pines como salidas
    ANSEL = 0x00; // Desactivar funciones analógicas
    CMCON0 = 0x07; // Desactivar comparadores


    while (1) {
        /*
        if(!GP3){
            while(!GP3){
                resultado++;
                if (resultado>99)
                {
                    resultado = 99;
                }
            }
        }
        */

        resultado = 82;
        decena = resultado/10;
        unidad = resultado%10;

        // Mostrar los números en los displays
        mostrarNumero(unidad, 2); // Mostrar unidades en el segundo display
        delay(10); // Tiempo de retardo en el display
        mostrarNumero(decena, 1); // Mostrar decenas en el primer display
        delay(10); // Tiempo de retardo en el display
    }
}