//lib pour la LED RGB
#include <ChainableLED.h>

//lib pour la gestion de la carte SD
//xxx

//libs pour les capteurs
//lib pour l'horloge RTC
#include <Wire.h>
#include <RTC.h>
static DS1307 RTC;
//lib pour le capteur de température, hygrométrie et pression
#include <SPI.h>
#include <BME280I2C.h>
BME280I2C bme;
//lib pour le capteur GPS
#include <SoftwareSerial.h> 
#include <TinyGPS.h>
#define RX 3 // Affectation des broches pour la liaison série logicielle
#define TX 4 // de l'Arduino
TinyGPS gps;
SoftwareSerial GPS(RX, TX); // Création de l'objet GPS pour la liaison série

//partie qui contient les fontions du mode configuration
#include "mode_conf.h"

//défini les pin de la LED RGB
#define RED_BUTTON_PIN 2
#define GREEN_BUTTON_PIN 3

//défini les pin pour les capteurs
#define CAP_LUM_PIN A0

//filtre de rebond des boutons en microsecondes
#define BUTTON_BOUNCE_FILTER 10000

//définition des différents modes
#define STANDARD 0
#define CONFIG 1
#define ECO 2
#define MAINTENANCE 3

//définition de la couleur de chaque mode pour la LED
const byte modeColor[4][3] = {
	{0, 255, 0},   //couleur pour mode STANDARD
	{255, 255, 0}, //couleur pour mode CONFIG
	{0, 0, 255},   //couleur pour mode ECO
	{255, 128, 0}  //couleur pour mode MAINTENANCE
};

//définition des différents capteurs
#define CAPTEUR_PRES_TEMP_HUM 0
#define CAPTEUR_GPS 1
#define CAPTEUR_LUM 2

byte currentMode; //enregistre le mode actuel
byte lastMode; //enregistre le mode précédent

//enregistre le temp pour l'intervalle de log_intervall
unsigned long LOG_INTERVAL_millis;
//enregistre le temps écoulé depuis la dernière lecture des capteurs
unsigned long last_sensor_call = 0; 

typedef struct{
	bool state = false;
	unsigned long duration = micros();
} Button;

typedef struct{
	//float Tab_Mesures[Nb_Val] = {0};
	float last_value;
	unsigned long last_call;
	int Nb_erreur = 0;
	bool actif = 1;
} Capteur;

//definitions des différents boutons et capteurs
Button red_button, green_button;

Capteur capt_pres, capt_temp, capt_hum, capt_gps, capt_lum;
Capteur capteurs[] = {capt_pres, capt_temp, capt_hum, capt_gps, capt_lum};

//on definit la LED RGB avec son clk_pin sur 6 et data sur 7 avec la quantite de leds
ChainableLED led(6, 7, 1);

void erreur_capteur(unsigned int *);
void button(Button*);
void red_button_action();
void green_button_action();
void state_checker();
bool isMode(byte);
void changeMode(byte);
void mode_caller();
void standard(bool);
void config();
void maintenance();
void acquisition(bool *);

void setup(){
	Serial.begin(9600);

	//initialise la LED RGB
	led.init();

	//initialise l'horloge RTC
	RTC.begin();

	//initialise le GPS
	GPS.begin(9600);

	//initialise le capteur de température/hygrométrie/pression
	bme.begin();

	//mode initiale est STANDARD
	changeMode(STANDARD);

	pinMode(RED_BUTTON_PIN, INPUT);
	pinMode(GREEN_BUTTON_PIN, INPUT);
	//vérifie si le bouton rouge est pressé durant le démarage
	if(!digitalRead(RED_BUTTON_PIN)) changeMode(CONFIG);

	attachInterrupt(digitalPinToInterrupt(RED_BUTTON_PIN), red_button_action, CHANGE);
	attachInterrupt(digitalPinToInterrupt(GREEN_BUTTON_PIN), green_button_action, CHANGE);
}

void loop(){
	//gère le changement de modes
	state_checker();

	//appel et execute les actions du mode sélectionné
	mode_caller();
}



void button(Button *ptrButton){
	//filtre les rebonds
	if((micros() - ptrButton->duration) < BUTTON_BOUNCE_FILTER) return;

	//initialise le moment d'appui ainsi que l'état
	ptrButton->duration = micros();
	ptrButton->state = !ptrButton->state;


	/*if(ptrButton->state){
		//executé quand bouton enfoncé
	} else {
		//executé quand bouton relaché
	}*/
}

void red_button_action(){
	//appel la fonction qui va gérer les conséquences de ce bouton
	button(&red_button);
}

void green_button_action(){
	//appel la fonction qui va gérer les conséquences de ce bouton
	button(&green_button);
}

void state_checker(){
	//initialise l'état enregistré pour correspondre à l'état physique
	//car des rebonds peuvent les dérégler
	red_button.state = !digitalRead(RED_BUTTON_PIN);
	green_button.state = !digitalRead(GREEN_BUTTON_PIN);

	//vérifie si un des deux bouton est actionné
	if(!red_button.state && !green_button.state) return;


	if(red_button.state){
		//je met le temps de micro en secondes
		byte redTime = (micros() - red_button.duration) / 1000000;


		//si BTN rouge appuyé 5s et mode standard -> maintenance
		if(redTime >= 5 && isMode(STANDARD)){
			changeMode(MAINTENANCE);

		//si BTN rouge appuyé 5s et mode economique -> standard
		} else if(redTime >= 5 && isMode(ECO)){
			changeMode(STANDARD);

		//si BTN rouge appuyé 5s et mode maintenance -> dernier mode
		} else if(redTime >= 5 && isMode(MAINTENANCE)){
			changeMode(lastMode);
		}

	
	} else {
		//je met le temps de micro en secondes
		byte greenTime = (micros() - green_button.duration) / 1000000;


		//si BTN vert appuyé 5s et mode standard -> economique
		if(greenTime >= 5 && isMode(STANDARD)){
			changeMode(ECO);

		//si BTN vert appuyé 5s et mode economique -> maintenance
		} else if(greenTime >= 5 && isMode(ECO)){
			changeMode(MAINTENANCE);
		}
	}
}

