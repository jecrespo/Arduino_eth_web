#include <EtherCard.h>
#include <EEPROM.h>
#include "Timer.h"

//Un timer para cada pin/relay
Timer t;
int id1;
int id2;
int id3;
int id4;

#define STATIC 1
#if STATIC
static byte myip[] = {192, 168, 31, 10};
static byte gwip[] = { 192, 168, 31, 1 };
#endif

static byte mymac[] = {0x74, 0x69, 0x69, 0x2D, 0x30, 0x31};

byte Ethernet::buffer[800];
BufferFiller bfill;

int LedPins[] = {3, 4, 5, 6};

int leeEEprom = 0;

boolean PinStatus[] = {1, 2, 3, 4};

const char http_OK[] PROGMEM =
  "HTTP/1.0 200 OK\r\n"
  "Content-Type: text/html\r\n"
  "Pragma: no-cache\r\n\r\n";

const char http_Found[] PROGMEM =
  "HTTP/1.0 302 Found\r\n"
  "Location: /\r\n\r\n";

const char http_Unauthorized[] PROGMEM =
  "HTTP/1.0 401 Unauthorized\r\n"
  "Content-Type: text/html\r\n\r\n"
  "<h1>401 Unauthorized</h1>";

const char http_Body[] PROGMEM =
  "<!DOCTYPE html><html><head><style>"
  "body {"
  "background-color: #ffffff;"
  "}"
  "h1 {"
  "color: Orange;"
  "text-align: center;"
  "}"
  "p {"
  "font-family: 'Times New Roman';"
  "font-size: 20px;"
  "text-align: center;"
  "color: Blue;"
  "}"
  "ul.a {"
  "list-style-type: circle;"
  "}"
  "</style></head><body>"
  "<h1>Control de dispositivos Alzamora 286</h1>"
  "<p>Proyecto Control de puertas 1.0</p><hr><p>Dispositivos</p>";

void homePage()
{
  bfill.emit_p(PSTR("$F"
                    "<title>Alzamora 286</title>$F"
                    "<ul class='a'>"
                    "<li>Porton Cochera Princiapl&nbsp;&nbsp;&nbsp;&nbsp;:<a href=\"?ArduinoPIN1=$F\">$F</a></li>"
                    "<li>Porton Cochera segundaria&nbsp;:<a href=\"?ArduinoPIN2=$F\">$F</a></li>"
                    "<li>Telefono Lobby&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;:<a href=\"?ArduinoPIN3=$F\">$F</a></li>"
                    "<li>Parlante&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;:<a href=\"?ArduinoPIN4=$F\">$F</a></li>"
                    "</ul><hr>Desarrolado por Alejandro Chacón"),
               http_OK, http_Body,
               PinStatus[0] ? PSTR("off") : PSTR("on"),
               PinStatus[0] ? PSTR("<font color=\"red\"><b>APAGAR</b></font>") : PSTR("<font color=\"grey\">ENCENDER</font>"),
               PinStatus[1] ? PSTR("off") : PSTR("on"),
               PinStatus[1] ? PSTR("<font color=\"red\"><b>APAGAR</b></font>") : PSTR("<font color=\"grey\">ENCENDER</font>"),
               PinStatus[2] ? PSTR("off") : PSTR("on"),
               PinStatus[2] ? PSTR("<font color=\"red\"><b>APAGAR</b></font>") : PSTR("<font color=\"grey\">ENCENDER</font>"),
               PinStatus[3] ? PSTR("off") : PSTR("on"),
               PinStatus[3] ? PSTR("<font color=\"red\"><b>APAGAR</b></font>") : PSTR("<font color=\"grey\">ENCENDER</font>"));
}


void setup()
{
  Serial.begin(9600);

  //Luego de mymac, el parámetro que le sigue corresponde a la línea CS. En este caso el CS sale por el PIN 10.
  //REVISAR!!
  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0);

