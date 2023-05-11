#include <TFT.h> // Hardware-specific library
#include <SPI.h>
// #include <SD.h>

//analogni vhod za merjenje toka
#define ISensor A1
#define hall A2

//digital for text
#define CS   10
#define DC   9
#define RESET  8

//digital for displaying img (BMP)
#define PIN_SD_CS 4
#define PIN_TFT_CS 10
#define PIN_DC 9
#define PIN_RST 8
//Zakasnitev pred menjavo slike
#define DELAY_IMAGE_SWAP 60000 // each image is shown for 60 seconds

//dolžina spremenljivke za prikaz toka
#define charsize 6

//tipke in rele
#define StartTipka 6 //start
#define StopTipka 7 //stop
#define StanjeRele 5

//mejne vrednosti opozoril
#define mejnaVrednost 5*0.9
#define opozorilnaVrednost 5*0.7


TFT myScreen = TFT(CS, DC, RESET);

// variable to keep track of the elapsed time
int counter = 0;
// char array to print on screen 
char printout[charsize];

int interval1;
int interval2;
char printoutToClear[charsize];

//stanje releja 
bool rele = false;
bool blokRele = false;

float current; 
int refresh = 1000;
int THall = 10;
int procentHall;

//Hall mesuring variables 
#define calibration_const 355.55
int max_val;
int new_val;
int old_val = 0;
float rms;
float IRMS;

//izpis kvadratkov / LED 
int yPos;

float mesure(){ // ----------------------------------funkcija merjenja----------------------------------
    unsigned int x=0;
    float AcsValue=0.0,Samples=0.0,AvgAcs=0.0,AcsValueF=0.0;
    
      for (int x = 0; x < 150; x++){ //Get 150 samples
      AcsValue = analogRead(ISensor);     //Read current sensor values   
      Samples = Samples + AcsValue;  //Add samples together
      delay (3); // let ADC settle before next sample 3ms
    }
    AvgAcs=Samples/150.0;//Taking Average of Samples
    
    //((AvgAcs * (5.0 / 1024.0)) is converitng the read voltage in 0-5 volts
    //2.5 is offset(I assumed that arduino is working on 5v so the viout at no current comes
    //out to be 2.5 which is out offset. If your arduino is working on different voltage than 
    //you must change the offset according to the input voltage)
    //0.185v(185mV) is rise in output voltage when 1A current flows at input
    AcsValueF = (2.5 - (AvgAcs * (5.0 / 1024.0)) )/0.185;
    
    Serial.println(AcsValueF);//Print the read current on Serial monitor
    return AcsValueF;
  }

// void init_SD() { //--------------------------inicializacija SD kartice------------------------------------
//   // try to init SD card
//   Serial.print(F("SD card init..."));
//   if (!SD.begin(PIN_SD_CS)) {
//     Serial.println(F("ERROR")); // failed
//     return;
//   }
//   Serial.println(F("SUCCESS")); // ok
// }

void setup(){

  pinMode(INPUT, StartTipka);
  pinMode(INPUT, StopTipka);
  pinMode(OUTPUT, StanjeRele);
  
  Serial.begin(9600); //Start Serial Monitor to display current read value on Serial monitor
  while (!Serial) {
    // wait until default serial connection is fully set up
  }
  
  //inicializiraj ekran
  //myScreen.begin();
  myScreen.initR(INITR_BLACKTAB);
  myScreen.setRotation(2);
  
  // clear the screen
  myScreen.background(0,0,0); 
  myScreen.stroke(255,0,255);
  
  // static text
  myScreen.setTextSize(1);
  myScreen.text("Meritev toka je:",0,0);
  myScreen.text("[A]",0,30);
  
}

void loop(){

  procentHall = map(IRMS, 0, 5, 0, 100); //Mapiraj vrednosti toka 0%-100%, tole zna ne delat --> problem v kalkulaciji IRMS 
 
  Rele(); 

  //-------------------------------updating text-------------------------------
  if ((millis() - interval1) > refresh){ 
    ScreenHall();
    ScreenProcent();
    ScreenProgBar();
    interval1 = millis();
  }

  //PrintSD(); // če popraviš knjižnico za branje SD kartice
}

void Hall(){
  if ((millis() - interval2) > THall){
  new_val = analogRead(hall);
    if(new_val > old_val) {
      old_val = new_val;
    }
  
  else {
      delayMicroseconds(50);
      new_val = analogRead(A0);
        if(new_val < old_val) {
          max_val = old_val;
          old_val = 0;
        }
      
      rms = max_val * 5.00 * 0.707 / 1024;
      IRMS = rms * calibration_const;
      
      Serial.print("  IRMS: ");
      Serial.println(IRMS);
      
      interval2 = millis();
    }
  }
}

