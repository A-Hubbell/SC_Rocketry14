//This is a tab for functions dealing with storing/exporting data from the rocket
/* DATA templates, data must be sent in this format in order for the ground application to process it properly
STATUS can be ONLINE or OFFLINE
BMP sensors  BMP:STATUS:PRESSURE:TEMPERATURE:ALTITUDE
GPS sensors  GPS:STATUS: austin put fields here
DOF sensors  DOF:STATUS: tom put your fields here

*/
include "Arduino.h"


void dataXbeeOut()
{
//  
//  //Send all 9-DOF data, 9 channels
//  for(int i = 0; i<9; i++)
//  {
//  Serial2.print(dofData[i]
//  Serial2.print("    ");
//  }
//  
//  //Each sensor (9dof/bmp180/gps) outputs all of its data on one line and then we go to the next line for the next sensor 
//  Serial2.println(" ");
//  
//  //Send all BMP180 data, 4 channels
  Serial2.print("BMP:ONLINE");
  for(int i = 0; i<4; i++)
  {
    Serial2.print(bmpData[i]);
    Serial2.print(":");
  }
//  
//  
//  Serial2.println(" ");
//  
//  //Send all GPS data, 10 channels
//  for(int i =0; i<10; i++)
//  {
//  Serial2.print(gpsData[i]);
//  Serial2.print("    ");
//  }
//  
//
//  Serial2.println(" ");
//  
}







//void dataSDOut()
//{
//  
//  //Send all 9-DOF data, 9 channels
//  for(int i = 0; i<9; i++)
//  {
//  Serial1.print(dofData[i]
//  Serial1.print("    ");
//  }
//  
//  //Each sensor (9dof/bmp180/gps) outputs all of its data on one line and then we go to the next line for the next sensor 
//  Serial1.println(" ");
//  
//  //Send all BMP180 data, 4 channels
//  for(int i = 0; i<4; i++)
//  {
//  Serial1.print(bmpData[i]);
//  Serial1.print("    ");
//  }
//  
//  
//  Serial1.println(" ");
//  
//  //Send all GPS data, 10 channels
//  for(int i =0; i<10; i++)
//  {
//  Serial1.print(gpsData[i]);
//  Serial1.print("    ");
//  }
//  
//
//  Serial1.println(" ");
//  
//}