#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
    Serial.println("DHCP ha fallado. Error al asignar IP.");
#endif

  ether.printIp("My Router IP: ", ether.myip);


  ether.printIp("My SET IP: ", ether.myip);
  for (int i = 0; i < 4 ; i++)
  {
    pinMode(LedPins[i], OUTPUT);
    //PinStatus[i]=true;
  }
  for (int i = 0; i < 4; i++)
  {
    leeEEprom = EEPROM.read(i);
    if (leeEEprom == 1)
    {
      digitalWrite (LedPins[i], HIGH);
      PinStatus[i] = true;
    }
    if (leeEEprom == 0)
    {
      digitalWrite (LedPins[i], LOW);
      PinStatus[i] = false;
    }

  }
}


void loop()
{
  t.update();

  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos) {
    bfill = ether.tcpOffset();
    char *data = (char *) Ethernet::buffer + pos;
    if (strncmp("GET /", data, 5) != 0) {
      bfill.emit_p(http_Unauthorized);
    }
    else {

      data += 5;
      if (data[0] == ' ') {
        homePage();
        for (int i = 0; i <= 3; i++)
          digitalWrite(LedPins[i], leeEEprom = EEPROM.read(i));
      }

      //En esta parte se ponen en HIGH las salidas según corresponda.
      else if (strncmp("?ArduinoPIN1=on ", data, 16) == 0) {
        PinStatus[0] = true;
        EEPROM.write(0, 1);
        bfill.emit_p(http_Found);
        id1 = t.after(20000, cambiapin1);
      }
      else if (strncmp("?ArduinoPIN2=on ", data, 16) == 0) {
        PinStatus[1] = true;
        EEPROM.write(1, 1);
        bfill.emit_p(http_Found);
        id2 = t.after(20000, cambiapin2);
      }
      else if (strncmp("?ArduinoPIN3=on ", data, 16) == 0) {
        PinStatus[2] = true;
        EEPROM.write(2, 1);
        bfill.emit_p(http_Found);
        id3 = t.after(20000, cambiapin3);
      }
      else if (strncmp("?ArduinoPIN4=on ", data, 16) == 0) {
        PinStatus[3] = true;
        EEPROM.write(3, 1);
        bfill.emit_p(http_Found);
        id4 = t.after(20000, cambiapin4);
      }

      //En esta parte se ponen en LOW las salidas según corresponda.
      else if (strncmp("?ArduinoPIN1=off ", data, 17) == 0) {
        PinStatus[0] = false;
        EEPROM.write(0, 0);
        bfill.emit_p(http_Found);
        t.stop(id1);
      }
      else if (strncmp("?ArduinoPIN2=off ", data, 17) == 0) {
        PinStatus[1] = false;
        EEPROM.write(1, 0);
        bfill.emit_p(http_Found);
        t.stop(id2);
      }
      else if (strncmp("?ArduinoPIN3=off ", data, 17) == 0) {
        PinStatus[2] = false;
        EEPROM.write(2, 0);
        bfill.emit_p(http_Found);
        t.stop(id3);
      }
      else if (strncmp("?ArduinoPIN4=off ", data, 17) == 0) {
        PinStatus[3] = false;
        EEPROM.write(3, 0);
        bfill.emit_p(http_Found);
        t.stop(id4);
      }

      else {
        //Si la página no se encontró.
        bfill.emit_p(http_Unauthorized);
      }
    }
    ether.httpServerReply(bfill.position());    // send http response
  }
}

void cambiapin1() {
  PinStatus[0] = false;
  EEPROM.write(0, 0);
}

void cambiapin2() {
  PinStatus[1] = false;
  EEPROM.write(1, 0);
}

void cambiapin3() {
  PinStatus[2] = false;
  EEPROM.write(2, 0);
}

void cambiapin4() {
  PinStatus[2] = false;
  EEPROM.write(3, 0);
}
