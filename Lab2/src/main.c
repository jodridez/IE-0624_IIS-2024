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

// Definicion de estados
typedef enum {
    WAITING,
    INIT,
    SHOW_SEQUENCE,
    USER_INPUT,
    CHECK_SEQUENCE,
    GAME_OVER
} state;


// Variables globales
volatile uint8_t button_red_pressed = 0;   // Variable para indicar si el boton rojo fue presionado
volatile uint8_t button_blue_pressed = 0;   // Variable para indicar si el boton azul fue presionado
volatile uint8_t button_green_pressed = 0;   // Variable para indicar si el boton verde fue presionado
volatile uint8_t button_yellow_pressed = 1;
volatile uint8_t dos_cientos_ms = 0;       // Contador de 200 ms
volatile uint16_t led_on_time = 0;           // Tiempo de encendido del LED en dificultad
volatile uint8_t blink_counter = 0;          // Contador para parpadeo de todos los LEDs
volatile uint8_t total_blinks = 0;           // Número total de parpadeos
volatile uint8_t led_state = 0;              // Estado actual del parpadeo (encendido/apagado)
volatile uint8_t parpadeo_intervalo = 0;    // Número de interrupciones para 2 segundos (10 interrupciones de 200 ms)
state estado = 0;                 // Estado actual de la máquina de estados
volatile uint8_t nivel = 0; // Nivel de dificultad
volatile uint8_t lenght = 4; // Longitud de la secuencia
volatile uint8_t revolviendo = 0; // Variable para controlar la secuencia de LEDs
int sequence_game[9]; // Secuencia de LEDs a memorizar 4 + 5 niveles de dificultad
int sequence_user[9]; // Secuencia de LEDs ingresada por el usuario

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
    PCMSK1 |= (1 << PCINT8); 
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
    if (led_on_time > 0) {
        dos_cientos_ms++;  // Incrementa el contador para el tiempo de encendido
        if (dos_cientos_ms >= led_on_time) {
            // Apaga los LEDs al completar el tiempo
            PORTB &= ~(1 << LED_RED) & ~(1 << LED_BLUE) & ~(1 << LED_GREEN) & ~(1 << LED_YELLOW);
            dos_cientos_ms = 0;
            led_on_time = 0;  // Resetea el tiempo de encendido
        }
    }

    // Manejo del parpadeo de todos los LEDs
    static uint8_t parpadeo_contador = 0;  // Contador para controlar los ciclos de parpadeo
    if (blink_counter < total_blinks) {
        parpadeo_contador++;
        if (parpadeo_contador >= parpadeo_intervalo) {
            if (led_state == 0) {
                // Enciende todos los LEDs
                PORTB |= (1 << LED_RED) | (1 << LED_BLUE) | (1 << LED_GREEN) | (1 << LED_YELLOW);
                led_state = 1;
            } else {
                // Apaga todos los LEDs
                PORTB &= ~(1 << LED_RED) & ~(1 << LED_BLUE) & ~(1 << LED_GREEN) & ~(1 << LED_YELLOW);
                led_state = 0;
                blink_counter++;  // Solo incrementa el contador cuando se apagan los LEDs
            }
            parpadeo_contador = 0;  // Reinicia el contador
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
// Función para manejar la dificultad y encender un solo LED durante un tiempo reducido
void dificultad_led(int led, int nivel) {
    int reduccion_tiempo = 27;              // Reducción de tiempo por nivel (9 niveles)
    led_on_time = 250 - (nivel * reduccion_tiempo); // 250 ms para nivel 0
    PORTB |= (1 << led);                    // Enciende el LED especificado
    dos_cientos_ms = 0;                     // Reinicia el contador de tiempo
}

// Función para parpadear todos los LEDs un número específico de veces
void blink_all_leds(int times) {
    total_blinks = times;  // Configura el número de veces que deben parpadear los LEDs
    blink_counter = 0;     // Reinicia el contador de parpadeos
    led_state = 0;         // Inicia con los LEDs apagados
    parpadeo_intervalo = 100;  // Configura el número de interrupciones necesarias para 2 segundos (10 interrupciones de 200 ms)
}

int random(){ // Genera un número aleatorio entre 0 y 3
          revolviendo++;
        if (revolviendo > 3) {
            revolviendo = 0;
        }

  return revolviendo%4;
}

// Generar secuencia aleatoria de LEDs
void generate_sequence(int *sequence, int length) {
    for (int i = 0; i < length; i++) {
      sequence[i] = random();  // Genera un número aleatorio entre 0 y 3
    }
}

void show_sequence(int *sequence, int length) {
    for (int i = 0; i < length; i++) {
        switch (sequence[i]) {
            case 0:
                dificultad_led(LED_RED, i);
                break;
            case 1:
                dificultad_led(LED_BLUE, i);
                break;
            case 2:
                dificultad_led(LED_GREEN, i);
                break;
            case 3:
                dificultad_led(LED_YELLOW, i);
                break;
            default:
                break;
        }
    }
}

// Función para recebir la secuencia del usuario
void get_user_sequence(int *sequence, int length) {
    for (int i = 0; i < length; i++) {
        // Espera a que el usuario presione un botón
        while (button_red_pressed == 0 && button_blue_pressed == 0 && button_green_pressed == 0) {
            // Espera a que el usuario presione un botón
        }
        // Registra el botón presionado en la secuencia
        if (button_red_pressed == 1) {
            sequence[i] = 0;
        } else if (button_blue_pressed == 1) {
            sequence[i] = 1;
        } else if (button_green_pressed == 1) {
            sequence[i] = 2;
        } else if (button_yellow_pressed == 1) {
            sequence[i] = 3;
        }
        // Reinicia las variables de los botones
        button_red_pressed = 0;
        button_blue_pressed = 0;
        button_green_pressed = 0;
        button_yellow_pressed = 0;
    }
}


// Función para verificar si la secuencia ingresada por el usuario es correcta
int check_sequence(int *sequence, int length) {
    for (int i = 0; i < length; i++) {
        // Espera a que el usuario presione un botón
        while (button_red_pressed == 0 && button_blue_pressed == 0 && button_green_pressed == 0 && button_yellow_pressed == 0) {
            // Espera a que el usuario presione un botón
        }
        // Verifica si el botón presionado coincide con la secuencia
        if ((button_red_pressed == 1 && sequence[i] == 0) || (button_blue_pressed == 1 && sequence[i] == 1) || (button_green_pressed == 1 && sequence[i] == 2) || (button_yellow_pressed == 1 && sequence[i] == 3)) {
            // Reinicia las variables de los botones
            button_red_pressed = 0;
            button_blue_pressed = 0;
            button_green_pressed = 0;
            button_yellow_pressed = 0;
        } else {
            return 0;  // Retorna 0 si la secuencia es incorrecta
        }
    }
    return 1;  // Retorna 1 si la secuencia es correcta
}

// Función para reiniciar las variables del juego
void reset_game() {
    button_red_pressed = 0;
    button_blue_pressed = 0;
    button_green_pressed = 0;
    button_yellow_pressed = 0;
    dos_cientos_ms = 0;
    led_on_time = 0;
    blink_counter = 0;
    total_blinks = 0;
    led_state = 0;
    parpadeo_intervalo = 0;
    revolviendo = 0;
}

// Función para manejar la máquina de estados
void FSM(){
  switch (estado)
  {
  case WAITING:
    if(button_red_pressed == 1 || button_blue_pressed == 1 || button_green_pressed == 1) {
      estado = INIT;
    }
  break;
  
  case INIT:
    blink_all_leds(2);
    estado = SHOW_SEQUENCE;
  break;

  case SHOW_SEQUENCE:
    // Mostrar secuencia
    generate_sequence(sequence_game, lenght);  // Genera una secuencia aleatoria de 4 LEDs
    show_sequence(sequence_game, lenght);  // Muestra la secuencia de LEDs
    estado = USER_INPUT;
  break;


   
  default:
    break;
  }
}

int main() {
    sei();              // Habilita interrupciones globales
    setup_pins();       // Configura los pines de los LEDs
    setup_button_interrupts(); // Configura las interrupciones de los botones
    setup_timer0();     // Configura el temporizador
    estado = WAITING;    // Inicializa la máquina de estados
    lenght = 4; // Longitud de la secuencia

    while (1) {
        FSM();

}
}
