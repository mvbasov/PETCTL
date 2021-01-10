#include "ASOLED.h"
#include <Wire.h>
#include <avr/pgmspace.h>

#include "a_Small_Rus.c"

// registers
#define ASA_OLED_COMMAND_MODE        0x80
#define ASA_OLED_DATA_MODE        0x40

//==========================================================

unsigned char ASA_utf8_preliminary_byte = 0;

unsigned char RecodeUTF_ASA(unsigned char c)
{
	switch (c)
	{
	case (unsigned char)0xD0:
		ASA_utf8_preliminary_byte = c; // wait second byte
		return 255;
	case (unsigned char)0xD1:
		ASA_utf8_preliminary_byte = c; // wait second byte
		return 255;
  case (unsigned char)0xD2:
    ASA_utf8_preliminary_byte = c; // wait second byte
    return 255;
  case (unsigned char)0xC2:
    ASA_utf8_preliminary_byte = c; // wait second byte
    return 255;
	default:
		if(ASA_utf8_preliminary_byte == 0) // first half of ASCII table or nonsupported symbol
		{
			if((c >= ' ') && (c <= '~')) {
				return c;
			}else{
				return '*';
			}
		}
		else if(ASA_utf8_preliminary_byte == 0xD0) //
		{
			ASA_utf8_preliminary_byte = 0;
			if((c >= 144) && (c <= 191))	{
				return c - 17;
			} else if (c == 129) { // Ё
				return 191;
      } else if (c == 131) { // Ѓ
        return 193;
      } else if (c == 132) { // Є
        return 195;
      } else if (c == 134) { // І
        return 197;
      } else if (c == 135) { // Ї
        return 199;
      } else if (c == 142) { // Ў
        return 201;
			} else {
				return '*';
			}
		}
		else if(ASA_utf8_preliminary_byte == 0xD1) //
		{
			ASA_utf8_preliminary_byte = 0;
			if((c >= 128) && (c <= 143)){
				return c + 47;
			} else if (c == 145) { // ё
				return 192;
      } else if (c == 147) { // ѓ
        return 194;
      } else if (c == 148) { // є
        return 196;
      } else if (c == 150) { // і
        return 198;
      } else if (c == 151) { // ї
        return 200;
      } else if (c == 158) { // ў
        return 202;
			} else {
				return '*';
			}
		}
    else if(ASA_utf8_preliminary_byte == 0xD2) //
    {
      ASA_utf8_preliminary_byte = 0;
      if (c == 144) { // Ґ
        return 203;
      } else if (c == 145) { // ґ
        return 204;
      } else {
        return '*';
      }
    }
    else if(ASA_utf8_preliminary_byte == 0xC2) //
    {
      ASA_utf8_preliminary_byte = 0;
      if (c == 167) { // §
        return 205;
      } else if (c == 176) { // °
        return 206;
      } else if (c == 177) { // ±
        return 207;
      } else if (c == 181) { // µ
        return 208;
      } else {
        return '*';
      }
    }
		else {
			return '*';
		}
	}
}

// ====================== LOW LEVEL =========================

void ASOLED::sendCommand(byte command){
	Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
	Wire.write(ASA_OLED_COMMAND_MODE);//data mode
	Wire.write(command);
	Wire.endTransmission();    // stop transmitting
}

void ASOLED::SetNormalOrientation() { // pins on top 
    LD.sendCommand(0xA1);
    LD.sendCommand(0xC8);
}

void ASOLED::SetTurnedOrientation() { // pins on bottom
    LD.sendCommand(0xA0);
    LD.sendCommand(0xC0);
}
  

void ASOLED::sendData(byte data){
	Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
	Wire.write(ASA_OLED_DATA_MODE);//data mode
	Wire.write(data);
	Wire.endTransmission();    // stop transmitting
}

void ASOLED::printChar(char C){ // write to temp string for numbers
  NumberString[LenString++] = C;
  NumberString[LenString] = 0;
}

