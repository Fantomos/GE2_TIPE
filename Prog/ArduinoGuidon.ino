#include <LiquidCrystal_I2C.h> // Ecran LCD
#include<SoftwareSerial.h>
#include <SD.h>  // SD

SoftwareSerial BTSerial(6, 7); // Bluetooth
LiquidCrystal_I2C lcd(0x27,16,2);  // initialisation ecran LCD

//Pour l'intervalle de temps
const unsigned long intervalMoy = 500; 
unsigned long lastRefreshTime = 0;

//Pour la carte SD
int pinSDCARD = 5;
File file;

//Pour la lecture de la valeur par bluetooth
byte typeDonnee;
char inDataPuissanceOLD[10];
char inDataPuissance[10]; 
char inDataCadenceOLD[10];
char inDataCadence[10]; 
char inChar;
byte indexP = 0;
byte indexC = 0;

//Pour la moyenne
int lastValue[60]={0};
int lastValueCadence[60]={0};
byte indexLastValue = 0;
int moy;
int oldmoy;

//Mode affichage
byte mode = 0; //==> Initialisation
bool etatBout= false;

void setup() {
  Serial.begin(9600);

  // Initialisation carte SD
  pinMode(pinSDCARD, OUTPUT);
  if (!SD.begin(pinSDCARD)) {
    Serial.println("Erreur initialisation carte SD."); 
  }

  // Initialisation bluetooth
  BTSerial.begin(38400);

  // Initialisation ecran LCD
  lcd.init();                     
  lcd.backlight();
  initLCDMod0();
  

  pinMode(INPUT,8);// Initialisation bouton
  etatBout = digitalRead(8);

  //Connexion des cartes
  while(!BTSerial.available()){}
  mode = 1;
  initLCDMod1();

  
  lastRefreshTime = millis();
}

void loop() {
  // Verif connexion avec deuxième carte arduino
  long debut = millis();
  while(!BTSerial.available()){
    changementMode();
    if(millis()-debut>4000){
      asm volatile ("  jmp 0"); 
    }
  }
  
  //changement de mode via bouton
  changementMode();
  

  // Lecture de la valeur par bluetooth
  indexP=0;
  indexC=0;
  while (BTSerial.available()){

      inChar = BTSerial.read();
      if(inChar == 'P'){
        typeDonnee = 0;
        memset(inDataPuissance, 0, sizeof(inDataPuissance));
      }
      else if(inChar == 'C'){
        typeDonnee = 1;
        memset(inDataCadence, 0, sizeof(inDataCadence));
      }
    
     if(indexP < 10 && indexC < 10 && inChar != 'P' && inChar != 'C') // One less than the size of the array
       {
          if(typeDonnee == 0){
             inDataPuissance[indexP] = inChar; 
             indexP++; 
             inDataPuissance[indexP] = '\0'; 
          }
          else if(typeDonnee == 1){
             inDataCadence[indexC] = inChar; 
             indexC++; 
             inDataCadence[indexC] = '\0'; 
          }
       }
       delay(10);
  }

  if(mode == 1){
    //Affichage sur l'ecran LCD Puissance instantanée
    if(strcmp(inDataPuissanceOLD,inDataPuissance)!= 0){
          // Supprime chiffres deja affichés
          lcd.setCursor(9,0);
          lcd.print("     ");
  
          // Affiche nouveaux chiffres
          lcd.setCursor(9,0);
          lcd.print(inDataPuissance);
          strcpy(inDataPuissanceOLD, inDataPuissance);
    }
  
    //Affichage sur l'ecran LCD Puissance moyenne
    if(moy != oldmoy){
          // Supprime chiffres deja affichés
          lcd.setCursor(9,1);
          lcd.print("     ");
  
          // Affiche nouveaux chiffres
          lcd.setCursor(9,1);
          lcd.print(moy);
          oldmoy=moy;
    }
  }
  else if(mode == 2){
    //Affichage sur l'ecran LCD Cadence instantanée
    if(strcmp(inDataCadenceOLD,inDataCadence)!= 0){
    
          // Supprime chiffres deja affichés
          lcd.setCursor(5,0);
          lcd.print("       ");
  
          // Affiche nouveaux chiffres
          lcd.setCursor(6,0);
          lcd.print(degToTour(atof(inDataCadence)));
        strcpy(inDataCadenceOLD, inDataCadence);
    }
  }
  
  
  //Calcul de la moyenne
   if(millis() - lastRefreshTime >= intervalMoy)
  {
    lastRefreshTime = millis();
    lastValue[indexLastValue] = atoi(inDataPuissance); //'atoi' conversion de char en int
    lastValueCadence[indexLastValue] = atoi(inDataCadence);
    indexLastValue++;
    
    if(indexLastValue == 60){
      indexLastValue=0; //Remise a zero de l'index pour moyenne glissante

      //Ecriture des 60 dernieres valeurs sur la carte SD
      file = SD.open("file.txt", FILE_WRITE);
        if (file) {
            file.println("Puissance : "); 
          for(int j=0;j<60;j++){
            file.println(lastValue[j]); 
          }
           file.println("Cadence : "); 
          for(int j=0;j<60;j++){
            file.println(lastValueCadence[j]); 
          }
           file.close(); 
          Serial.println("Ecriture");
        }
        else{
          Serial.println("Could not open file (writing).");
      }
    }
    moy=moyenne();
 }

 

   
  
}




// Initialisation ecran LCD
void initLCDMod0(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Initialisation...");
}

void initLCDMod1(){
   lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("P.inst : ");
  lcd.setCursor(15,0);
  lcd.print("W");
  lcd.setCursor(0,1);
  lcd.print("P.moy : ");
  lcd.setCursor(15,1);
   lcd.print("W");
}

void initLCDMod2(){
   lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Cad : ");
  lcd.setCursor(12,0);
  lcd.print("tr/m");

}



int moyenne(){
 int somme=0;
 int nbelement=0;
 for (int i = 0 ; i < 60 ; i++)
    {
        if(lastValue[i]!=0){
           somme += lastValue[i] ; 
           nbelement++;
        }
       
    } 

  int result = somme/nbelement;
   return result;
}

//Conversion deg/s à tr/min
int degToTour(double degs){
  return round((degs/360)*60);
}


void changementMode(){
  bool etatMtn = digitalRead(8);
    if(etatBout != etatMtn ){
      if(etatBout == false){
        mode = (mode % 2)+1;//1=
        delay(100);
        if(mode == 1){
          initLCDMod1();
        }
        else if(mode == 2){
          initLCDMod2();
        }
       }
      etatBout = etatMtn;
    }
}
