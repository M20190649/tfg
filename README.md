# Trabajo de Fin de Grado: Sistema de bajo coste para detección de dispositivos WiFi y Bluetooth

### Autor: Adrián Álvarez Sáez
### Tutor: Juan Julián Merelo Guervós

El principal objetivo de este proyecto es desarrollar un sensor de detección de presencia utilizando SoCs de bajo coste y consumo que permitan monitorizar en tiempo real el movimiento de transeúntes y vehículos mediante el análisis de tráfico WiFi y BLE generado por dispositivos móviles personales como smartphones o dispositivos *wearables*, este sensor además enviará estas detecciones a un servidor central.

Para facilitar la visualización de estas detecciones se plantea un objetivo secundario consistente en el desarrollo de un servidor central que permita recopilar las detecciones de los sensores, este servidor constará de una API REST para facilitar el envío de las detecciones además de una interfaz web para poderlas visualizar cómodamente. El código de este servidor se puede consultar en el siguiente repositorio: https://github.com/adalsa91/vedetra-server.

---
##Documentación

La documentación de este proyecto está realizada con `LaTeX`, por lo tanto para generar el archivo PDF necesitaremos instalar `TeXLive` y `biber` en nuestra distribución.

Una vez instalada, tan solo deberemos situarnos en el directorio `doc` y ejecutar los siguientes comandos:

`
$ pdflatex proyecto.tex
$ biber ./proyecto.bcf
$ pdflatex proyecto.tex
`

# INSTRUCCIONES

Lee [INSTALL.md](INSTALL.md) para las instrucciones de uso.
