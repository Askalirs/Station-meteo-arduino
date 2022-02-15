#include <BME280I2C.h>
#include <Wire.h>
#include <RTC.h>
#include <SoftwareSerial.h> 
#include <TinyGPS.h>
#include <SPI.h>
#include <SD.h>
#define RX 3 // Affectation des broches pour la liaison série logicielle
#define TX 4 // de l'Arduino
#define capteur A0                                  // affectation des broches       
BME280I2C bme;  
TinyGPS gps; 
static DS1307 RTC;
SoftwareSerial GPS(RX, TX); // Création de l'objet GPS pour la liaison série
//entre l'Arduino et le module GP       
File myFile;
String name_file; 
int nb_rec=0;

void setup(){
  Serial.begin(9600);
  Wire.begin(9600);
  RTC.begin();
  RTC.startClock();
  GPS.begin(9600); 
  String annee,mois,jour;
  format(&annee,&mois,&jour);
  name_file = annee+mois+jour+"_"+String(nb_rec)+".txt";
  Serial.println(annee+mois+jour+"_"+String(nb_rec)+".txt");
  while(!bme.begin()){
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }
  if(!SD.begin(4)){
    Serial.println("initialization failed!");
    while (1);
  }
}


void loop(){
  pression();
  location();
  luminosite();
  verification();
  
  delay(500);
}


void pression(){
   float temp(NAN), hum(NAN), pres(NAN);
   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_Pa);
   bme.read(pres, temp, hum, tempUnit, presUnit);
   write_data(pres," Pression :"); 
   write_data(temp," Temperature :"); 
   write_data(hum," Humidite :"); 
}


void location(){
  while(GPS.available()){
    byte recu;
    recu = GPS.read(); // Lecture de la trame envoyée par le module GPS
  if (gps.encode(recu)){
      float slat,slon;
      gps.f_get_position(&slat, &slon);
      //Serial.print("Latitude :");
      //Serial.println(slat, 6);
      //Serial.print("Longitude:");
      //Serial.println(slon, 6);
      write_data(slat," Latitude :");
      write_data(slon," Longitude :");
    }
  }
}

void luminosite(){
  int lumiere;
  lumiere=analogRead(capteur);         // conversion AN       
  //Serial.print("lumiere:   ");       
  //Serial.println(lumiere);  // affichage dans le moniteur série de la valeur de la luminosité
  write_data(lumiere," Lumiere :");
  }

void write_data(float donne, String txt){
  myFile=SD.open(name_file, FILE_WRITE);
  myFile.print(txt);
  myFile.print(donne);
  myFile.close();
  }

void verification(){
  int FILE_MAX_SIZE=2000;
  myFile=SD.open(name_file);
  if(FILE_MAX_SIZE<myFile.size()){
    myFile.close();
    nb_rec=nb_rec+1;
    String annee,mois,jour;
    format(&annee,&mois,&jour);
    if (nb_rec==10){
      nb_rec=0;
      myFile=SD.open(name_file);
      if(SD.exists(name_file)){
        SD.remove(name_file); 
      }
      name_file = annee+mois+jour+"_"+String(nb_rec)+".txt";
    }
    else{
      myFile=SD.open(name_file);
      if(SD.exists(name_file)){
        SD.remove(name_file); 
      }
      name_file = annee+mois+jour+"_"+String(nb_rec)+".txt";
    }
  }
  myFile.close();
}


void format(String *years,String *months,String *jourr){
    String yy=String(RTC.getYear());
    *years=yy.substring(2,4);
  if (String(RTC.getMonth()).length()==1){
    *months="0"+String(RTC.getMonth());
    }
  else{
    *months=String(RTC.getMonth());
    }
  if (String(RTC.getDay()).length()==1){
    *jourr="0"+String(RTC.getDay());
    }
  else{
    *jourr=String(RTC.getDay());
    }
  }
