import serial
import time
import paho.mqtt.client as mqtt

# Configuración del puerto serie
puerto = '/dev/ttyACM0'
baudrate = 115200

# Configuración de MQTT
broker = "iot.eie.ucr.ac.cr"
topic_pub = 'v1/devices/me/telemetry'

# Inicializa el cliente MQTT
client = mqtt.Client()
client.username_pw_set("t59vaa9irj8q9cixb7k3")  # Configura las credenciales
client.connect(broker, 1883, 1)  # Conecta al broker

# Abre el puerto serie
with serial.Serial(puerto, baudrate, timeout=1) as ser:
    while True:
        linea = ser.readline().decode('utf-8').rstrip()  # Lee y decodifica la línea
        if linea:  # Si hay datos en la línea
            # Separa los datos en una lista
            datos = linea.split(',')
            if len(datos) == 5:  # Verifica que haya 5 elementos
                # Convierte los datos a los tipos apropiados
                x = float(datos[0])
                y = float(datos[1])
                z = float(datos[2])
                nivel_de_bateria = float(datos[3])
                estado = int(datos[4])
                if estado == 1:
                    alerta = 'Sí'
                if estado == 0:
                    alerta = 'No'
                
                # Crea un mensaje en formato JSON
                msg = f'{{"x":{x},"y":{y},"z":{z},"Bateria":{nivel_de_bateria},"Bateria baja":{alerta}}}'
                
                # Publica el mensaje en el tópico
                client.publish(topic_pub, msg)
        
        time.sleep(0.1)  # Pausa para evitar sobrecarga del CPU
