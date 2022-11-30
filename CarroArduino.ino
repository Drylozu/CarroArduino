#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// Pines
#define IR_IZQ 5  // D1
#define IR_DER 4  // D2
#define IR_BCK 0  // D3
#define MT_1 16   // D0
#define MT_2 14   // D5
#define MT_3 12   // D6
#define MT_4 13   // D7

#define US_ECHO 1     // TX
#define US_TRIGGER 3  // RX

// Red WiFi
#define SSID "AUTO"
#define PSK "auto1234"
#define DNS_PORT 53

// Páginas
#define CONTROL_HTML "<!DOCTYPE html><html><head> <meta charset=\"UTF-8\"> <title>AUTO</title> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no\"/> <meta name=\"HandheldFriendly\" content=\"true\"/> <style>*{-webkit-user-select: none; -moz-user-select: none; user-select: none;}body{margin: 0; padding: 0; background-color: #101010; color: white; font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, Helvetica, Arial, sans-serif, \"Apple Color Emoji\", \"Segoe UI Emoji\", \"Segoe UI Symbol\";}#main{width: 100%; height: 100vh; display: flex; justify-content: center; align-items: center;}.controls{height: 100%; display: flex; flex-direction: column; align-items: center; justify-content: space-evenly;}.movement{display: flex; flex-wrap: wrap; flex-direction: column; gap: 1rem;}.movement>*{flex: 1; display: flex; justify-content: center;}.triangle{clip-path: polygon(50% 0%, 0% 100%, 100% 100%); background-color: white; width: 10vh; height: 10vh;}.circle{background-color: crimson; width: 10vh; height: 10vh; border-radius: 100%;}.double{display: flex; gap: 1rem;}#left{transform: rotate(-90deg);}#right{transform: rotate(90deg);}#backward{transform: rotate(180deg);}.sensor{width: 5vh; height: 5vh; border: 2px solid palegreen; border-radius: 0.25rem;}.sensor[data-checked=\"0\"]{background-color: palegreen;}.sensors{font-size: 5vh; display: flex; flex-direction: column; align-items: center; gap: 0.5rem;}.col{display: flex; align-items: center; flex-direction: column;}.text-sensor{font-weight: bold; color: black; -webkit-text-stroke: 2px palegreen;}.velocity{display: flex; flex-direction: column; align-items: center; font-size: 1.5rem;}</style></head><body> <div id=\"main\"> <div class=\"controls\"> <div class=\"sensors\"> <div class=\"col\"> <span>F(cm)</span> <span class=\"text-sensor\" id=\"front-sen\">-</span> </div><div class=\"double\"> <span>L</span> <div class=\"sensor\" data-checked=\"0\" id=\"left-sen\"></div><div style=\"width: 2rem;\"></div><div class=\"sensor\" data-checked=\"0\" id=\"right-sen\"></div><span>R</span> </div><div class=\"col\"> <div class=\"sensor\" data-checked=\"0\" id=\"back-sen\"></div><span>B</span> </div></div><div class=\"velocity\"> <span>V</span> <input type=\"range\" name=\"slider\" id=\"velocity\" min=\"50\" max=\"255\" step=\"1\" value=\"50\"> <span id=\"vel\">50</span> </div><div class=\"movement\"> <div> <button class=\"triangle\" auto-movement id=\"forward\"></button> </div><div class=\"double\"> <button class=\"triangle\" auto-movement=\"temp\" id=\"left\"></button> <button class=\"circle\" auto-movement id=\"stop\"></button> <button class=\"triangle\" auto-movement=\"temp\" id=\"right\"></button> </div><div> <button class=\"triangle\" auto-movement id=\"backward\"></button> </div></div></div></div><script>const leftSensor=document.getElementById('left-sen'); const rightSensor=document.getElementById('right-sen'); const backSensor=document.getElementById('back-sen'); const frontSensor=document.getElementById('front-sen'); const velocitySlider=document.getElementById('velocity'); const velocity=document.getElementById('vel'); velocitySlider.addEventListener('input', ()=>{velocity.innerText=velocitySlider.value;}); velocitySlider.addEventListener('change', ()=>{fetch('/v?n=' + velocitySlider.value).catch(console.error);}); let lastState='s'; let lastMove='s'; const movementControls=[...document.querySelectorAll('[auto-movement]').values()]; for (const control of movementControls) control.addEventListener('click', ()=>{let state=control.getAttribute('auto-movement')==='temp' && lastState===control.id[0] ? lastMove : control.id[0]; fetch('/' + state).catch(console.error) .then(()=>{if (control.getAttribute('auto-movement') !=='temp'){lastMove=state;}lastState=state;});}); async function fetchVelocity(){try{const data=Number(await fetch('/v' + velocitySlider.value).then((d)=> d.text())); if (!isNaN(data)) velocity.innerText=data;}catch{fetchVelocity();}}fetchVelocity(); async function fetchSensors(){const data=await fetch('/sen').then((d)=> d.json()); leftSensor.setAttribute('data-checked', data.irIzq); rightSensor.setAttribute('data-checked', data.irDer); backSensor.setAttribute('data-checked', data.irBck); frontSensor.innerText=data.usFrt;}setInterval(fetchSensors, 700); </script></body></html>"