bool isMode(byte mode){
	return mode == currentMode ? true : false;
}

void changeMode(byte mode){
	lastMode = currentMode;

	//change le mode actuel
	currentMode = mode;

	//configure la LED sur la bonne couleur
	led.setColorRGB(0, modeColor[mode][0], modeColor[mode][1], modeColor[mode][2]);

	//reset le timer des bouton
	red_button.duration = micros();
	green_button.duration = micros();
}

void mode_caller(){
	//appel le mode actuellement sélectionné
	switch (currentMode){
		
	case STANDARD:
		standard(0);
		break;

	case CONFIG:
		config();
		break;

	case ECO:
		standard(1);
		break;

	case MAINTENANCE:
		maintenance();
		break;
	
	default:
		//pas cansé être exécuté
		break;
	}
}

void standard(bool eco){
	int log_intervall;
	EEPROM.get(0, log_intervall);
	if(((unsigned int)log_intervall * 3600 * (eco + 1)) < (millis() - LOG_INTERVAL_millis)){
		acquisition(&eco);
	}
}

void config(){
	if (Serial.available()){
    	String SerialReponse = Serial.readString();
    	check_command(&SerialReponse);
  }
}

void maintenance(){
}

void acquisition(bool *eco){
	for(unsigned int num_capteur = 0; num_capteur < sizeof(capteurs) / sizeof(Capteur); num_capteur++){
		unsigned int timeout_cap;
		switch (num_capteur){

		case CAPTEUR_PRES_TEMP_HUM:
			float temp(NAN), hum(NAN), pres(NAN);
			BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
			BME280::PresUnit presUnit(BME280::PresUnit_Pa);
			bme.read(pres, temp, hum, tempUnit, presUnit);
			Serial.println("Pression : " + String(pres));
			Serial.println("Température : " + String(temp));
			Serial.println("Humidité : " + String(hum));

			//on a du mettre cette boucle car ce capteur en gère 3
			for(byte i = 0; i < 3; i++){
				bool cap_state;
				EEPROM.get(10 + i * 3, cap_state);
				capteurs[i].actif = cap_state;
				if(capteurs[i].actif == false)
					break;

				EEPROM.get(4, timeout_cap);
				capteurs[i].last_value != NAN ? capteurs[i].last_call = millis() : 0;

				//si la pression est en erreur et timeout dépassé
				if(pres == NAN){
					if((millis() - capteurs[i].last_call) > timeout_cap * 3600){
						erreur_capteur(&num_capteur);
						break;
					}
					break;
				}
				//écrire donné
			}

			capt_pres.last_value = pres;
			capt_temp.last_value = temp;
			capt_hum.last_value = hum;

		case CAPTEUR_GPS:
			*eco ? capteurs[num_capteur + 2].actif = !capteurs[num_capteur + 2].actif : 0;
			if(capteurs[num_capteur + 2].actif == false)
				break;
			
			byte recu;
			float slat,slon = 0;
			while(GPS.available()){
				recu = GPS.read(); // Lecture de la trame envoyée par le module GPS
			
				if (gps.encode(recu)){
					gps.f_get_position(&slat, &slon);
					Serial.print("Latitude :");
					Serial.println(slat, 6);
					Serial.print("Longitude:");
					Serial.println(slon, 6);
				}
			}

			EEPROM.get(4, timeout_cap);
			capteurs[num_capteur + 2].last_value != NAN ? capteurs[num_capteur + 2].last_call = millis() : 0;

			//si la pression est en erreur et timeout dépassé
			if(slat + slon == 0){
				if((millis() - capteurs[num_capteur + 2].last_call) > timeout_cap * 3600){
					erreur_capteur(&num_capteur);
					break;
				}
				break;
			}
			//écrire donné

		case CAPTEUR_LUM:
			int val = analogRead(CAP_LUM_PIN);
			bool cap_state;
			EEPROM.get(5, cap_state);
			capteurs[num_capteur + 2].actif = cap_state;
			if(capteurs[num_capteur + 2].actif == false)
				break;

			EEPROM.get(4, timeout_cap);
			capteurs[num_capteur + 2].last_value != NAN ? capteurs[num_capteur + 2].last_call = millis() : 0;

			//si la pression est en erreur et timeout dépassé
			if(val == 0){
				if((millis() - capteurs[num_capteur + 2].last_call) > timeout_cap * 3600){
					erreur_capteur(&num_capteur);
					break;
				}
				break;
			}
			//écrire donné
		}
	}
}

//void write_data(byte *num_cap, float *data)

void erreur_capteur(unsigned int *num_cap){
	switch (*num_cap){

	case 0:
		//démarrer erreur pour capteur de pression
		break;
	
	case 1:
		//démarrer erreur pour capteur de température
		break;
	
	case 2:
		//démarrer erreur pour capteur de humidité
		break;
	
	case 3:
		//démarrer erreur pour capteur de gps
		break;
	
	case 4:
		//démarrer erreur pour capteur de lumière
		break;
	}
}