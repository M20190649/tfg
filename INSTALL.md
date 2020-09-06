# Instalación

Para compilar el proyecto y subir el binario a la placa es necesario instalar la herramienta [PlatformIO Core](https://docs.platformio.org/en/latest/core/), esta herramienta se encargará de decargar automáticamente la *toolchain* y librerías necesarias para compilar el proyecto, además de otras herramientas para grabar el binario en la placa o monitorizar la salida del puerto *Serial*.

En sistemas Linux podemos realizar la instalación con un simple comando sin necesidad de privilegios de administrador, este comando descargará y ejecutará un script de Python que realizará la instalación:

```bash
$ python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py)"
```

Para otros métodos de instalación se puede consultar la [documentación oficial de PlatformIO](https://docs.platformio.org/en/latest/core/installation.html#local-download-mac-linux-windows).

Esta instalación nos creará un entorno aislado en la ruta `$HOME/.platformio/penv/` con los binarios necesarios, podemos invocarlos especificando la ruta completa (`$HOME/.platformio/penv/bin/platformio`) o añadir los binarios al path siguiendo las [intrucciones de la documentación](https://docs.platformio.org/en/latest/core/installation.html#unix-and-unix-like)

Este proyecto ha sido desarrollado con las placas de desarrollo de la familia ESP32 DevKitC pero es compatible con cualquier otra placa basada en los SoCs ESP32, en caso de utilizar una placa diferente es necesario modificar el parámetro `board` del fichero `platformio.ini` con el identificador de la placa correspondiente, podemos encontrar el código de la placa buscando en el [explorador de PlatformIO](https://platformio.org/boards)

Antes de compilar el proyecto es necesario establecer el valor de algunas variables en el fichero `src/main.cpp`
 
```C
#define DEBUG 0           // Al establece su valor a 1 se imprimirá por el puerto Serial información de depuración, por ejemplo, las detecciones realizadas o el JSON enviado al servidor central. Por defecto su valor es 0.

#define WIFI_SSID ""      // SSID de la red WiFi a la que el sensor se conectará para enviar las detecciones.
#define WIFI_PASSWD ""    // Contraseña de la red WiFi especificada.
#define SERVER_URL ""     // URL del endpoint al que se enviaran las detecciones (http://<host>:<puerto>/api/v1.0/detections-collection/).
```

Una vez instalado, especificada la placa de desarrollo a usar y configuradas las variables podemos lanzar la compilación y la subida del firmware a la placa, para ello debemos situarnos en la raíz del proyecto dónde se encuentra el fichero `platformio.ini` y ejecutar el siguiente comando:

```bash
$ platformio run --target upload
```

Cuando la subida finalice la placa se reiniciará y arrancará el nuevo firmware, para monitorizar la salida del serial podemos ejecutar el siguiente comando:
```bash
$ platformio device monitor
```

Usando este comando en el arranque de la placa podremos obtener el identificador único (`Station_id`) de la placa que será necesario durante la [instalación del servidor](https://github.com/adalsa91/vedetra-server/blob/master/INSTALL.md). En el siguiente código se puede ver un ejemplo de la información que muestra la placa por el puerto `Serial` al arrancar.

```
Initializing system.
Generating station ID.
Station_id: 363daf8619557863c7b4f9767389cb6c
```
