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
// Librerias de la biblioteca libopencm3
#include "clock.h" // Para configuracion de reloj
#include "console.h" // Para funciones de consola
#include "sdram.h" // Para funciones de SDRAM
#include "lcd-spi.h" // Para funciones de LCD
#include "gfx.h"  // Para funciones de graficos



/*////////////////////////////////////////////////////////////////////////////////////////
    P   R   O   T   O   T   I   P   O   S
*////////////////////////////////////////////////////////////////////////////////////////
void mostrar_datos(float nivel_bateria, float eje_x, float eje_y, float eje_z);
void setup(void);


/*////////////////////////////////////////////////////////////////////////////////////////
    P   R   O   G   R   A   M   A   -  P   R   I   N   C   I   P   A   L
*///////////////////////////////////////////////////////////////////////////////////////
int main(void)
{
    // Configuracion inicial
    setup();

    // Variables
    float nivel_bateria = 0.0; // Nivel de bateria
    float eje_x = 0.0; // Eje X
    float eje_y = 0.0; // Eje Y
    float eje_z = 0.0; // Eje Z

    // Mostrar datos en pantalla
    mostrar_datos(nivel_bateria, eje_x, eje_y, eje_z);

    // Bucle infinito
    while (1) {
        // Leer sensores
        nivel_bateria = 5.0;
        eje_x = 4.32;
        eje_y = 180.01;
        eje_z = 21.32;

        // Mostrar datos en pantalla
        mostrar_datos(nivel_bateria, eje_x, eje_y, eje_z);
    }
}


/*///////////////////////////////////////////////////////////////////////////////////////
    D   E   F   I   N   I   C   I   O   N   E   S   -   F   U   N   C   I   O   N   E   S
*///////////////////////////////////////////////////////////////////////////////////////
void setup() {
	clock_setup(); // Configurar reloj
	console_setup(115200); // Configurar consola
	sdram_init(); // Inicializar SDRAM
	lcd_spi_init(); // Inicializar LCD
    gfx_init(lcd_draw_pixel, 240, 320); // Inicializar graficos
}


// Función para mostrar los datos en la pantalla LCD
void mostrar_datos(float nivel_bateria, float eje_x, float eje_y, float eje_z){

    char buffer[50]; // para almacenar las cadenas de texto retornadas por sprintf

    // Limpiar la pantalla y configurar el texto
    gfx_fillScreen(LCD_BLACK); // Limpiar pantalla
    gfx_setTextSize(2); // Tamano de texto
    gfx_setTextColor(LCD_WHITE, LCD_BLACK); // Color del texto en blanco, fondo negro

    // Mostrar nivel de batería
    sprintf(buffer, "Bateria:%.2f V", nivel_bateria); // Convierte de float a char
    gfx_setCursor(0, 10);
    gfx_puts(buffer);

    // Leyenda
    gfx_setCursor(0, 40);
    gfx_puts("Ejes");

    // Mostrar eje X
    sprintf(buffer, " X:%.2f", eje_x); // Convertir a entero
    gfx_setCursor(0, 70);
    gfx_puts(buffer);

    // Mostrar eje Y
    sprintf(buffer, " Y:%.2f", eje_y); // Convertir a entero
    gfx_setCursor(0, 100);
    gfx_puts(buffer);

    // Mostrar eje Z
    sprintf(buffer, " Z:%.2f", eje_z); // Convertir a entero
    gfx_setCursor(0, 130);
    gfx_puts(buffer);

    lcd_show_frame();  // Mostrar en pantalla
}