void ASOLED::printString_6x8(const char *String, byte X, byte Y){
  CurrFont = Font_6x8;
 	setCursorXY(X, Y);
	while(*String){
		unsigned char c = RecodeUTF_ASA(*String++);
		if (c != 255) {                   //			printChar6(c);
      Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
      Wire.write(ASA_OLED_DATA_MODE);//data mode
      Wire.write(textMode);
      for(byte i=0; i<5; i++)
        Wire.write(pgm_read_byte(&SmallFont[(c-32)*(int)5 + i + 4])^textMode);
      CurrX += 6;
      Wire.endTransmission();    // stop transmitting
		}
	}
}

void ASOLED::printString_6x8(const __FlashStringHelper *ifsh, byte X, byte Y){
  CurrFont = Font_6x8;
   setCursorXY(X, Y);
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);
  while(pgm_read_byte(p)){
    unsigned char c = RecodeUTF_ASA(pgm_read_byte(p++));
    if (c != 255) {                   //      printChar6(c);
      Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
      Wire.write(ASA_OLED_DATA_MODE);//data mode
      Wire.write(textMode);
      for(byte i=0; i<5; i++)
        Wire.write(pgm_read_byte(&SmallFont[(c-32)*(int)5 + i + 4])^textMode);
      CurrX += 6;
      Wire.endTransmission();    // stop transmitting
    }
  }
}

unsigned int EnlardeByte2Word(char b)
{
	unsigned int d = 0;
	for (byte i = 0; i < 8; i++)
	{
		unsigned int e = (((unsigned int)b) & (1 << i)) << i;
		d = d | e | (e << 1);
	}
	return d;
}

void ASOLED::printString_12x16(const char *String, byte X, byte Y){
  CurrFont = Font_12x16;
  setCursorXY(X, Y);
  const char *String0 = String;
  unsigned int m = 0;
  byte tmpX = CurrX;
  while(*String){                               // print upper half of the string
    unsigned char c = RecodeUTF_ASA(*String++);
    if (c != 255)
    {
      Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
      Wire.write(ASA_OLED_DATA_MODE);//data mode
      if(tmpX < (ASOLED_Max_X - 1)) {
        Wire.write(textMode);
        Wire.write(textMode);
        tmpX += 2;
      }
      for(byte i=0; i<5; i++) {
        if(tmpX < (ASOLED_Max_X - 1)) {
          m = EnlardeByte2Word(pgm_read_byte(&SmallFont[(c-32)*(int)5 + i + 4]));
          Wire.write(lowByte(m)^textMode);
          Wire.write(lowByte(m)^textMode);
          tmpX += 2;
        }
      }
      Wire.endTransmission();    // stop transmitting
    } 
  }
  setCursorXY(CurrX, CurrY+1);
  while(*String0){                               // print lower half of the string
    unsigned char c = RecodeUTF_ASA(*String0++);
    if (c != 255)
    {
      Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
      Wire.write(ASA_OLED_DATA_MODE);//data mode
      if(CurrX < (ASOLED_Max_X - 1)) {
        Wire.write(textMode);
        Wire.write(textMode);
        CurrX += 2;
      }
      for(byte i=0; i<5; i++) {
        if(CurrX < (ASOLED_Max_X - 1)) {
          m = EnlardeByte2Word(pgm_read_byte(&SmallFont[(c-32)*(int)5 + i + 4]));
          Wire.write(highByte(m)^textMode);
          Wire.write(highByte(m)^textMode);
          CurrX += 2;
        }
      }
      Wire.endTransmission();    // stop transmitting
    } 
  }
  setCursorXY(CurrX, CurrY-1);
}