// ---------------------------handling releja za DC meritve ------------------------------------
// void Rele(){
//     //vpiši stanje releja na izhod
//   digitalWrite(StanjeRele, rele);
//   //beri ukaz za rele
//   if ((mesure() >= mejnaVrednost)||(mesure() <= -mejnaVrednost)){
//     rele = false;
//   }
//   else if(digitalRead(StopTipka)==HIGH){
//     rele = false;
//   }
//   else if(digitalRead(StartTipka)==HIGH){
//     rele = true;
//     myScreen.noStroke(); // don't draw a line around the next rectangle
//     myScreen.fill(0,0,0); // set the fill color to black
//     myScreen.rect(98,10,20,20); //draw a rectangle across the screen // y pos, x pos, sizey, sizex
//   }
// }

//handling releja za HALL meritve
void Rele(){
    //vpiši stanje releja na izhod
  digitalWrite(StanjeRele, rele);
  //beri ukaz za rele
  if ((mesure() >= mejnaVrednost)||(mesure() <= -mejnaVrednost)){
    rele = false;
  }
  else if(digitalRead(StopTipka)==HIGH){
    rele = false;
  }
  else if(digitalRead(StartTipka)==HIGH){
    rele = true;

    myScreen.noStroke(); // don't draw a line around the next rectangle
    myScreen.fill(0,0,0); // set the fill color to black
    myScreen.rect(98,10,20,20); //draw a rectangle across the screen // y pos, x pos, sizey, sizex
  }
}

// --------------------------------izpis meritev z DC senzorjem ------------------------------------
// void Screen(){
//
//   //-----------------------------------izpis merjene vrednosti DC meritev-----------------------------
//     // convert mesure to a string
//   String transform = String (mesure());
//   // add converted mesure(type: string) to an array
//   transform.toCharArray(printout,charsize);
//
//     // increase font size for text in loop()
//     myScreen.setTextSize(3);
//
//     myScreen.stroke(0,0,0); //fill brush with black
//     myScreen.text(printoutToClear,0,10);
//
//     myScreen.stroke(0,255,255); //fill brush with color
//     myScreen.text(printout,0,10);
//
//     //nauč se kako kopirat array, da boš lahka zbrisal prejšn tekst preden na njegovo mesto napišeš novga... Narjen
//     memcpy(printoutToClear, printout, strlen(printout)); //Duplicate array to store in data 
//
//     //---------------------------------izpis kvadratkov ---------------------------------
//     if (rele == false){
//       //screen square draw RED
//       myScreen.noStroke(); // don't draw a line around the next rectangle
//       myScreen.fill(0,0,255); // set the fill color to red
//       //y pos, x pos, sizey, sizex
//       myScreen.rect(98,10,20,20); //draw a rectangle across the screen
//     }
//
//     if((mesure() >= opozorilnaVrednost)||(mesure() <= -opozorilnaVrednost)){
//       //screen square draw YELLOW
//       myScreen.fill(0,255,255); // set the fill color to black
//       //y pos, x pos, sizey, sizex
//       myScreen.rect(98,40,20,20); //draw a rectangle across the screen
//     }
//     else{ //izbriši pravokotnik
//       myScreen.noStroke(); // don't draw a line around the next rectangle
//       myScreen.fill(0,0,0); 
//       //y pos, x pos, sizey, sizex
//       myScreen.rect(98,40,20,20); //draw a rectangle across the screen
//     }
//
//     if (rele == true){
//       //screen square draw GRREN
//       myScreen.fill(0,255,0); // set the fill color to green
//       //y pos, x pos, sizey, sizex
//       myScreen.rect(98,70,20,20); //draw a rectangle across the screen
//     }
//     else{ //izbriši pravokotnik
//       myScreen.noStroke(); // don't draw a line around the next rectangle
//       myScreen.fill(0,0,0); // set the fill color to black
//       //y pos, x pos, sizey, sizex
//       myScreen.rect(98,70,20,20); //draw a rectangle across previous rectangle 
//     }
//
// }

// izpis meritev z HALL senzorjem 
void ScreenHall(){
  //-----------------------------------izpis merjene vrednosti DC meritev-----------------------------
    // convert mesure to a string
  String transform = String (IRMS); //<-- tu vstavi vrednost za izpis na zaslonu
  // add converted mesure(type: string) to an array
  transform.toCharArray(printout,charsize);
  
    // increase font size for text in loop()
    myScreen.setTextSize(3);
    
    myScreen.stroke(0,0,0); //fill brush with black
    myScreen.text(printoutToClear,0,10);
    
    myScreen.stroke(0,255,255); //fill brush with color
    myScreen.text(printout,0,10);

    //nauč se kako kopirat array, da boš lahka zbrisal prejšn tekst preden na njegovo mesto napišeš novga... Narjen
    memcpy(printoutToClear, printout, strlen(printout)); //Duplicate array to store in data 
}

