/*  
Universidad de Costa Rica
Escuela de Ingenieria Electrica

IE-0624 Laboratorio de Microcontroladores

main.c

Autor: Jonathan Rodriguez Hernandez <jonathan.rodriguezhernandez@ucr.ac.cr>.
Carnet: B76490.
Fecha: 28/8/2024.

Descripcion:
    Programa destinado a ser implementado en un microcontrolador pic12f683
    controla una tombola digital que puede producir hasta 10 numeros aleatorios
    antes de reiniciarse
*/
#include <pic14/pic12f683.h>


// Definiciones de pines
#define DATA_PIN   GP0 // Pin de datos (conectado a DS del 74HC595)
#define CLOCK_PIN  GP4 // Pin del reloj de desplazamiento (conectado a SH_CP del 74HC595)
#define LATCH_PIN  GP5 // Pin del reloj de almacenamiento (conectado a ST_CP del 74HC595)
#define DISPLAY1_PIN GP1 // Control del primer display
#define DISPLAY2_PIN GP2 // Control del segundo display
#define BUTTON_PIN GP3 // Pin del botón

// Declaacion de variables
// Segmentos para los números 0-9
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

unsigned int revolviendo = 0;
unsigned int resultado = 0;
unsigned int numeroSorteo = 0;
unsigned int posicion = 0;
unsigned int numerosSorteados[10] = {0};
unsigned int unidad = 0;
unsigned int decena = 0;

/*
Configuracion:
  _WDTE_OFF: Desactiva el watchdog Timer para evitar interrupciones inesperadas.
  _MCLRE_OFF: Desactiva la función MCLR, permitiendo el uso de GP3 como pin I/O.
*/
typedef unsigned int word;
word __at 0x2007 __CONFIG = (_WDTE_OFF & _MCLRE_OFF);

///////////////////////////////////////////////////
// Funciones
/*
    enviarDato: Se encarga del envio de datos
    al 74HC595 para encender los segmentos
    deseados
*/
void enviarDato(unsigned char dato) {
    for (int i = 0; i < 8; i++) {
        DATA_PIN = (dato & 0x80) >> 7; // Enviar el bit más significativo primero
        CLOCK_PIN = 1; // Pulso en el reloj de desplazamiento
        dato <<= 1;
        CLOCK_PIN = 0;
    }
}

/*
    mostrarNumero: Se encarga de intercambiar el
    encendido de las pantallas, asi como de 
    enviarles el numero deseado
    la velocidad de la permutacion da el efecto de 
    tener 2 digitos con la dos pantallas encendidadas
*/
void mostrarNumero(unsigned char numero, char display) {
    if (display == 1) {
        DISPLAY1_PIN = 1; // Activar el primer display
        DISPLAY2_PIN = 0; // Desactivar el segundo display
    } 
    if (display == 2) {
        DISPLAY1_PIN = 0; // Desactivar el primer display
        DISPLAY2_PIN = 1; // Activar el segundo display
    }
    if (display == 0) {
        DISPLAY1_PIN = 0; // Desactivar ambos displays
        DISPLAY2_PIN = 0; 
    }

    LATCH_PIN = 0; // Latch bajo para preparar el almacenamiento
    enviarDato(segmentos[numero]); // Enviar el dato correspondiente al número
    LATCH_PIN = 1; // Latch alto para actualizar el display
}


// numeroYaSorteado: Verifica que el numero no haya salido ya
int numeroYaSorteado(int numero, int numerosSorteados[]) {
    for (int i = 0; i < 11; i++) {
        if (numerosSorteados[i] == numero) {
            return 1; // Número ya sorteado
        }
    }
    return 0; // Número no sorteado
}

// Delay: Genera retrasos
void delay(unsigned int tiempo) {
    unsigned int i;
    unsigned int j;
    for(i = 0; i < tiempo; i++)
        for(j = 0; j < 1275; j++);
}

// parpadeo99: parpadea 3 veces en la pantalla el numero 99
void parpadeo99(){
    for (int i = 0; i < 3; i++)
    {
        mostrarNumero(9,1);
        delay(10);
        mostrarNumero(9,2);
        delay(10);
        mostrarNumero(0,0);
        delay(200);
    }
}


////////////////////////////////////////////////////////////
void main() {
    // Configuración de los pines como salida
    TRISIO = 0b00001000; // GP3 como entrada (botón), el resto como salidas
    ANSEL = 0x00; // Desactiva funciones analógicas
    CMCON0 = 0x07; // Desactiva comparadores
    GPIO = 0x00;   //Salidas inicialmente en 0

    while (1) {
        // Incrementar el número "revolviendo" constantemente
        revolviendo++;
        if (revolviendo > 99) {
            revolviendo = 0;
        }

        if (GP3){
            if(posicion<11 && numeroYaSorteado(resultado, numerosSorteados)){
                resultado = revolviendo;
                numerosSorteados[posicion] = resultado; 
                posicion++;
            }

            if (posicion==11)
            {
                for (int i = 0; i < 10; i++) {
                    numerosSorteados[i] = 0; // Establecer el elemento en cero
                }
                posicion = 0;
                resultado = 0;
                parpadeo99();
            }
        }

        decena = resultado / 10;
        unidad = resultado % 10;
        // Mostrar el número en los displays
        mostrarNumero(unidad, 2); // Mostrar unidades en el segundo display
        delay(10); // Tiempo de retardo en el display
        mostrarNumero(decena, 1); // Mostrar decenas en el primer display
        delay(10); // Tiempo de retardo en el display
    }
}