void ASOLED::printString_12x16(const __FlashStringHelper *ifsh, byte X, byte Y){
  CurrFont = Font_12x16;
  setCursorXY(X, Y);
  const __FlashStringHelper *ifsh0 =  ifsh;
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);
  unsigned int m = 0;
  byte tmpX = CurrX;
  while(pgm_read_byte(p)){                               // print upper half of the string
    unsigned char c = RecodeUTF_ASA(pgm_read_byte(p++));
    if (c != 255)
    {
      Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
      Wire.write(ASA_OLED_DATA_MODE);//data mode
      if(tmpX < (ASOLED_Max_X - 1)) {
        Wire.write(textMode);
        Wire.write(textMode);
        tmpX += 2;
      }
      for(byte i=0; i<5; i++) {
        if(tmpX < (ASOLED_Max_X - 1)) {
          m = EnlardeByte2Word(pgm_read_byte(&SmallFont[(c-32)*(int)5 + i + 4]));
          Wire.write(lowByte(m)^textMode);
          Wire.write(lowByte(m)^textMode);
          tmpX += 2;
        }
      }
      Wire.endTransmission();    // stop transmitting
    } 
  }
  setCursorXY(CurrX, CurrY+1);
  p = reinterpret_cast<PGM_P>(ifsh0);
  while(pgm_read_byte(p)){                               // print lower half of the string
    unsigned char c = RecodeUTF_ASA(pgm_read_byte(p++));
    if (c != 255)
    {
      Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
      Wire.write(ASA_OLED_DATA_MODE);//data mode
      if(CurrX < (ASOLED_Max_X - 1)) {
        Wire.write(textMode);
        Wire.write(textMode);
        CurrX += 2;
      }
      for(byte i=0; i<5; i++) {
        if(CurrX < (ASOLED_Max_X - 1)) {
          m = EnlardeByte2Word(pgm_read_byte(&SmallFont[(c-32)*(int)5 + i + 4]));
          Wire.write(highByte(m)^textMode);
          Wire.write(highByte(m)^textMode);
          CurrX += 2;
        }
      }
      Wire.endTransmission();    // stop transmitting
    } 
  }
  setCursorXY(CurrX, CurrY-1);
}

void ASOLED::printString(const char *String, byte X, byte Y)  // Current font
{
  if(CurrFont == Font_6x8)
    printString_6x8(String, X, Y);
  else
    printString_12x16(String, X, Y);   
}

void ASOLED::printString(const __FlashStringHelper *ifsh, byte X, byte Y)  // Current font
{
  if(CurrFont == Font_6x8)
    printString_6x8(ifsh, X, Y);
  else
    printString_12x16(ifsh, X, Y);   
}

byte ASOLED::printNumber(long long_num, byte X, byte Y){
  LenString = 0;
	setCursorXY(X, Y);
	byte char_buffer[10] = "";
	byte i = 0;
	byte f = 0; // number of characters
	if (long_num < 0) {
		f++;
		printChar('-');
		long_num = -long_num;
	} 
	else if (long_num == 0) {
		f++;
		printChar('0');
    printString(&NumberString[0]); //, X, Y);
		return f;
	} 
	while (long_num > 0) {
		char_buffer[i++] = long_num % 10;
		long_num /= 10;
	}
	f += i;
	for(; i > 0; i--) 
		printChar('0'+ char_buffer[i - 1]);
  printString(&NumberString[0]); //, X, Y);
	return f;
}

byte ASOLED::printNumber(float float_num, byte prec, byte X, byte Y){
  LenString = 0;
	setCursorXY(X, Y);
// prec - 6 maximum
	byte num_int = 0;
	byte num_frac = 0;
	byte num_extra = 0;
	long d = float_num; // get the integer part
	float f = float_num - d; // get the fractional part
	if (d == 0 && f < 0.0){
		printChar('-');
		num_extra++;
		printChar('0');
		num_extra++;
		f *= -1;
    printString(&NumberString[0]);
    LenString = 0;
	}
	else if (d < 0 && f < 0.0){
		num_int = printNumber(d);  // count how many digits in integer part
    LenString = 0;
		f *= -1;
	}
	else{
		num_int = printNumber(d);  // count how many digits in integer part
    LenString = 0;
	}
	// only when fractional part > 0, we show decimal point
	if (f > 0.0){
		printChar('.');
		num_extra++;
    printString(&NumberString[0]); //, X, Y);
    LenString = 0;
		if (num_int + prec > 8) 
			prec = 8 - num_int;
    for (byte j=0; j<prec; j++){
      f *= 10.0;
      byte dd = f;
      printChar('0' + dd);
      f -= dd;
    }
    printString(&NumberString[0]);
    num_frac = prec;
	}
	return num_int + num_frac + num_extra;
}

