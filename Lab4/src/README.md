# Laboratorio 4

## Instrucciones de uso

1. Compila el ejecutando el siguiente comando en la terminal:
    ```bash
    make
    ```

2. Luego, convierte el archivo ELF a formato binario con el siguiente comando:
    ```bash
    arm-none-eabi-objcopy -O binary lab4_main.elf firmware.bin
    ```

3. Finalmente, carga el archivo binario en el dispositivo y reinícialo con:
    ```bash
    st-flash --reset write firmware.bin 0x8000000
    ```
