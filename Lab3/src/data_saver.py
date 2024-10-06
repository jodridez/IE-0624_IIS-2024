import serial
import pandas as pd
import time

# Configura el puerto serial
ser = serial.Serial('COM2', 9600) # Arduino COM1, python COM2
time.sleep(2)  # Espera a que se establezca la conexión

data = []
run_program = True  # Flag para controlar el bucle

try:
    print("Leyendo datos... Presiona 'Ctrl + C' para detener.")

    while run_program:
        if ser.in_waiting:  # Verifica si hay datos disponibles en el puerto serial
            line = ser.readline().decode('utf-8').strip()  # Lee línea
            print(f"Datos recibidos: {line}")  # Feedback al usuario
            data.append(line.split())  # Separa por espacios

except KeyboardInterrupt:
    print("\nInterrupción detectada. Guardando datos...")

finally:
    # Guarda los datos en un archivo CSV al finalizar
    if data:
        # DataFrame para manejar 5 columnas (Tensiones + Unidad)
        df = pd.DataFrame(data, columns=['TENSION 1', 'TENSION 2', 'TENSION 3', 'TENSION 4', 'UNIDAD'])
        
        df.to_csv('data_saved.csv', index=False) # Guarda los datos en un archivo CSV
        print("Datos guardados en 'data_saved'.") # Feedback al usuario
    else:
        print("No se recibieron datos.") # Feedback al usuario
    
    ser.close()  # Cierra el puerto serial
    print("Puerto serial cerrado.") # Feedback al usuario
