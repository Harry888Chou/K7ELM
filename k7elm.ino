//--------------------------------------
// K7ELM @ ESP8266
// v0.2 2018/6/14
//--------------------------------------

#include <ESP8266WiFi.h>

IPAddress local_IP(192,168,0,10);
IPAddress gateway(192,168,0,10);
IPAddress subnet(255,255,255,0);

const char* ssid = "K7ELM_192.168.0.10";
const char* pass = "12345678";

WiFiServer server(35000);						// TCP PORT 35000

char 	wfdata1[20]={0};  		// wifi data in buffer
char 	wfdata2[20]={0};  		// wifi data out buffer
char	wfdataR[20]={0};		// command repeated
byte  	dlcdata[20]={0};  		// dlc data buffer 


int dlcCommand(byte cmd, byte num, byte loc, byte len, byte data[]) 
{
  byte crc = (0xFF - (cmd + num + loc + len - 0x01)); 
  // checksum FF - (cmd + num + loc + len - 0x01)

  unsigned long timeOut = millis() + 250; // timeout @ 250 ms
  memset(data, 0, sizeof(data));
  
  while(Serial.available()) Serial.read();
  
	Serial.write(cmd);  // header/cmd read memory ??
	Serial.write(num);  // num of bytes to send
	Serial.write(loc);  // address
	Serial.write(len);  // num of bytes to read
	Serial.write(crc);  // checksum
	 
  int i = 0;
  while (i < (len+3+5) && millis() < timeOut) {
    if (Serial.available()) { 
		if (i<5) Serial.read(); else data[i-5] = Serial.read(); 
		i++; 
	}
  }
  if (data[0] != 0x00 && data[1] != (len+3)) { return 0; }	// or use checksum?
  if (i < (len+3+5)) { return 0; } 							// timeout ERROR
  return 1; 												// success
}
  
  
  
void setup()
{ 
  Serial1.begin(115200); 			// for debugging
  Serial.begin(9600);
 
  Serial1.println();

  Serial1.print("Setting soft-AP configuration ... ");
  Serial1.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
  
  Serial1.println(WiFi.softAP(ssid, pass) ? "Ready" : "Failed!");
  Serial1.print("Setting soft-AP ... ");

  Serial1.print("Soft-AP IP address = ");
  Serial1.println(WiFi.softAPIP());
 
  server.begin();
}