void ASOLED::drawBitmap(const byte *bitmaparray, byte X, byte Y, byte width, byte height){
// max width = 128
// max height = 8
  for (int j = 0; j <  height; j++) {
    setCursorXY(X, Y + lowByte(j));
    Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
    Wire.write(ASA_OLED_DATA_MODE);//data mode
    for (byte i = 0; i < width; i++){
      if((i % 16) == 15)
      {
        Wire.endTransmission();    // stop transmitting
        Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
        Wire.write(ASA_OLED_DATA_MODE);//data mode
      }
      Wire.write(pgm_read_byte(bitmaparray + i + 4 + j*width));
    }
    Wire.endTransmission();    // stop transmitting
  }
}

void ASOLED::drawBitmap(const byte *bitmaparray, byte X, byte Y){
  byte width  = pgm_read_byte(&bitmaparray[0]);
  byte height = pgm_read_byte(&bitmaparray[1])/8;
  for (int j = 0; j <  height; j++) {
    setCursorXY(X, Y + lowByte(j));
    Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
    Wire.write(ASA_OLED_DATA_MODE);//data mode
    for (byte i = 0; i < width; i++)    {
      if((i % 16) == 15)
      {
        Wire.endTransmission();    // stop transmitting
        Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
        Wire.write(ASA_OLED_DATA_MODE);//data mode
      }
      Wire.write(pgm_read_byte(bitmaparray + i + 4 + j*width));
    }
    Wire.endTransmission();    // stop transmitting
  }
}

void ASOLED::VertBar(int Num, int Val, int MinVal, int MaxVal)
{
	if (Val < MinVal) Val = MinVal;            // consider that Val already scaled
	if (Val > MaxVal) Val = MaxVal;
	Val -= MinVal;
    for (int i = 0; i < 8; i++)              // go from high levels to low ones
	{
		setCursorXY(Num, i);
    int b;                                   // will calc this value for each columns (bars)
		int UpB = (MaxVal - MinVal)*(8-i)/8;
    if (Val <= UpB)                          // top boundary
		{
			int DownB = (MaxVal - MinVal)*(7-i)/8;
      if (Val >= DownB)                      // bottom boundary
      {                                      // Val are in this interval
				int j = (Val - DownB)*8/(UpB - DownB);
				int k = 0xFF00;
				b = (k >> j) & 0xFF;
			}
			else
			{
				b = 0;
			}
		}
		else
		{
			b = 0xFF;
		}
		sendData(b);
	}
  setCursorXY(0, 0);
}

// =================== High Level ===========================

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// commands
#define ASA_OLED_CMD_DISPLAY_OFF      0xAE
#define ASA_OLED_CMD_DISPLAY_ON     0xAF
#define ASA_OLED_CMD_NORMAL_DISPLAY   0xA6
#define ASA_OLED_CMD_INVERSE_DISPLAY    0xA7
#define ASA_OLED_CMD_SET_BRIGHTNESS   0x81

#define ASA_OLED_RIGHT_SCROLL       0x26
#define ASA_OLED_LEFT_SCROLL        0x27
#define ASA_OLED_SET_VERTICAL_SCROLL_AREA 0xA3
#define ASA_OLED_VERTICAL_RIGHT_SCROLL  0x29
#define ASA_OLED_VERTICAL_LEFT_SCROLL   0x2A
#define ASA_OLED_CMD_ACTIVATE_SCROLL    0x2F
#define ASA_OLED_CMD_DEACTIVATE_SCROLL  0x2E

#define HORIZONTAL_ADDRESSING 0x00
#define PAGE_ADDRESSING     0x02

