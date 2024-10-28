/*  
Universidad de Costa Rica
Escuela de Ingenieria Electrica

IE-0624 Laboratorio de Microcontroladores

lab4_main.c

Autor: Jonathan Rodriguez Hernandez <jonathan.rodriguezhernandez@ucr.ac.cr>.
Carnet: B76490.
Fecha: 30/10/2024.

Descripcion:
    Se desarrolla un sismografo digital para registrar las oscilaciones en un edificio. 
    Este sismografo tiene alimentacion por bateria y poco ancho de banda. 
    para el envio de datos. 
Caracteristicas:
    1. Utilizala placa STM32F429 Discovery kit y la biblioteca libopencm3.
    2. Lee los ejes del giroscopio (X,Y,Z).
    3. Incluye un switch/boton para habilitar/deshabilitar comunicaciones por USART/USB.
    4. Un LED parpadea indicando la habilitacion de la transmision/recepcion de datos por el periferico USART/USB.
    5. Lee el nivel de la baterıa cuyo rango es de [0,9]V, en caso de estar cerca del limite minimo de operacion del
    microcontrolador(7 V) enciende un LED de alarma parpadeante y envia la notificacion de bateria baja al
    dashboard de thingsboard.
    6. Desplega en la pantalla LCD el nivel de bateria, los valores de los ejes X,Y,Z y si la comunicacion serial/USB
    esta habilitada.
*/

// Librerias usadas en el ejemplo de lcd-serial
#include <stdio.h> // Para funciones de entrada y salida

#include <stdint.h> // Para definiciones de tipos de datos

#include <math.h> // Para funciones matematicas
 // Librerias del ejemplo de lcd-serial
#include "clock.h" // Para configuracion de reloj

#include "console.h" // Para funciones de consola

#include "sdram.h" // Para funciones de SDRAM

#include "lcd-spi.h" // Para funciones de LCD

#include "gfx.h"  // Para funciones de graficos

//Librerias del ejemplo spi-mems
#include <libopencm3/stm32/rcc.h>

#include <libopencm3/stm32/gpio.h>

#include <libopencm3/stm32/spi.h>

// Librerias del ejemplo adc-dac-printf
#include <libopencm3/stm32/adc.h>

// Librerias usadas en el ejemplo usart
#include <libopencm3/stm32/usart.h>


// Definiciones relacionadas con el giroscopio
#define READ 0x80 // Para hacer la mascara OR con el registro de lectura 
//  Registros de configuracion del giroscopio
#define CTRL_REG1 0x20 //  Control de ejes
#define CTRL_REG2 0x21 // Configuracion de filtro paso alto
#define CTRL_REG4 0x23 // Configuracion de DPS y modo SPI (USAR MODO ESCLAVO)
//  Registros de lectura  de los ejes. Se dividen en parte alta y baja, para cada eje. Y se expresan en complemento a 2
#define OUT_X_L 0x28
#define OUT_X_H 0x29

#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B

#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D

//  Definiciones para configurar el giroscopio}
//  Para el registro CTRL_REG1 ( Control de ejes)
//#define DR1    0 << 7 // Frecuencia de datos
//#define DR0    0 << 6 // Frecuencia de datos
//#define BW1    0 << 5 // Ancho de banda
//#define BW0    0 << 4 // Ancho de banda
#define PD 1 << 3 // Modo de encendido
#define ZEN 1 << 2 // Habilitar eje Z
#define XEN 1 << 1 // Habilitar eje X
#define YEN 1 << 0 // Habilitar eje Y
//  Para el registro CTRL_REG2 (Filtro paso alto)
#define HPM1 0 << 5 // Seleccion de (modo modo normal)
#define HPM0 0 << 4 // Seleccion de modo
#define HPCF3 0 << 3 // Frencuencia de corte (La mas alta)
#define HPCF2 0 << 2 // Frecuencia de corrte
#define HPCF1 0 << 1 // Frecuencia de corrte
#define HPCF0 0 << 0 // Frecuencia de corrte
//  Para el registro CTRL_REG4 Configuracion de DPS y modo SPI
#define BDU 1 << 7 // Bloqueo de datos. No se actualizan los registros hasta que se lean
#define BLE 0 << 6 // Fin de datos. Los datos se leen en el orden de registro
#define FS1 1 << 5 // Escala completa 2000 DPS, para detectar solo movimientos bruscos
#define FS0 1 << 4 // Escala completa
#define SIM 0 << 0 // Modo SPI (Modo esclavo)