void loop()
{
  int i=0;		int j=0; 
  WiFiClient client = server.available();
  // wait for a client to connect
  if (client)
  {
			Serial1.println("\n[Client connected]");
			sprintf_P(wfdata2, PSTR("K7ELM v0.2\r\n>"));
			j=0;
			while (wfdata2[j] != '\0') {
				client.print(wfdata2[j]); 
				Serial1.write(wfdata2[j]); 
				j++;
			}
	while (	client.connected()) {	
    while ( client.available()) {
      wfdata1[i] = toupper(client.read());
      
      Serial1.write(wfdata1[i]); // debug
      
      if (wfdata1[i] == '\r') { // terminate at \r

		wfdata1[i] = '\0'; i=0;
        if (strlen(wfdata1) != 0)  strcpy(wfdataR,wfdata1); else strcpy(wfdata1,wfdataR);
      
		if (!strcmp(wfdata1, "ATD")) {							// set all to defaults
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}
		else if (!strcmp(wfdata1, "ATDP")) {							// display protocol 
			sprintf_P(wfdata2, PSTR("AUTO\r\n>"));
		}
		else if (!strcmp(wfdata1, "ATI")) { 					// print the ID
			sprintf_P(wfdata2, PSTR("K7ELM v0.2\r\n>"));
		}
		else if (!strcmp(wfdata1, "ATZ")) { 					// reset all 
			sprintf_P(wfdata2, PSTR("K7ELM v0.2\r\n>"));
		}
		else if (!strcmp(wfdata1, "ATPC")) { 					// protocol close 
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}
		else if (strstr(wfdata1, "AT@")) { 						// device description 
			sprintf_P(wfdata2, PSTR("Taiwan Honda Accord K7\r\n>"));
		}
		else if (strstr(wfdata1, "ATE")) { 						// echo on/off 
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}
		else if (strstr(wfdata1, "ATL")) { 						// linfeed on/off 
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}
		else if (strstr(wfdata1, "ATM")) { 						// memory on/off 
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}
		else if (strstr(wfdata1, "ATS")) { 						// space on/off
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}
		else if (strstr(wfdata1, "ATH")) { 						// headers on/off 
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}
		else if (strstr(wfdata1, "ATSP")) { 					// set protocol to ? and save it
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}
		else if (strstr(wfdata1, "ATDPN")) { 					// describe  protocol by #
			sprintf_P(wfdata2, PSTR("0\r\n>"));
		}
		else if (strstr(wfdata1, "ATST")) { 					// set Timeout 
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}	
		else if (strstr(wfdata1, "ATAT")) { 					// set Adaptive Timing AUTO1 
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}			
		else if (!strcmp(wfdata1, "ATRV")) { 					// read voltage in float / volts
			sprintf_P(wfdata2, PSTR("12.0V\r\n>"));
		}
//--------------------------------------------------------------
		else if (!strcmp(wfdata1, "04")) { 						// MODE 04:clear dtc / stored values
			dlcCommand(0x21, 0x04, 0x01, 0x00, dlcdata); 		// reset ecu
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}
		else if (!strcmp(wfdata1, "03")) { 						// MODE 03: request dtc
          // do scan then report the errors
          // 43 01 33 00 00 00 00 = P0133
          //sprintf_P(wfdata2, PSTR("SEARCHING...\r\n"));
          //sprintf_P(wfdata2, PSTR("UNABLE TO CONNECT\r\n>"));
			sprintf_P(wfdata2, PSTR("43 01 33 00 00 00 00\r\n>"));
			sprintf_P(wfdata2, PSTR("OK\r\n>"));
		}
		else if (!strcmp(wfdata1, "0100")) {
			sprintf_P(wfdata2, PSTR("41 00 BE 3E B0 11\r\n>"));
		}
		else if (!strcmp(wfdata1, "0101")) { 					// dtc / AA BB CC DD / A7 = MIL on/off, A6-A0 = DTC_CNT
				if (dlcCommand(0x20, 0x05, 0x0B, 0x01, dlcdata)) {
					byte a = ((dlcdata[2] >> 5) & 1) << 7; 		// get bit 5 on dlcdata[2], set it to a7
					sprintf_P(wfdata2, PSTR("41 01 %02X 00 00 00\r\n>"), a);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
	//else if (!strcmp(wfdata1, "0102")) { // freeze dtc / 00 61 ???
	//  if (dlcCommand(0x20, 0x05, 0x98, 0x02, dlcdata)) {
	//    sprintf_P(wfdata2, PSTR("41 02 %02X %02X\r\n>"), dlcdata[2], dlcdata[3]);
	//  }
	//  else {
	//    sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
	//  }
	//}
		else if (!strcmp(wfdata1, "0103")) { 					// fuel system status / 01 00 ???
    //if (dlcCommand(0x20, 0x05, 0x0F, 0x01, dlcdata)) { // flags
    //  byte a = dlcdata[2] & 1; // get bit 0 on dlcdata[2]
    //  a = (dlcdata[2] == 1 ? 2 : 1); // convert to comply obd2
    //  sprintf_P(wfdata2, PSTR("41 03 %02X 00\r\n>"), a);
    // }
				if (dlcCommand(0x20, 0x05, 0x9a, 0x02, dlcdata)) {
					sprintf_P(wfdata2, PSTR("41 03 %02X %02X\r\n>"), dlcdata[2], dlcdata[3]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "0104")) { 					// engine load (%)
				if (dlcCommand(0x20, 0x05, 0x9c, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("41 04 %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "0105")) { 					// ect (°C)
				if (dlcCommand(0x20, 0x05, 0x10, 0x01, dlcdata)) {
					float f = dlcdata[2];
					f = 155.04149 - f * 3.0414878 + pow(f, 2) * 0.03952185 - pow(f, 3) * 0.00029383913 + pow(f, 4) * 0.0000010792568 - pow(f, 5) * 0.0000000015618437;
					dlcdata[2] = round(f) + 40; // A-40
					sprintf_P(wfdata2, PSTR("41 05 %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "0106")) { 					// short FT (%)
				if (dlcCommand(0x20, 0x05, 0x20, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("41 06 %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "0107")) { 					// long FT (%)
				if (dlcCommand(0x20, 0x05, 0x22, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("41 07 %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "010B")) { 					// map (kPa)
				if (dlcCommand(0x20, 0x05, 0x12, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("41 0B %02X\r\n>"), dlcdata[2]);
				} 
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "010C")) { 					// rpm
				if (dlcCommand(0x20, 0x05, 0x00, 0x02, dlcdata)) {
					unsigned int srpm = (dlcdata [2] << 8 | dlcdata [3]);
					unsigned int rpm = (1875000*4) / (srpm+1);
					sprintf_P(wfdata2, PSTR("41 0C %02X %02X\r\n>"), highByte(rpm), lowByte(rpm));
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "010D")) { 					// vss (km/h)
				if (dlcCommand(0x20, 0x05, 0x02, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("41 0D %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "010E")) { 					// timing advance (°)
				if (dlcCommand(0x20, 0x05, 0x26, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("41 0E %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "010F")) { 					// iat (°C)
				if (dlcCommand(0x20, 0x05, 0x11, 0x01, dlcdata)) {
					float f = dlcdata[2];
					f = 155.04149 - f * 3.0414878 + pow(f, 2) * 0.03952185 - pow(f, 3) * 0.00029383913 + pow(f, 4) * 0.0000010792568 - pow(f, 5) * 0.0000000015618437;
					dlcdata[2] = round(f) + 40; // A-40
					sprintf_P(wfdata2, PSTR("41 0F %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "0111")) { 					// tps (%)
				if (dlcCommand(0x20, 0x05, 0x14, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("41 11 %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "0113")) { 					// o2 sensor present ???
			sprintf_P(wfdata2, PSTR("41 13 80\r\n>")); 			// 10000000 / assume bank 1 present
		}
		else if (!strcmp(wfdata1, "0114")) { 					// o2 (V)
				if (dlcCommand(0x20, 0x05, 0x15, 0x01, dlcdata)) {
					float o2 = dlcdata[2];
					dlcdata[2] = round(o2*200/51.3);
					sprintf_P(wfdata2, PSTR("41 14 %02X FF\r\n>"), dlcdata[2]);
				}
				else {
				sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "011C")) { 					// obd-I ??
			sprintf_P(wfdata2, PSTR("41 1C 04\r\n>"));
		}
		else if (!strcmp(wfdata1, "0120")) {					// PIDs 33/40
			sprintf_P(wfdata2, PSTR("41 20 00 00 20 01\r\n>")); 
		}
  //else if (!strcmp(wfdata1, "012F")) { // fuel level (%)
  //  sprintf_P(wfdata2, PSTR("41 2F FF\r\n>")); // max
  //}
		else if (!strcmp(wfdata1, "0130")) {					// ??????
			sprintf_P(wfdata2, PSTR("41 30 20 00 00 01\r\n>"));
		}
		else if (!strcmp(wfdata1, "0133")) { 					// baro (kPa)
				if (dlcCommand(0x20, 0x05, 0x13, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("41 33 %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "0140")) {
			sprintf_P(wfdata2, PSTR("41 40 48 00 00 00\r\n>"));
		}
		else if (!strcmp(wfdata1, "0142")) { 					// ecu voltage (V)
				if (dlcCommand(0x20, 0x05, 0x17, 0x01, dlcdata)) {
					float f = dlcdata[2]; f = f / 10.45;
					unsigned int u = f * 1000; 					// ((A*256)+B)/1000
					sprintf_P(wfdata2, PSTR("41 42 %02X %02X\r\n>"), highByte(u), lowByte(u));
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "0145")) { 					// iacv / relative throttle position
				if (dlcCommand(0x20, 0x05, 0x28, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("41 45 %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("DATA ERROR\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "0151")) { 					// fuel TYPE: gasoline 01
				sprintf_P(wfdata2, PSTR("41 51 01\r\n>"));
		}
		else if (!strcmp(wfdata1, "0160")) {					// PIDs 
			sprintf_P(wfdata2, PSTR("41 20 00 00 00 00\r\n>")); 
		}
//--------------------------------------------------------------
		else if (!strcmp(wfdata1, "2008")) { 					// custom hobd mapping / flags
				if (dlcCommand(0x20, 0x05, 0x08, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("60 08 %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("NO DATA\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "200B")) { 					// custom hobd mapping / flags
    //sprintf_P(wfdata2, PSTR("60 0C AA\r\n>")); // 10101010 / test data
				if (dlcCommand(0x20, 0x05, 0x0B, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("60 0B %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("NO DATA\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "200C")) { 					// custom hobd mapping / flags
				if (dlcCommand(0x20, 0x05, 0x0C, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("60 0C %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("NO DATA\r\n>"));
				}
		}
		else if (!strcmp(wfdata1, "200F")) { 					// custom hobd mapping / flags
				if (dlcCommand(0x20, 0x05, 0x0F, 0x01, dlcdata)) {
					sprintf_P(wfdata2, PSTR("60 0F %02X\r\n>"), dlcdata[2]);
				}
				else {
					sprintf_P(wfdata2, PSTR("NO DATA\r\n>"));
				}
		}
		else {
			sprintf_P(wfdata2, PSTR("NO DATA\r\n>"));
		} 
//---------------------------------------------------------------------------	  
		j=0;
		while (wfdata2[j] != '\0') {
				client.print(wfdata2[j]); 
				Serial1.write(wfdata2[j]); 
				j++;
		}
        break;
      } else if (wfdata1[i] != 32 || wfdata1[i] != 10) {  ++i; } // ignore space and newline
    }
	}
    // close the connection:
    client.stop();
  }
}
