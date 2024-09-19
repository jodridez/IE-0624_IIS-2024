#include <avr/io.h>        // Librería para manejo de puertos
#include <avr/interrupt.h> // Librería para manejo de interrupciones
#include <stdlib.h>        // Librería para funciones de utilidad
#include <time.h>          // Librería para funciones de tiempo
#include <util/delay.h>    // Librería para retardos

//////////////////////////////////////
// DEFINICIONES
//////////////////////////////////////
#define LED_RED PB7
#define LED_BLUE PB6
#define LED_GREEN PB5
#define TWO_SECONDS 266 
#define TWO_MILI_SECONDS 29
// Definición de estados
typedef enum {
    WAITING,
    INIT,
    GENERATE_SEQUENCE,
    SHOW_SEQUENCE,
    USER_INPUT,
    CHECK_SEQUENCE,
    GAME_OVER
} state;

// Variables globales
int contador = 0; // Contador de tiempo
int button_red_pressed = 0;   // Variable para indicar si se presionó el botón rojo
int button_blue_pressed = 0;  // Variable para indicar si se presionó el botón azul
int button_green_pressed = 0; // Variable para indicar si se presionó el botón verde
int sequence_game[9]; // Secuencia de juego 3 + 6 niveles
int level = 0; // Nivel actual del juego
state estado; // Estado inicial de la máquina de estados

//////////////////////////////////////
// FUNCIONES DE CONFIGURACIÓN
//////////////////////////////////////
void setup_pins() {
    // Configura los pines de los LEDs como salidas
    DDRB |= (1 << LED_RED) | (1 << LED_BLUE) | (1 << LED_GREEN);
    // Inicializa los LEDs apagados
    PORTB &= ~((1 << LED_RED) | (1 << LED_BLUE) | (1 << LED_GREEN));
}

void setup_button_interrupts() {
    // Configura interrupciones externas en INT0 (PB3) e INT1 (PB2)
    GIMSK |= (1 << INT0) | (1 << INT1) | (1 << PCIE0); // Habilita interrupción externa
    MCUCR |= (1 << ISC00) | (1 << ISC10); // Interrupción por cualquier cambio en los pines

    PCMSK |= (1 << PCINT0); // Habilita la interrupción para el pin PB0 (botón verde)
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
// ISR del temporizador para manejar los LEDs
ISR(TIMER0_COMPA_vect) {
    contador++;
}

// Interrupción del botón rojo (INT0)
ISR(INT0_vect) {
    if (contador > 50) {
        button_red_pressed = 1; // Marca que se presionó el botón rojo
    }
}

// Interrupción del botón azul (INT1)
ISR(INT1_vect) {
    if (contador > 50) {
        button_blue_pressed = 1; // Marca que se presionó el botón azul
    }
}

// Interrupción del botón verde (PCINT0)
ISR(PCINT0_vect) {
    if (contador > 50) {
        button_green_pressed = 1; // Marca que se presionó el botón verde
    }
}

//////////////////////////////////////
// FUNCIONES
//////////////////////////////////////
void all_leds_on() {
    PORTB |= (1 << LED_RED) | (1 << LED_BLUE) | (1 << LED_GREEN);
}

void all_leds_off() {
    PORTB &= ~((1 << LED_RED) | (1 << LED_BLUE) | (1 << LED_GREEN));
}

void generate_sequence(int* sequence_game, int level) {
    // Genera una secuencia aleatoria de 3 + 6 niveles
    for (int i = 0; i < 3 + level; i++) {
        sequence_game[i] = rand() % 3; // Números aleatorios entre 0 y 2
    }
}

void show_sequence(int* sequence_game, int level) {
    for (int i = 0; i < 3 + level; i++) {
        // Apaga todos los LEDs
        all_leds_off();
        contador = 0;
        while(contador<TWO_SECONDS -level*TWO_MILI_SECONDS){
          // Enciende el LED correspondiente
          if (sequence_game[i] == 0) {
            PORTB |= (1 << LED_RED);
          } 
          if (sequence_game[i] == 1) {
            PORTB |= (1 << LED_BLUE);
          }
          if (sequence_game[i] == 2) {
            PORTB |= (1 << LED_GREEN);
          }
        }
        contador = 0;
    }
    // Apaga todos los LEDs después de mostrar la secuencia
    all_leds_off();
}


// Función para manejar la máquina de estados
void FSM() {
    switch (estado) {
      case WAITING:
        // Espera a que se presione un botón
        if (button_red_pressed || button_blue_pressed || button_green_pressed) {
          contador = 0;
          estado = INIT;
        }
      break;

      case INIT:
        for (int i = 0; i < 2; i++) {
          while (contador < 200) {
            if (contador < 100) { // 1 segundo ha pasado (5 x 200 ms)
              all_leds_on(); // Enciende todos los LEDs
            }
            if (contador >= 100 && contador < 200) { // 2 segundos han pasado (10 x 200 ms)
              all_leds_off(); // Apaga todos los LEDs
            }
          }
            contador = 0;
        }
        estado = GENERATE_SEQUENCE; // Cambiar a GENERATE_SEQUENCE para crear una nueva secuencia
      break;

      case GENERATE_SEQUENCE:
        //generate_sequence(sequence_game, level);
        estado = SHOW_SEQUENCE;
      break;

      case SHOW_SEQUENCE:
      sequence_game[0] = 0;
      sequence_game[1] = 1;
      sequence_game[2] = 2;
        show_sequence(sequence_game, level);
        estado = USER_INPUT;
      break;

      default:
          break;
    }
}

int main() {
    sei();               // Habilita interrupciones globales
    setup_pins();        // Configura los pines de los LEDs
    setup_button_interrupts(); // Configura las interrupciones de los botones
    setup_timer0();      // Configura el temporizador
    contador = 0;        // Inicializa el contador de tiempo
    estado = WAITING;    // Inicializa la máquina de estados
    level = 0;           // Inicializa el nivel del juego
    srand(time(NULL));   // Inicializa la semilla para números aleatorios

    while (1) {
        FSM(); // Ejecuta la máquina de estados
    }
}
