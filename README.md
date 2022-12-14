# Carro Arduino
Este es un proyecto de un carro hecho con Arduino controlado por WiFi. [Redirecciona los DNS](https://en.wikipedia.org/wiki/Captive_portal#Redirect_by_DNS) para dirigir todo el tráfico a la misma página, los controles del carro, creando un [portal cautivo](https://en.wikipedia.org/wiki/Captive_portal).

Los controles son código HTML con CSS y JS dentro de él. Están en el archivo `controles.html` y cualquier cambio a estos deben ser minificados a lo más mínimo posible, reemplazando la constante global `CONTROL_HTML` (línea 23 del archivo `CarroArduino.ino`) con su versión minificada.

<img src="./control.png" height="300">

## Componentes
- 1x **WEMOS D1 mini** (tarjeta principal)
- 1x **Módulo controlador de motores L298N**
- 4x **Motores DC**
- 3x **Sensor de obstáculos infrarrojo FC-51**
- 1x **Sensor de ultrasonido HC-SR04**

## Configuración
![Configuración de pines](./esquemaDark.png#gh-dark-mode-only)
![Configuración de pines](./esquemaLight.png#gh-light-mode-only)

~~No recuerdo cuál fue una de las razones por las que usé los pines RX y TX pero así quedó.~~

Los pines tales como RX y TX deben estar desconectados al comunicar el Arduino con tu PC (al subir el código a la placa).