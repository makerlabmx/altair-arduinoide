Contents/Contenido:
- [English](#user-content-english)
- [Español](#user-content-spanish)

* * *

### English:
# Altair Hardware Definition and Libraries for the Arduino IDE

## Installation:

Install Arduino IDE version 1.5.7 BETA or newer for your OS:
- Windows: [Installer](http://downloads.arduino.cc/arduino-1.5.7-windows.exe)
- Mac OSX: [OSX 10.7 or greater](http://downloads.arduino.cc/arduino-1.5.7-macosx-java7-r2.zip), [older OSX](http://downloads.arduino.cc/arduino-1.5.7-macosx.zip)
- Linux: [32 bit](http://downloads.arduino.cc/arduino-1.5.7-linux32.tgz), [64 bit](http://downloads.arduino.cc/arduino-1.5.7-linux64.tgz)

Install the USB drivers:
- Windows and OSX: [FTDI Drivers](http://www.ftdichip.com/Drivers/VCP.htm)
- Linux: Should be already included in your Distros Kernel, if not, check the link above.

Clone this repo inside the "hardware" folder inside your sketch folder as "altair".
Alternatively, You can download the zip, uncompress it and manually copy it.
(Create the folder if it doesn't exist)

Example:
```
cd <path to your home folder>/Arduino/hardware/
git clone https://github.com/makerlabmx/altair.git altair
```

## Use:

- Open the Arduino IDE
- Connect your Altair via the USB-Serial to your PC
- In the menu bar, select Tools > Board > altair
- Select the correct serial port in Tools > Port
- For an example on using the Aquila libraries, select File > Examples > Aquila > template
- Upload the example with the arrow icon in the IDE

* * *

### Spanish:
# Bibliotecas y definición de hardware de Altair para el IDE de Arduino

## Instalación:

Instala el IDE de Arduino versión 1.5.7 BETA o superior para tu sistema operativo:
- Windows: [Instalador](http://downloads.arduino.cc/arduino-1.5.7-windows.exe)
- Mac OSX: [OSX 10.7 o superior](http://downloads.arduino.cc/arduino-1.5.7-macosx-java7-r2.zip), [OSX más antiguo](http://downloads.arduino.cc/arduino-1.5.7-macosx.zip)
- Linux: [32 bit](http://downloads.arduino.cc/arduino-1.5.7-linux32.tgz), [64 bit](http://downloads.arduino.cc/arduino-1.5.7-linux64.tgz)

Instala los drivers USB:
- Windows y OSX: [FTDI Drivers](http://www.ftdichip.com/Drivers/VCP.htm)
- Linux: Ya deben de estar incluidos en el kernel de tu distribución, si no es así, checa el link anterior.

Clona este repositorio dentro de la carpeta "hardware" en la carpeta de sketchs de arduino, respetando el nombre "altair".
Alternativamente puedes descargar el zip, descomprimirlo y copiarlo manualmente.
(Si no existe la carpeta, créala)

Ejemplo:
```
cd <dirección de tu carpeta personal>/Arduino/hardware/
git clone https://github.com/makerlabmx/altair.git altair
```

## Uso:

- Abre el IDE de Arduino
- Conecta el Altair por medio del adaptador USB-Serial a tu PC
- En la barra de menú, selecciona Herramientas > Placa > altair
- Selecciona el puerto serial correspondiente en Herramientas > Port
- Para ver un ejemplo del uso de las bibliotecas Aquila, selecciona Archivo > Ejemplos > Aquila > template
- Sube el ejemplo con el ícono de flecha en el IDE