// Sensibilidad del giroscopio
#define SENSITIVITY 0.08

// Estructura para almacenar las lecturas de los ejes X, Y, y Z del giroscopio
typedef struct axis {
  int16_t x;
  int16_t y;
  int16_t z;
}
axis;

/*////////////////////////////////////////////////////////////////////////////////////////
    P   R   O   T   O   T   I   P   O   S
*/ ///////////////////////////////////////////////////////////////////////////////////////
void reg_write(uint16_t reg, uint16_t val); // Función para escribir en un registro
uint8_t reg_read(uint8_t reg); // Función para leer un registro

void gyro_setup(void); // Función para configurar el giroscopio
//void adc_setup(void); // Función para configurar el ADC  (Convertidor Analógico-Digital)
void setup(void); // Función para configurar el microcontrolador

int16_t axis_read(uint8_t out_l_reg, uint8_t out_m_reg); // Función para leer un eje del giroscopio
axis xyz_read(void); // Función para leer valores de los ejes X, Y y Z

//uint16_t read_adc_naiive(uint8_t channel); // Función para leer un canal del ADC
void display_data(axis measurement, float battery_lvl, bool send); // Función para desplegar los datos en la pantalla LCD

/*///////////////////////////////////////////////////////////////////////////////////////
    D   E   F   I   N   I   C   I   O   N   E   S   -   F   U   N   C   I   O   N   E   S
*/ //////////////////////////////////////////////////////////////////////////////////////
// Funcion para ESCRIBIR en un registro
void reg_write(uint16_t reg, uint16_t val) {
  //CS para el giroscopio corresponde al pin 01 del puerto C
  gpio_clear(GPIOC, GPIO1); // 1 Bajar CS del giroscpio para comenzar 

  spi_send(SPI5, reg); // 2 Se indica cual registro se quiere leer, 
  spi_read(SPI5); // 3 Se lee la respuesta

  spi_send(SPI5, val); // 4 Se indica cual es el valor para el registro 
  spi_read(SPI5); // 5 Se lee la respuesta

  gpio_set(GPIOC, GPIO1); // 6 Se pone en alto CS
}

// Funcion para LEER en un registro 
uint8_t reg_read(uint8_t command) {
  gpio_clear(GPIOC, GPIO1); // 1 Bajar CS del giroscpio para comenzar 

  spi_send(SPI5, command); // 2 Se indica cual registro se quiere leer
  spi_read(SPI5); // 3 Se lee la respuesta

  spi_send(SPI5, 0); // 4 Se envia un 0
  uint8_t result = spi_read(SPI5); // 5 Se lee la respuesta
  gpio_set(GPIOC, GPIO1); // 6 Se pone en alto CS
  return result; // Devuelve el resultado leído
}

// Función para configurar el módulo SPI5 y GPIOs relacionados
void gyro_setup(void) {
  /*Configuracion de pines giroscopio, extraido de spi-mems*/
  rcc_periph_clock_enable(RCC_GPIOF | RCC_GPIOC); // Habilita el reloj de los puertos F y C

  //// Configura los pines 7, 8 y 9 del puerto F para funciones alternas (SCK, MISO, MOSI de SPI)
  gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN,
    GPIO7 | GPIO8 | GPIO9);
  // Establece la función alternativa 5 para los pines 7, 8 y 9 del puerto F 
  gpio_set_af(GPIOF, GPIO_AF5, GPIO7 | GPIO8 | GPIO9);
  // Establece las opciones de salida para los pines 7 y 9 del puerto F
  gpio_set_output_options(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ,
    GPIO7 | GPIO9);

  /* Chip select line */
  gpio_set(GPIOC, GPIO1); // Establece en alto el pin 1 del puerto C (CS del giroscopio)
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1); // Configura el pin 1 del puerto C como salida

  // Habilita el reloj para SPI5
  rcc_periph_clock_enable(RCC_SPI5);

  // Configuración de SPI5. Siguiendo la guia
  spi_set_master_mode(SPI5); // Establece SPI5 en modo maestro
  spi_set_baudrate_prescaler(SPI5, SPI_CR1_BAUDRATE_FPCLK_DIV_4); // Configura la velocidad de baudios de SPI5. Extraido de spi-mems
  spi_set_clock_polarity_0(SPI5); // Configura la polaridad del reloj a 0
  spi_set_clock_phase_0(SPI5); // Configura la fase del reloj a 0
  spi_set_full_duplex_mode(SPI5); // Establece SPI5 en modo full duplex
  spi_set_unidirectional_mode(SPI5); // Establece SPI5 en modo unidireccional (pero con 3 cables)
  spi_enable_software_slave_management(SPI5); // Habilita la gestión de esclavo por software
  spi_send_msb_first(SPI5); // Establece la transmisión de bits empezando por el más significativo
  spi_set_nss_high(SPI5); // Establece el pin NSS (Chip Select) en alto
  spi_enable(SPI5); // Habilita SPI5

  //Configuracion de registros del giroscopio
  //Para el registro CTRL_REG1 (Control de ejes y poder)
  reg_write(CTRL_REG1, PD | ZEN | XEN | YEN);
  //Para el registro CTRL_REG2 (Filtro paso alto)
  reg_write(CTRL_REG2, HPM1 | HPM0 | HPCF3 | HPCF2 | HPCF1 | HPCF0);
  //Para el registro CTRL_REG4 (Configuracion de DPS y SPI)
  reg_write(CTRL_REG4, BDU | BLE | FS1 | FS0 | SIM);
}

