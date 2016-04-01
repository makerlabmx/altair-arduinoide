Contents/Contenido:
- [English](#user-content-english)
- [Español](#user-content-spanish)

* * *

### English:
# Altair Hardware Definition and Libraries for the Arduino IDE

**Note: ** This version is only compatible with Hub (aquila-server) version 0.2.0 or newer.

## Installation:

- Install Arduino 1.6.8 from the [Arduino website](http://www.arduino.cc/en/main/software).
- Start Arduino and open Preferences window.
- Enter ``http://makerlabmx.github.io/altair-arduinoide/package_makerlabmx_index.json`` into Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
- Open Boards Manager from Tools > Board menu and install Aquila Boards platform (and don't forget to select Altair board from Tools > Board menu after installation).
- Install the USB drivers:
  - Windows and OSX: 
    - For FTDI-based USB-Serial (purple board): [FTDI Drivers](http://www.ftdichip.com/Drivers/VCP.htm)
    - For CP120X-based USB-Serial (black board): [CP210x Drivers](https://www.silabs.com/products/mcu/Pages/USBtoUARTBridgeVCPDrivers.aspx)
  - Linux: Should be already included in your Distros Kernel, if not, check the links above.

## Use:

- Open the Arduino IDE
- Connect your Altair via the USB-Serial to your PC
- In the menu bar, select Tools > Board > altair
- Select the correct serial port in Tools > Port
- For an example on using the Aquila libraries, select File > Examples > AquilaProtocol > template
- Upload the example with the arrow icon in the IDE

The best place to ask questions related to this core is the Aquila community forum: http://community.aquila.io/.

* * *

### Spanish:
# Bibliotecas y definición de hardware de Altair para el IDE de Arduino

**Nota: ** Esta versión es únicamente compatible con versiones 0.2.0 o superior del hub (aquila-server).

## Instalación:

- Instala Arduino 1.6.8 desde el [sitio de Arduino](http://www.arduino.cc/en/main/software).
- Inicia Arduino y abre la ventana de preferencias.
- Introduce ``http://makerlabmx.github.io/altair-arduinoide/package_makerlabmx_index.json`` en el campo "Gestor de URLs adicionales de tarjetas". Puedes agregar múltiples URLs separándolas con comas.
- Abre el Gestor de tarjetas desde el menú Herramientas > Placa e instala la plataforma "Aquila Boards" (no olvides seleccionar Altair desde el menú Herramientas > Placa después de instalarlo).
- Instala los drivers USB:
  - Windows y OSX:
    - Para USB-Serial basados en FTDI (placa morada): [FTDI Drivers](http://www.ftdichip.com/Drivers/VCP.htm)
    - Para USB-Serial basados en CP210X (placa negra): [CP210x Drivers](https://www.silabs.com/products/mcu/Pages/USBtoUARTBridgeVCPDrivers.aspx)
  - Linux: Ya deben de estar incluidos en el kernel de tu distribución, si no es así, checa el link anterior.


## Uso:

- Abre el IDE de Arduino
- Conecta el Altair por medio del adaptador USB-Serial a tu PC
- En la barra de menú, selecciona Herramientas > Placa > altair
- Selecciona el puerto serial correspondiente en Herramientas > Port
- Para ver un ejemplo del uso de las bibliotecas Aquila, selecciona Archivo > Ejemplos > AquilaProtocol > template
- Sube el ejemplo con el ícono de flecha en el IDE

El mejor lugar para hacer preguntas relacionadas con esta definición de hardware es el foro de la comunidad Aquila: http://community.aquila.io/.