// izpis meritev z HALL senzorjem 
void ScreenProcent(){
        //-----------------------------------izpis merjene vrednosti DC meritev-----------------------------
    // convert procentHall to a string
  String transform = String (procentHall); //<-- tu vstavi vrednost za izpis na zaslonu
  // add converted mesure(type: string) to an array
  transform.toCharArray(printout,charsize);
  
    // increase font size for text in loop()
    myScreen.setTextSize(1);
    
    myScreen.stroke(0,0,0); //fill brush with black
    myScreen.text(printoutToClear,0,30);
    
    myScreen.stroke(0,255,255); //fill brush with color
    myScreen.text(printout,0,30);
    
    //nauč se kako kopirat array, da boš lahka zbrisal prejšn tekst preden na njegovo mesto napišeš novga... Narjen
    memcpy(printoutToClear, printout, strlen(printout)); //Duplicate array to store in data 
}

// izpis meritev z HALL senzorjem 
void ScreenProgBar(){
      //---------------------------------PROGRESS BAR ---------------------------------
    // width 128 - ()
    myScreen.fill(255,255,255); // set the fill color to white
    myScreen.rect(40,14,20,procentHall); //draw a rectangle across previous rectangle (y pos, x pos, sizey, sizex)
    myScreen.fill(0,0,0); // set the fill color to black
    myScreen.rect(40,14+procentHall,20,100-procentHall); //draw a rectangle across previous rectangle (y pos, x pos, sizey, sizex)
}

//----------------------------------------uporaba SD kartice --------------------------------------------------
void PrintSD(){
  //  //------------------------------printing image----------------------------------------------
//  File dir = SD.open("/"); // open root path on SD card
//  File entry;
//  char name[16];
//  bool worked_once = false;
//
//  while (entry = dir.openNextFile()) { // iteratively opens all files on SD card. 
//    Serial.print(F("Opened File: "));
//    Serial.println(entry.name());
//    strcpy(name, entry.name()); // file name is copied to variable "name"
//    entry.close(); // After copying the name, we do not need the entry anymore and, therefore, close it.
//    int filename_len = strlen(name);
//    if ((filename_len >= 4 && strcmp(name + filename_len - 4, ".BMP") == 0)) { // check if the current filename ends with ".BMP". If so, we might have an image.
//      PImage image = myScreen.loadImage(name); // loads the file from the SD card
//      if (image.isValid()) { // If the loaded image is valid, we can display it on the TFT.
//        Serial.println(F("Image is valid... drawing image."));  
//        myScreen.image(image, 0, 0); // this function call displays the image on the TFT. 
//        worked_once = true; // we set this variable to true, in order to indicate that at least one image could be displayed
//        delay(DELAY_IMAGE_SWAP);
//      } else {
//        Serial.println(F("Image is not valid... image is not drawn."));  
//      }  
//      image.close(); // image is closed as we do not need it anymore.
//    } else {
//      Serial.println(F("Filename does not end with BMP!"));  
//    }
//  }
//  dir.close(); // directory is closed
//
//  if (worked_once == false) { // if not a single image could be shown, we reconnect to the SD card reader.
//    Serial.println(F("Warning: Printing an image did not work once! Trying to reinitalize SD card reader."));        
//    SD.end();
//    init_SD();
//  }
}

void ScreenLED(){
  //---------------------------------izpis kvadratkov ---------------------------------
  //y pozicija vseh kvadratkov 
  yPos = 70; 

    if (procentHall <= 75){
      Green(33);
    }
    else{
      Black(33);
    }

    if ((procentHall >= 75)&&(procentHall <= 90)){
      Yellow(66);
    }
    else{
      Black(66);
    }

    if (procentHall >= 90){
      Red(99);
    }
    else{
      Black(99);
    }
}

void Green(int a){
  myScreen.fill(0,255,0);
  myScreen.rect(14+a,yPos,20,20);
}

void Yellow(int a){
  myScreen.fill(0,255,255);
  myScreen.rect(14+a,yPos,20,20);
}

void Red(int a){
  myScreen.fill(0,0,255);
  myScreen.rect(14+a,yPos,20,20);
}

void Black(int a){
  myScreen.fill(0,0,0);
  myScreen.rect(14+a,yPos,20,20);
}