// Función para configurar el ADC (Convertidor Analógico-Digital) extraido de adc-dac-printf.c
static void adc_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOA); // Habilita el reloj del puerto A
  rcc_periph_clock_enable(RCC_ADC1); // Habilita el reloj del ADC1

  // Configura los pines 5 del puerto A como analógicos
  gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO5);

  adc_power_off(ADC1); // Apaga el ADC1

  adc_disable_scan_mode(ADC1); // Deshabilita el modo de escaneo del ADC1
  adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC); // Configura el tiempo de muestreo en todos los canales del ADC1

  adc_power_on(ADC1); // Enciende el ADC1

}

// Función para leer un canal del ADC (Convertidor Analógico-Digital) extraido de adc-dac-printf.c
static uint16_t read_adc_naiive(uint8_t channel) {
  uint8_t channel_array[16];
  channel_array[0] = channel;
  adc_set_regular_sequence(ADC1, 1, channel_array);
  adc_start_conversion_regular(ADC1);
  while (!adc_eoc(ADC1));
  uint16_t reg16 = adc_read_regular(ADC1);
  return reg16;
}

// Función para leer un eje del giroscopio, combinando el byte menos significativo (out_l_reg) y el más significativo (out_m_reg)
int16_t axis_read(uint8_t out_l_reg, uint8_t out_m_reg) {
  return reg_read(out_l_reg) | (reg_read(out_m_reg) << 8);
}

// Función para leer los valores de los ejes X, Y, Z del giroscopio
axis xyz_read(void) {
  axis measurement;
  // Lee y escala los valores de los ejes
  measurement.x = axis_read(OUT_X_L | READ, OUT_X_H | READ) * SENSITIVITY;
  measurement.y = axis_read(OUT_Y_L | READ, OUT_Y_H | READ) * SENSITIVITY;
  measurement.z = axis_read(OUT_Z_L | READ, OUT_Z_H | READ) * SENSITIVITY;

  return measurement; // Devuelve la measurement  de los 3 ejes
}

//FUNCIONES DEL LED DE ADVERTENCIA DE BATERIA BAJA
static void led_red_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOG); // Habilita el reloj del puerto G
  gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO14); // Configura el pin 5 del puerto G como salida
}

static void led_red_toggle(float battery_lvl) {
  if (battery_lvl < 7) {
    gpio_toggle(GPIOG, GPIO14); // Cambia el estado del pin 5 del puerto G
  } else {
    gpio_clear(GPIOG, GPIO14); // Pone en bajo el pin 5 del puerto G
  }
}

//FUNCIONES DE LED DE COMUNICACION
static void led_green_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOG); // Habilita el reloj del puerto G
  gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13); // Configura el pin 6 del puerto G como salida
}

static void led_greend_blink(bool send) {
  if (send) {
    gpio_toggle(GPIOG, GPIO13); // Cambia el estado del pin 6 del puerto G
  } else {
    gpio_clear(GPIOG, GPIO13); // Pone en bajo el pin 6 del puerto G
  }
}

