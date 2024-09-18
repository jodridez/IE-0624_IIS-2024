/*
Universidad de Costa Rica
Escuela de Ingenieria Electrica

IE-0624 Laboratorio de Microcontroladores

main.c

Autores: Jonathan Rodriguez Hernandez <jonathan.rodriguezhernandez@ucr.ac.cr>.

Carnets: B76490.
         
Fecha: 18/9/2024.

Descripcion:
Desarrolla un juego de memoria llamado Simon dice, utiliza 4 leds, 4 botones y el microcontrolador ATtiny4313. 
En este juego hay 4 botones que corresponden a 4 LEDs de colores distintos, la idea consiste en memorizar una 
secuencia aleatoria de luces para reproducirla posteriormente.

Al principio del juego se van encendiendo unos LEDs en un orden y se debe memorizar cuales LEDs se han encendido y en cual
orden para reproducir la secuencia. La dificultad va en aumento conforme se va avanzando de modo que al principio se
empieza por memorizar cuatro led y posteriormente se va incrementando el numero de LEDSs a memorizar. 

Conforme se va incrementando la secuencia el tiempo que duran los LEDs encendidos se va reduciendo tambien, al inicio cada LED
dura 2 segundos encendidos, posteriormente por cada incremento de la secuencia se debe reducir este tiempo en 200ms 
(con base a esto se tiene un maximo de 9 niveles de dificultad).

Para iniciar el juego se puede presionar cualquier boton y como indicacion para el usuario, se parpadean todos los LEDs
2 veces. El juego finaliza cuando el usuario se equivoca en la secuencia, momento en el cual aparece una indicacion que
consiste en el parpadeo de todos los LEDs 3 veces.

La temporizacion de los LEDs se realiza utilizando los temporizadores del microcontrolador. Para hacer la lectura
de los botones, como tambien del fin de cuenta del timer, se usan interrupciones.

Se utiliza un modelo de maquina de estados para la programacion del microcontrolador.
*/


#include <avr/io.h>        // Libreria para manejo de puertos
#include <avr/interrupt.h> // Libreria para manejo de interrupciones

//////////////////////////////////////
// DEFINICIONES
//////////////////////////////////////
#define LED_RED PB7
#define LED_BLUE PB6
#define LED_GREEN PB5
#define LED_YELLOW PB4


// Variables globales
volatile uint8_t button_red_pressed = 0;   // Variable para indicar si el boton rojo fue presionado
volatile uint8_t button_blue_pressed = 0;   // Variable para indicar si el boton azul fue presionado
volatile uint8_t button_green_pressed = 0;   // Variable para indicar si el boton verde fue presionado
volatile uint8_t button_yellow_pressed = 1;
volatile uint8_t dos_cientos_ms = 0;       // Contador de 200 ms
volatile uint8_t led_on_time = 0;          // Tiempo que el LED rojo debe estar encendido

//////////////////////////////////////
// FUNCIONES DE CONFIGURACION
//////////////////////////////////////
void setup_pins() {
    // Configura los pines de los LEDs como salidas
    DDRB |= (1 << LED_RED) | (1 << LED_BLUE) | (1 << LED_GREEN) | (1 << LED_YELLOW);
    // Inicializa los LEDs apagados
    PORTB &= ~(1 << LED_RED) | (1 << LED_BLUE) | (1 << LED_GREEN) | (1 << LED_YELLOW);
}

void setup_button_interrupts() {
    // Configura interrupciones externas en INT0 (PB3)
    GIMSK |= (1 << INT0) | (1 << INT1) | (1 << PCIE0) | (1 << PCIE1);  // Habilita interrupción externa
    MCUCR |= (1 << ISC00) | (1 << ISC10); // Interrupción por cualquier cambio en el pin
    
    PCMSK |= (1 << PCINT0); // Habilita la interrupción para el pin PC0 (PCINT16)
    PCMSK1 |= (1 << PCINT8); // Habilita la interrupción para el pin PC1 (PCINT17)
}

void setup_timer0() {
    // Configuración del Timer0 en modo CTC (Clear Timer on Compare Match)
    TCCR0A |= (1 << WGM01); // Modo CTC
    // Prescaler de 1024 para el temporizador
    TCCR0B |= (1 << CS00) | (1 << CS02);
    // Valor de comparación para generar interrupciones cada 200 ms
    OCR0A = 100; // Basado en un reloj de 1 MHz y prescaler de 1024
    // Habilita las interrupciones por comparación
    TIMSK |= (1 << OCIE0A);
}

//////////////////////////////////////
// INTERRUPCIONES
//////////////////////////////////////

// Interrupción del Timer0
ISR(TIMER0_COMPA_vect) {
    if (led_on_time > 0) {
        dos_cientos_ms++;
        if (dos_cientos_ms >= led_on_time) {
            PORTB = (0 << LED_RED) | (0 << LED_BLUE) ; // Apaga el LED rojo
            dos_cientos_ms = 0;
            led_on_time = 0; // Resetea el tiempo de encendido
        }
    }
}

// Interrupción del botón rojo (INT0)
ISR(INT0_vect) {
    button_red_pressed = 1; // Marca que se presionó el botón rojo
}

ISR(INT1_vect) {
    button_blue_pressed = 1; // Marca que se presionó el botón rojo
}

ISR(PCINT0_vect) {
    button_green_pressed = 1; // Marca que se presionó el botón rojo
}

ISR(__vector_PCINT8_vect) {
    button_yellow_pressed = 1; // Marca que se presionó el botón rojo
}




/////////////////////////////////////
// FUNCIONES DE CONTROL
//////////////////////////////////////
void dificultad_led(int led, int nivel) {
    int reduccion_tiempo = 27;              // Reducción de tiempo por nivel (9 niveles)
    led_on_time = 250 - (nivel * reduccion_tiempo); // 250 Equivale a 200 ms
    PORTB |= (1 << led);                    // Enciende el LED
    dos_cientos_ms = 0;                     // Reinicia el contador de 200 ms
}

int main() {
    sei();              // Habilita interrupciones globales
    setup_pins();       // Configura los pines de los LEDs
    setup_button_interrupts(); // Configura las interrupciones de los botones
    setup_timer0();     // Configura el temporizador

    while (1) {
        if (button_red_pressed == 1) {
            dificultad_led(LED_RED, 0); // Enciende el LED rojo en el nivel 0
            button_red_pressed = 0;     // Resetea la variable del botón
        }

        if (button_blue_pressed == 1) {
        dificultad_led(LED_BLUE, 0); // Enciende el LED rojo en el nivel 0
        button_blue_pressed = 0;     // Resetea la variable del botón
        }

        if (button_green_pressed == 1) {
        dificultad_led(LED_GREEN, 0); // Enciende el LED rojo en el nivel 0
        button_green_pressed = 0;     // Resetea la variable del botón
        }

        if(button_yellow_pressed == 1) {
        dificultad_led(LED_YELLOW, 0); // Enciende el LED rojo en el nivel 0
        button_yellow_pressed = 0;     // Resetea la variable del botón
        }

}
}