// IP: 192.168.1.1
IPAddress apIP(192, 168, 1, 1);
DNSServer dns;
ESP8266WebServer server(80);

uint8 irIzq = LOW;
uint8 irDer = LOW;
uint8 irBck = LOW;
unsigned int usFrt;
unsigned long duration;

unsigned int velocity = 50;

void setup() {
  pinMode(IR_IZQ, INPUT);
  pinMode(IR_DER, INPUT);
  pinMode(IR_BCK, INPUT);

  pinMode(MT_1, OUTPUT);
  pinMode(MT_2, OUTPUT);
  pinMode(MT_3, OUTPUT);
  pinMode(MT_4, OUTPUT);

  pinMode(US_TRIGGER, OUTPUT);
  pinMode(US_ECHO, INPUT);

  setupWiFi();
}

void loop() {
  readSensors();
  dns.processNextRequest();
  server.handleClient();
}

void readSensors() {
  // Infrarrojos
  irIzq = digitalRead(IR_IZQ);
  irDer = digitalRead(IR_DER);
  irBck = digitalRead(IR_BCK);

  // Ultrasonido
  digitalWrite(US_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIGGER, LOW);
  duration = pulseIn(US_ECHO, HIGH);
  usFrt = duration * 0.017;
}

auto move(char dir) {
  return [dir]() {
    switch (dir) {
      case 'f':
        analogWrite(MT_1, 0);
        analogWrite(MT_2, velocity);
        analogWrite(MT_3, 0);
        analogWrite(MT_4, velocity);
        break;
      case 'l':
        analogWrite(MT_1, 0);
        analogWrite(MT_2, velocity);
        analogWrite(MT_3, velocity);
        analogWrite(MT_4, 0);
        break;
      case 's':
        analogWrite(MT_1, 0);
        analogWrite(MT_2, 0);
        analogWrite(MT_3, 0);
        analogWrite(MT_4, 0);
        break;
      case 'r':
        analogWrite(MT_1, velocity);
        analogWrite(MT_2, 0);
        analogWrite(MT_3, 0);
        analogWrite(MT_4, velocity);
        break;
      case 'b':
        analogWrite(MT_1, velocity);
        analogWrite(MT_2, 0);
        analogWrite(MT_3, velocity);
        analogWrite(MT_4, 0);
        break;
    }

    server.send(204);
  };
}

void setupWiFi() {
  dns.setErrorReplyCode(DNSReplyCode::NoError);
  dns.start(DNS_PORT, "*", apIP);

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID, PSK);

  // Página de controles
  server.on("/", []() {
    server.send(200, "text/html", CONTROL_HTML);
  });

  // Estado de sensores
  server.on("/sen", []() {
    server.send(200, "application/json", "{\"irIzq\":" + String(irIzq) + ",\"irDer\":" + String(irDer) + ",\"irBck\":" + String(irBck) + ",\"usFrt\":" + String(usFrt) + "}");
  });

  // Velocidad
  server.on("/v", []() {
    if (server.hasArg("n")) {
      velocity = server.arg("n").toInt();
      server.send(204);
      return;
    }

    server.send(200, "text/html", String(velocity));
  });

  server.on("/f", move('f'));  // Adelante
  server.on("/l", move('l'));  // Izquierda
  server.on("/s", move('s'));  // Detenerse
  server.on("/r", move('r'));  // Derecha
  server.on("/b", move('b'));  // Atrás

  // Redireccionamiento (DNS o 404)
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.1.1/");
    server.send(301);
  });

  server.begin();
}