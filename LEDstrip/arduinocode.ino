#include <Adafruit_NeoPixel.h>

#define PIN 6

// this code will drive a neopixel string, and will receive commands over a serial connection. Commands will take the form
// XN.M where
// X is the command (C, M, S, V, ...)
// N is the host number (1, 2, 3)
// M is a parameter (C needs cpuload, M needs memload, S needs state, V needs targethost)


Adafruit_NeoPixel strip = Adafruit_NeoPixel(144, PIN, NEO_GRB + NEO_KHZ800);
// layout for 3 host zones on 144 pixels: 144/3=48. We position each host zone at the center of these 48 pixels:
// 24 A 48 B 48 C 24
#define posA 24
#define posB 72
#define posC 120
// these positions could be calculated for an arbitrary number of hosts. To be implemented...
#define hostradius 10


// serial communication buffer string
String serialReceived, command, hoststring, argsstring;
char commandChar, hostChar, argsChar;
const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data

void setup() {
  // put your setup code here, to run once:
  strip.begin();               // initialize strip
  strip.show();                // Update all LEDs (= turn OFF, since none of them have been set yet!) // put your main code here, to run repeatedly:
  Serial.begin(115200);              //Starting serial communication
  while (!Serial) { // wait for port to be ready
   hostload(1, 1, 1); // prepare minimal intensity on all host zones
   hostload(2, 1, 1);
   hostload(3, 1, 1);
  }
    
  //Tell the computer that we're ready for data
  Serial.println("RDY");
}

void loop() {
  // main loop

  //Serial.setTimeout(10000);
   
  
  if (Serial.available() > 0) { // if bytes are available through the serial port
  char commandChar = Serial.read(); //read the first byte
  int host = Serial.parseInt();
  char separator = Serial.read(); //unused
  switch (commandChar) {
    case 'M': 
      {
        int load = Serial.parseInt();
        hostmemload(host, load);
      };
      break;
    case 'C':
      {
        int load = Serial.parseInt();
        hostcpuload(host, load);
      };
      break;
    case 'V':
      {
        int sourcehost = host;
        int targethost = Serial.parseInt();
        vMotion(50,sourcehost,targethost);
      };
      break;
    case 'S':
      {
        int state = Serial.parseInt();
        hoststate(host,state);
      };
      break;
    case 'D':
      strip.show();
      break;
    default:
      //while (Serial.available() > 0) Serial.read();
      //vMotion(10,1,2);
      break;
    };
  };

    // Clear up the serial buffer
    //while (Serial.available() > 0) Serial.read();
    
    // Let the sender know we're ready for more data
    delay(100);
    Serial.println("RDY");    

};

int hostpos(uint8_t host) {
  switch (host) {
    case 1:
      return posA;
      break;
    case 2:
      return posB;
      break;
    case 3:
      return posC;
      break;
  }
}

void hoststate(uint8_t host, uint8_t state) {
  // 0: disconnected
  // 1: standby mode
  // 2: not responding
  // 3: maintenance mode
  // 4: powered off
    int basepos = hostpos(host) - hostradius;
    int middlepos = hostpos(host);
  for (uint16_t i = 0; i < 2*hostradius+1; i++) {
    strip.setPixelColor(basepos  + i, strip.Color(0, 0, 0));
  };
  switch (state) {
    case 0: // disconnected
      for (uint16_t i = 1; i < min(hostradius,4); i++) {
        strip.setPixelColor(middlepos - hostradius + i, strip.Color(40, 0, 0));
        strip.setPixelColor(middlepos + hostradius - i, strip.Color(40, 0, 0));
      };
      break;
    case 1: // standby mode
      for (uint16_t i = 1; i < min(hostradius,5); i++) {
        strip.setPixelColor(middlepos - hostradius + i, strip.Color(0, 10, 0));
        strip.setPixelColor(middlepos + hostradius - i, strip.Color(0, 10, 0));
      };
      break;
    case 2: // not responding
      for (uint16_t i = 1; i < min(hostradius,8); i++) {
        strip.setPixelColor(middlepos - hostradius + i, strip.Color(255, 0, 0));
        strip.setPixelColor(middlepos + hostradius - i, strip.Color(255, 0, 0));
      };
      break;
    case 3: // maintenance mode
      for (uint16_t i = 1; i < min(hostradius,8); i++) {
        strip.setPixelColor(middlepos - hostradius + i, strip.Color(128, 83, 0));
        strip.setPixelColor(middlepos + hostradius - i, strip.Color(128, 83, 0));
      };
      break;
    case 4: // powred off
      for (uint16_t i = 1; i < min(hostradius,3); i++) {
        strip.setPixelColor(middlepos + i, strip.Color(10, 10, 10));
        strip.setPixelColor(middlepos - i, strip.Color(10, 10, 10));

      };
      break;
  }
  strip.show();
}


void hostload(uint8_t host, uint8_t cpuload, uint8_t memload) {
  // cpuload and memload should be a percentage [0-100]
  hostcpuload(host, int(cpuload));
  hostmemload(host, int(memload));
  strip.show();
}

void hostcpuload(uint8_t host, uint8_t load) {
  // cpuload and memload should be a percentage [0-100]
  int basepos = hostpos(host) - hostradius;
  for (uint16_t i = 0; i < hostradius; i++) {
    strip.setPixelColor(basepos + 2 * i, strip.Color(0, 0, load * 2));
  }
  strip.show();

}

void hostmemload(uint8_t host, uint8_t load) {
  int basepos = hostpos(host) - hostradius;
  for (uint16_t i = 0; i < hostradius; i++) {
    strip.setPixelColor(basepos + 1 + 2 * i, strip.Color(0, load * 2, 0));
  }   strip.show();

}

//vMotion animation
void vMotion(uint8_t wait, uint8_t sourcehost, uint8_t targethost) {
  uint32_t c = strip.Color(50, 50, 50); //color for animation
  int sourcepos = hostpos(sourcehost);
  int targetpos = hostpos(targethost);
  int direction = 1;
  switch (3 + targethost - sourcehost) {
    case 1:
      direction = 1;
      break;
    case 2:
      direction = -1;
      break;
  }
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 4; q++) {
      for (int i = hostradius+1; i < (posB - posA - hostradius - 1); i = i + 4) {
        int currentpos = (strip.numPixels() + sourcepos + (direction * (i + q))) % strip.numPixels();
        strip.setPixelColor(currentpos, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i = hostradius+1; i < (posB - posA - hostradius - 1); i = i + 4) {
        int currentpos = (strip.numPixels() + sourcepos + (direction * (i + q))) % strip.numPixels();
        strip.setPixelColor(currentpos, 0);        //turn every third pixel off
      }
      strip.show();

    }
  }
}






