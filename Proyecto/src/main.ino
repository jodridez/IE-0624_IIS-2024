/*  
Universidad de Costa Rica
Escuela de Ingenieria Electrica

IE-0624 Laboratorio de Microcontroladores

main.ino

Autor: Jonathan Rodriguez Hernandez <jonathan.rodriguezhernandez@ucr.ac.cr>.
Carnet: B76490.
Fecha: -/-/2024.

Descripcion:
Se trata de un contador de visitantes que hace uso del Arduino Nano BLE 33 y dos sensores
de proximidad para detectar y contar el numero de personas que ingresan a una habitacion o area
especifica. El sistema debe enviar los datos a una interfaz web utilizando conectividad proporcionada
por un modulo WiFi y la plataforma IoT ThingsBoard para recolectar, analizar y visualizar los datos
de visitantes.

Caracteristicas:
Dos sensores de proximidad vinculados a dos puertas, una de entrada y otra de salida,
permiten determinar la direccion del movimiento, en este caso, si la persona sale o entra. Se
guarda el instante en que la persona entro o salio.
Los datos son enviados a la plataforma IoT elegida, para su monitoreo, almacenamiento y
analisis:
-Numero total de Personas: La cantidad total de personas presentes en el lugar en un
momento dado.
-Flujo de Personas: La cantidad de personas que entran y salen en diferentes intervalos de
tiempo (por hora, por día, etc.).
-Pico de Asistencia: Identificar los momentos del día o días de la semana con mayor numero
de personas.
-Tendencias de Asistencia: Analizar como varía la asistencia a lo largo del tiempo para
predecir futuros patrones de flujo.
-Duración de la Estancia: Medir cuanto tiempo pasan las personas en el lugar, lo que puede
ayudar a entender el comportamiento del cliente.
-Seguridad y Emergencias: Monitorear la cantidad de personas para garantizar que no se
superen los limites de capacidad y mejorar los planes de evacuación.
*/

// Includes

/*
 Definicion de pines
*/
// Pines de sensores de proximidad HC-SR04 (Trigger y Echo)
#define TRIG_INT 12
#define ECHO_INT 11
#define TRIG_OUT 10
#define ECHO_OUT 9

/*
 Variables globales 
*/
// Constante para la conversion de tiempo a distancia
const float CONVERSION_FACTOR = 58.0;  // Factor para convertir el tiempo de pulso a distancia en cm
const int DISTANCIA_MAXIMA = 400;  // Distancia maxima que se considerara valida (400 cm)
const int DISTANCIA_MINIMA = 2;  // Distancia mínima que se considerara valida (2 cm)

const int ancho_puerta = 100; // Ancho de la puerta en cm

// Variables para almacenar la distancia de los sensores
int distanciaEntrada = 0;
int distanciaSalida = 0;

// Variables para almacenar el monitoreo de las personas
int personasDentro = 0;

// Funcion para medir la distancia de un sensor de ultrasonido
int medirDistancia(int trig, int echo) {
    digitalWrite(trig, LOW);  // Asegurarse de que el pin de trigger este en bajo
    delayMicroseconds(2);  // Esperar 2 microsegundos
    digitalWrite(trig, HIGH);  // Enviar un pulso de 10 microsegundos
    delayMicroseconds(10);  // Esperar 10 microsegundos
    digitalWrite(trig, LOW);  // Poner el pin de trigger en bajo nuevamente
    int duracion = pulseIn(echo, HIGH, 30000);  // Esperar la respuesta del eco con un timeout de 30 ms
    if (duracion == 0) {
        return -1;  // Si no se detecta eco, retornar -1 (valor invalido)
    }
    int distancia = duracion / CONVERSION_FACTOR;
    return  distancia;
}

// Funcion para pruebas
void serializarDatos(int distanciaEntrada, int distanciaSalida) {
    if (distanciaEntrada >= DISTANCIA_MINIMA && distanciaEntrada <= DISTANCIA_MAXIMA) {
        Serial.print("Distancia Entrada: ");
        Serial.print(distanciaEntrada);
        Serial.print(" cm - ");
    } else {
        Serial.print("Distancia Entrada: No valida - ");
    }
    if (distanciaSalida >= DISTANCIA_MINIMA && distanciaSalida <= DISTANCIA_MAXIMA) {
        Serial.print("Distancia Salida: ");
        Serial.print(distanciaSalida);
        Serial.println(" cm");
    } else {
        Serial.println("Distancia Salida: No valida");
    }
}

//funcion para contar personas
void contarPersonas(int distanciaEntrada, int distanciaSalida) {
    if (distanciaEntrada >= DISTANCIA_MINIMA && distanciaEntrada <= DISTANCIA_MAXIMA) { // Si la distancia de entrada es valida
        if (distanciaEntrada <= ancho_puerta) { // Si la distancia de entrada es menor o igual al ancho de la puerta
            personasDentro++; // Aumentar el contador de personas dentro
        }
    }
    if (distanciaSalida >= DISTANCIA_MINIMA && distanciaSalida <= DISTANCIA_MAXIMA) { // Si la distancia de salida es valida
        if (distanciaSalida <= ancho_puerta) { // Si la distancia de salida es menor o igual al ancho de la puerta
            personasDentro--; // Disminuir el contador de personas dentro
            if(personasDentro <= 0){ // Si el contador de personas dentro es menor o igual a 0
              personasDentro = 0; // Establecer el contador de personas dentro a 0
            }
        }
    }
}


// Configuración inicial del sistema
void setup() {
    // Inicializar el puerto serial
    Serial.begin(9600);
    // Inicializar los pines de los sensores
    pinMode(TRIG_INT, OUTPUT);
    pinMode(ECHO_INT, INPUT);
    pinMode(TRIG_OUT, OUTPUT);
    pinMode(ECHO_OUT, INPUT);
}

// Bucle principal
void loop() {
    // Medir la distancia de los sensores
    distanciaEntrada = medirDistancia(TRIG_INT, ECHO_INT);
    distanciaSalida = medirDistancia(TRIG_OUT, ECHO_OUT);
    // Serializar los datos
    //serializarDatos(distanciaEntrada, distanciaSalida);
    contarPersonas(distanciaEntrada, distanciaSalida);
    // Mostrar el numero de personas dentro
    Serial.print("Personas dentro: ");
    Serial.println(personasDentro);
    // Esperar 1 segundo
    delay(1000);
}
