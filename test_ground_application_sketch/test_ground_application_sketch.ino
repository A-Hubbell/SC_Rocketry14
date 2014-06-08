float dofData[9] = {0,0,0,0,0,0,0}; //9 channels (accel xyz, mag xyz, heading, 2 unused)
float gpsData[8] = {0,0,0,0,0,0,0,0}; // CHANGE to suit number of required data fields 
float bmpData[4] = {0,0,0,0};  // Altitude  Temperature  Pressure


String rocketStage = "Launch Mode";

boolean dofStatus = false; // Used to determine if sensor is online or offline
boolean gpsStatus = true;
boolean bmpStatus = true;

void setup(){
  Serial.begin(9600);
  
}
void loop (){
    for ( int i = 0; i < 8; i++)
      gpsData[i] = rand () % 100;
    for ( int i = 0; i < 8; i++ )
      dofData[i] = rand () % 100;
    for ( int i = 0; i < 4; i++ )
      bmpData[i] = rand () % 100;
      
      dataXbeeOut();
}
  
void dataXbeeOut()
{
  
  String parseString;
  char dtostrfbuffer[15];
  if ( dofStatus )
  {
    parseString = "DOF:ONLINE";
    
    for(int i = 0; i<7; i++) 
    {
      parseString += ":";
      parseString += dtostrf(dofData[i],8,2, dtostrfbuffer);
    }
  }
    else
      parseString = "DOF:OFFLINE";
  Serial.println(parseString);
  
  if ( bmpStatus )
  {
    parseString = "BMP:ONLINE";
    for(int i = 0; i<4; i++)
    {
      parseString += ":";
      parseString += dtostrf(bmpData[i],8,2, dtostrfbuffer);
    }
  }
  else
    parseString = "BMP:OFFLINE";
  Serial.println(parseString);

  if ( gpsStatus )
  {
    parseString = "GPS:ONLINE:";
    for(int i =0; i<10; i++)
    {
        parseString += ":";
        parseString += dtostrf(bmpData[i],8,2, dtostrfbuffer);
    }
  }
  else 
    parseString = "GPS:OFFLINE";
   Serial.println(parseString);
   
   Serial.println( "RocketStatus:" + rocketStage );
  
}
