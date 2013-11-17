/*Yuktix Technologies Pvt Ltd.: code to send data to a remote server */
//Program description : Connect to internet and ping to a server.


#include <SoftwareSerial.h>

SoftwareSerial GprsSerial( 4,5 );

void setup(){GprsSerial.begin(9600);
  Serial.begin(9600);

  Serial.println("Config SIM900...");
  delay(2000);
  Serial.println("Done!...");
  GprsSerial.flush();
  Serial.flush();

  // attach or detach from GPRS service //if = 1, that means GPRS service is active over SIm and 0 means not.
  GprsSerial.println("AT+CGATT?");
  delay(1000);
  toSerial();


  // bearer settings
  GprsSerial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(2000);
  toSerial();

  // bearer settings
  GprsSerial.println("AT+SAPBR=3,1,\"APN\",\"aircelgprs\""); //here rcomnet is the APN name for reliance in Delhi, you have to change it to the APNname for your provider
  delay(2000);
  toSerial();

  // bearer settings
  GprsSerial.println("AT+SAPBR=1,1");
  delay(2000);
  toSerial();
}


void loop()
{
   // initialize http service
   GprsSerial.println("AT+HTTPINIT=?");
   delay(2000);
   toSerial();
   
   GprsSerial.println("AT+HTTPINIT");
   delay(2000);
   toSerial();

   // set http param value
   GprsSerial.println("AT+HTTPPARA=\"URL\",\"http://yuktix-api.cloudapp.net:8080/sensordb/v1/echo/bobo\"");
//Change the URL over here. i used open source HTTP request URL and send blank data to test.
   delay(2000);
   toSerial();

   // set http action type 0 = GET, 1 = POST, 2 = HEAD
   GprsSerial.println("AT+HTTPACTION=0");
   delay(6000);
   toSerial();

   // read server response
   GprsSerial.println("AT+HTTPREAD");
   delay(1000);
   toSerial();

   GprsSerial.println("");
   GprsSerial.println("AT+HTTPTERM");
   toSerial();
   delay(300);

   GprsSerial.println("");
   delay(10000);
}

void toSerial()
{
  while(GprsSerial.available()!=0)
  {
    Serial.write(GprsSerial.read());
  }
}
