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

#include <avr/io.h> // Libreria para manejo de puertos
#include <avr/interrupt.h> // Libreria para manejo de interrupciones

//////////////////////////////////////
//DEFINICIONES
/////////////////////////////////////
// Definicion de pines
#define LED_RED PB7
#define LED_BLUE PB6
#define LED_GREEN PB5
#define LED_YELLOW PB4
#define BUTTON_RED PB3    // Interrupcion del PIN PB3 
#define BUTTON_BLUE PB2   // interrupcion del PIN PB2
#define BUTTON_GREEN PB1  // Interrupcion del PIN PB1
#define BUTTON_YELLOW PB0 // Interrupcion del PIN PB0

// Definicion de variables
int button_red_pressed = 0;    // Variable para indicar si el boton rojo fue presionado
int button_blue_pressed = 0;   // Variable para indicar si el boton azul fue presionado
int button_green_pressed = 0;  // Variable para indicar si el boton verde fue presionado
int button_yellow_pressed = 0; // Variable para indicar si el boton amarillo fue presionado
int dos_cientos_ms = 0; // Contador de 200ms

// Definicion de estados (PROVISIONAL)
typedef enum {
  INIT,     // Estado inicial
  WAITING,  // Estado de espera
  PLAYING,  // Estado de juego
  WRONG     // Estado de error
} state;


//////////////////////////////////////
//FUNCIONES DE CONFIGURACION
/////////////////////////////////////
void setup_pins(){ // Funcion de configuracion de registros de pines
  // Configura los pines de los LEDs como salidas
  DDRB |= (1 << LED_RED) | (1 << LED_BLUE) | (1 << LED_GREEN) | (1 << LED_YELLOW);
  // Inicializa los estados de los pines  en LOW
  PORTB = 0x00;
}

void setup_button_interrupts() { // Funcion de configuracion de interrupciones por cambio de pin
  GIMSK |= (1 << PCIE0);  // Habilita las interrupciones en el grupo PCINT0-7 (puerto B)
  PCMSK |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3);  // Habilita interrupciones en PB0-PB3
}

void setup_timer0(){ // Funcion de configuracion de temporizador, se configura para generar interrupciones cada 200ms
  // Modo CTC (Clear Timer on Compare Match) genera una interrupcion cuando el contador llega al valor de OCR0A
  TCCR0A |=(1 << WGM01);  //0x00;//
  // Selecciona fuente de reloj (divide la frecuencia del reloj entre 1024)
  TCCR0B = (1 << CS00) | (1 << CS02); 
  // Valor de comparacion para que la cuenta sea de 200ms
  // Se calcula a partir del oscilador interno de 1MHz y el preescalador de 1024
  OCR0A = 100; // Se calcula para generar una interrupcion con una frecuencia de 5Hz (cada 200ms)
  // Habilita las interrupciones por comparacion  
  TIMSK |= (1 << OCIE0A); //(1 << TOIE0);//
  // Activa la bandera de interrucion por comparacion
  TIFR |= (1<<OCF0A);//(1 << TOV0); 
}

//////////////////////////////////////
//ATENCION DE INTERRUPCIONES
/////////////////////////////////////

ISR(TIMER0_COMPA_vect){//TIMER0_OVF_vect) { // Interrupcion del temporizador
  dos_cientos_ms++; // Incrementa el contador de medio segundo
}


//ISR(PCINT3_vect){
  //button_red_pressed = 1;
//}

ISR(PCINT2_vect){
  button_blue_pressed = 1;
}

ISR(PCINT1_vect){
  button_green_pressed = 1;
}

ISR(PCINT0_vect){
  button_yellow_pressed = 1;
}
  


//////////////////////////////////////
//FUNCIONES DE JUEGO
/////////////////////////////////////
void led_on(int led){ // Funcion para encender un LED. FUNCIONA
  PORTB |= (1 << led);
}

void led_off(int led){ // Funcion para apagar un LED
  PORTB &= ~(1 << led);
}

void all_leds_on(){ // Funcion para encender todos los LEDs. FUNCIONA
  PORTB |= (1 << LED_RED) | (1 << LED_BLUE) | (1 << LED_GREEN) | (1 << LED_YELLOW);
}

void all_leds_off(){ // Funcion para apagar todos los LEDs
  PORTB &= ~(1 << LED_RED) & ~(1 << LED_BLUE) & ~(1 << LED_GREEN) & ~(1 << LED_YELLOW);
}

int main(){
  sei(); // Habilita las interrupciones
  setup_pins(); // Configura los pines de los LEDs
  setup_timer0(); // Configura el temporizador
  dos_cientos_ms = 0;
  while(1){
  
    if (dos_cientos_ms <100) { // 1 segundo ha pasado (5 x 200 ms)
        all_leds_on(); // Enciende todos los LEDs
    } 
    if(dos_cientos_ms >100 && dos_cientos_ms <200){ // 2 segundos han pasado (10 x 200 ms)
        all_leds_off(); // Apaga todos los LEDs
    }
    if (dos_cientos_ms >200)
    {
      dos_cientos_ms = 0; // Reinicia el contador de 200ms
    }
    

  }
 
}