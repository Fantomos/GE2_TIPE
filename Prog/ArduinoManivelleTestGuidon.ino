// Bluetooth
#include <SoftwareSerial.h>
SoftwareSerial BTSerial(2, 3); // RX | TX


const unsigned long interval2 = 1000; 
unsigned long lastRefreshTime = 0;

  
void setup() 
{
  Serial.begin(9600);

  //Bluetooth     Adresse MAC HC-06 : 98D3:31:FB1CB7
  BTSerial.begin(38400);  

  
}
 
void loop()
{

 if (BTSerial.available()){
    Serial.write(BTSerial.read());
  }
 
  if (Serial.available()){
  BTSerial.write(Serial.read());
  }
  
  if(millis() - lastRefreshTime >= interval2)
  {
    lastRefreshTime = millis();
    int a = random(1,100);
    BTSerial.print(a);
    Serial.println(a);
  }
}