// Función para configurar el botón, extraido de button.c
static void botton_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOA); // Habilita el reloj del puerto A
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0); // Configura el pin 0 del puerto A como entrada
}

// Funcion para configurar el USART. Extraido del ejemplo usart.c
static void usart_setup(void) {

  /* Enable GPIOG clock for LED & USARTs. */
  rcc_periph_clock_enable(RCC_GPIOG);
  rcc_periph_clock_enable(RCC_GPIOA);

  /* Enable clocks for USART2. */
  rcc_periph_clock_enable(RCC_USART1);

  /* Setup GPIO pins for USART1 transmit. */
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);

  /* Setup USART1 TX pin as alternate function. */
  gpio_set_af(GPIOA, GPIO_AF7, GPIO9);

  /* Setup USART2 parameters. */
  usart_set_baudrate(USART1, 115200);
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  usart_set_mode(USART1, USART_MODE_TX);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

  /* Finally enable the USART. */
  usart_enable(USART1);
}

// Función para mostrar los datos en la pantalla LCD
void display_data(axis measurement, float battery_lvl, bool send) {
  char buffer[20];
  int enter = 30; // Espacio entre lineas
  // Limpiar la pantalla y configurar el texto
  gfx_fillScreen(LCD_BLACK); // Limpiar pantalla
  gfx_setTextSize(2); // Tamano de texto
  gfx_setTextColor(LCD_WHITE, LCD_BLACK); // Color del texto en blanco, fondo negro

  // Mostrar nivel de batería
  sprintf(buffer, "Bateria:%.2f V", battery_lvl); // Convierte de float a char
  gfx_setCursor(0, 1 * enter);
  gfx_puts(buffer);

  // Leyenda
  gfx_setCursor(0, 3 * enter);
  gfx_puts("Ejes");

  // Mostrar eje X
  sprintf(buffer, " X:%d", measurement.x); // Convertir a entero
  gfx_setCursor(0, 4 * enter);
  gfx_puts(buffer);

  // Mostrar eje Y
  sprintf(buffer, " Y:%d", measurement.y); // Convertir a entero
  gfx_setCursor(0, 5 * enter);
  gfx_puts(buffer);

  // Mostrar eje Z
  sprintf(buffer, " Z:%d", measurement.z); // Convertir a entero
  gfx_setCursor(0, 6 * enter);
  gfx_puts(buffer);

  // Mostrar estado de comunicacion
  gfx_setCursor(0, 8 * enter);
  if (send) {
    gfx_puts("Comunicacion:");
    gfx_setCursor(0, 9 * enter);
    gfx_puts(" ON");
  } else {
    gfx_puts("Comunicacion:");
    gfx_setCursor(0, 9 * enter);
    gfx_puts(" OFF");
  }

  lcd_show_frame(); // Mostrar en pantalla
}

// Función para inicializar todo el sistema
void setup(void) {
  clock_setup();
  console_setup(115200);
  sdram_init();

  gyro_setup();
  adc_setup();
  led_red_setup();
  led_green_setup();
  botton_setup();
  usart_setup();
  lcd_spi_init();
  gfx_init(lcd_draw_pixel, 240, 320);

}

/*////////////////////////////////////////////////////////////////////////////////////////
    P   R   O   G   R   A   M   A   -  P   R   I   N   C   I   P   A   L
*/ //////////////////////////////////////////////////////////////////////////////////////
int main(void) {
  axis measurement;
  uint16_t input_adc5;
  uint16_t battery_lvl;
  bool send = false;

  setup();

  // Bucle principal del programa
  while (1) {
    measurement = xyz_read(); // Lee datos del giroscopio

    input_adc5 = read_adc_naiive(5); // Lee el canal 5 del ADC
    battery_lvl = input_adc5;

    //Si se presiona el boton, se alterna el estado de 'enviar'
    if (gpio_get(GPIOA, GPIO0)) {
      send = !send;
    }

    display_data(measurement, battery_lvl, send); // Muestra la data en la pantalla LCD.

    led_red_toggle(battery_lvl); // Enciende el LED de advertencia de batería baja
    led_greend_blink(send); // Enciende el LED verde de comunicación

    msleep(100); // Espera 100 ms
  }
  return 0; // Fin del programa
}