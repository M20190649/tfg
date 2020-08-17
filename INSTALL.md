# Instalación

Para compilar el proyecto y subir el binario a la placa es necesario instalar la herramienta [PlatformIO Core](https://docs.platformio.org/en/latest/core/), esta herramienta se encargará de decargar automáticamente la toolchain y liberías necesarias para compilar el proyecto.

Podemos realizar la instalación con un simple comando sin necesidad de privilegios de administrador, este comando descargará y ejecutará un script de Python que realizará la instalación:

```bash
$ python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py)"
```

Para otros métodos de instalación se puede consultar la [documentación oficial de PlatformIO](https://docs.platformio.org/en/latest/core/installation.html#local-download-mac-linux-windows).

Esta instalación nos creará un entorno aislado en la ruta `$HOME/.platformio/penv/` con los binarios necesarios, podemos invocarlos especificando la ruta completa (`$HOME/.platformio/penv/bin/platformio`) o añadir los binarios al path siguiendo las [intrucciones de la documentación](https://docs.platformio.org/en/latest/core/installation.html#unix-and-unix-like)

Este proyecto ha sido desarrollado con las placas de desarrollo de la familia ESP32 DevKitC pero es compatible con cualquier otra placa basada en los SoCs ESP32, en caso de utilizar una placa diferente es necesario modificar el parámetro `board` del fichero `platformio.ini` con el identificador de la placa correspondiente, podemos encontrar el código de la placa buscando en el [explorador de PlatformIO](https://platformio.org/boards)

Antes de compilar el proyecto es necesario establecer el valor de algunas variables en el fichero `src/main.cpp`
 
```C
#define DEBUG 0           // Al establecer su valor a 1 se imprimirá por serial información de depuración como las detecciones realizadas y enviadas.

#define WIFI_SSID ""      // SSID de la re wifi que se utilizará para enviar las detecciones
#define WIFI_PASSWD ""    // Contraseña de la red WiFi especificada
#define SERVER_URL ""     // Dirección del servidor al que se enviarán las detecciones
```

Una vez instalado, especificada la placa de desarrollo a usar y configuradas las variables podemos lanzar la compilación y la subida del firmware a la placa con el siguiente comando:

```bash
$ platformio run --target upload
```

Cuando la subida finalice la placa se reiniciará y arrancará el nuevo firmware, para monitorizar la salida del serial podemos ejecutar el siguiente comando:
```bash
$ platformio device monitor
```