#define Scroll_Left       0x00
#define Scroll_Right      0x01
#define Scroll_Up       0x02
#define Scroll_Down       0x03

#define Scroll_2Frames      0x07
#define Scroll_3Frames      0x04
#define Scroll_4Frames      0x05
#define Scroll_5Frames      0x00
#define Scroll_25Frames     0x06
#define Scroll_64Frames     0x01
#define Scroll_128Frames    0x02
#define Scroll_256Frames    0x03

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SSD1306 Commandset
// ------------------
// Fundamental Commands
#define ASA_DISPLAY_ALL_ON_RESUME	    	0xA4
// Addressing Setting Commands
#define ASA_MEMORY_ADDR_MODE	    		0x20
// Hardware Configuration Commands
#define ASA_SET_START_LINE    			0x40
#define ASA_SET_SEGMENT_REMAP	    		0xA0
#define ASA_SET_MULTIPLEX_RATIO		    	0xA8
#define ASA_COM_SCAN_DIR_DEC			0xC8
#define ASA_SET_DISPLAY_OFFSET    			0xD3
#define ASA_SET_COM_PINS	    			0xDA
#define ASA_CHARGE_PUMP   			0x8D
// Timing & Driving Scheme Setting Commands
#define ASA_SET_DISPLAY_CLOCK_DIV_RATIO		0xD5
#define ASA_SET_PRECHARGE_PERIOD		0xD9
#define ASA_SET_VCOM_DESELECT		    	0xDB

void ASOLED::init(){
	Wire.begin();
	// upgrade to 400KHz! (only use when your other i2c device support this speed)
#ifndef __SAM3X8E__
	if (I2C_400KHZ){
		// save I2C bitrate (default 100Khz)
		byte twbrbackup = TWBR;
		TWBR = 12; 
	}
#else
	Wire.setClock(400000); 
#endif
	setPageMode();	// default addressing mode
	clearDisplay();
	setCursorXY(0,0);
	
// Additional command
	LD.setPowerOff();
	LD.sendCommand(ASA_SET_DISPLAY_CLOCK_DIV_RATIO);  // D5
	LD.sendCommand(0x80);                             // 80
	LD.sendCommand(ASA_SET_MULTIPLEX_RATIO);          // A8
	LD.sendCommand(0x3F);                             // 3F
	LD.sendCommand(ASA_SET_DISPLAY_OFFSET);           // D3
	LD.sendCommand(0x0);                              //  0
	LD.sendCommand(ASA_SET_START_LINE | 0x0);         // 40
	LD.sendCommand(ASA_CHARGE_PUMP);                  // 8D
		LD.sendCommand(0x14);                        // 14
	LD.sendCommand(ASA_MEMORY_ADDR_MODE);             // 20
	LD.sendCommand(0x00);                             //  0
	LD.sendCommand(ASA_SET_SEGMENT_REMAP | 0x1);      // A1 (A0+1)
	LD.sendCommand(ASA_COM_SCAN_DIR_DEC);             // C8
	LD.sendCommand(ASA_SET_COM_PINS);                 // DA
	LD.sendCommand(0x12);                             // 12
	LD.setBrightness(0xCF);                           // CF
	LD.sendCommand(ASA_SET_PRECHARGE_PERIOD);         // D9
		LD.sendCommand(0xF1);                        // F1
	LD.sendCommand(ASA_SET_VCOM_DESELECT);            // DB
	LD.sendCommand(0x40);                             // 40
	LD.sendCommand(ASA_DISPLAY_ALL_ON_RESUME);        // A4
	LD.setNormalDisplay();
	LD.setPowerOn();
}

void ASOLED::setCursorXY(byte X, byte Y){
// Y - 1 unit = 1 page (8 pixel rows)
// X - 1 unit = 1 pixel columns
  if (X < 128)
    if ((X != CurrX) || (Y != CurrY)){
#ifdef _SH1106_
      sendCommand(0x00 + ((X+2) & 0x0F)); 		//set column lower address
      sendCommand(0x10 + (((X+2)>>4)&0x0F)); 	//set column higher address
#else
      sendCommand(0x00 + (X & 0x0F)); 		//set column lower address
      sendCommand(0x10 + ((X>>4)&0x0F)); 	//set column higher address
#endif
      sendCommand(0xB0 + Y); 					//set page address
      CurrX = X;
      CurrY = Y;
    }
}

void ASOLED::setFont(const char font)
{
  CurrFont = font;
}

void ASOLED::clearDisplay()	{
//  Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
//  Wire.write(ASA_OLED_DATA_MODE);//data mode
//  Wire.write(data);
//  Wire.endTransmission();    // stop transmitting
//	for(byte page=0; page<8; page++) {	
//		setCursorXY(0, page);     
//		for(byte column=0; column<128; column++)  //clear all columns
//			sendData(0);    
//	}
//	setCursorXY(0,0);
////    setCursorXY(0,0);
////    for(byte j = 0; j < 64; j++) {
////      Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
////      Wire.write(ASA_OLED_DATA_MODE);//data mode
////      for(byte i = 0; i < 16; i++)
////        Wire.write(0);
////      Wire.endTransmission();    // stop transmitting
////    }
////    setCursorXY(0,0);
//  byte width  = 128;
//  byte height = 8;
  for (byte j = 0; j <  8; j++) {
    setCursorXY(0, j);
    Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
    Wire.write(ASA_OLED_DATA_MODE);//data mode
    for (byte i = 0; i < 128; i++)    {
      if((i % 16) == 15)
      {
        Wire.endTransmission();    // stop transmitting
        Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
        Wire.write(ASA_OLED_DATA_MODE);//data mode
      }
      Wire.write(0);
    }
    Wire.endTransmission();    // stop transmitting
  }
}

/*=========================================
void ASOLED::drawBitmap(const byte *bitmaparray, byte X, byte Y){
  byte width  = pgm_read_byte(&bitmaparray[0]);
  byte height = pgm_read_byte(&bitmaparray[1])/8;
  for (int j = 0; j <  height; j++) {
    setCursorXY(X, Y + lowByte(j));
    Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
    Wire.write(ASA_OLED_DATA_MODE);//data mode
    for (byte i = 0; i < width; i++)    {
      if((i % 16) == 15)
      {
        Wire.endTransmission();    // stop transmitting
        Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
        Wire.write(ASA_OLED_DATA_MODE);//data mode
      }
      Wire.write(pgm_read_byte(bitmaparray + i + 4 + j*width));
    }
    Wire.endTransmission();    // stop transmitting
  }
}

=========================================*/

void ASOLED::setInverseDisplay(){
	sendCommand(ASA_OLED_CMD_INVERSE_DISPLAY);
}

void ASOLED::setNormalDisplay(){
	sendCommand(ASA_OLED_CMD_NORMAL_DISPLAY);
}

void ASOLED::SetInverseText() // next string will draw inverse
{
  textMode = 0xff;  
}

void ASOLED::SetNormalText() // next string will draw normal
{
  textMode = 0;  
}

void ASOLED::setPowerOff(){
	sendCommand(ASA_OLED_CMD_DISPLAY_OFF);
}

void ASOLED::setPowerOn(){
	sendCommand(ASA_OLED_CMD_DISPLAY_ON);
}

void ASOLED::setBrightness(byte Brightness){
	sendCommand(ASA_OLED_CMD_SET_BRIGHTNESS);
	sendCommand(Brightness);
}

void ASOLED::setPageMode(){
	addressingMode = PAGE_ADDRESSING;
	sendCommand(0x20); 				//set addressing mode
	sendCommand(PAGE_ADDRESSING); 	//set page addressing mode
}

void ASOLED::setHorizontalMode(){
	addressingMode = HORIZONTAL_ADDRESSING;
	sendCommand(0x20); 				//set addressing mode
	sendCommand(HORIZONTAL_ADDRESSING); 	//set page addressing mode
}

ASOLED LD;  // Preinstantiate Objects


