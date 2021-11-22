/*
 * File:   newmain.c
 * Author: Zaw Myat
 *
 * Created on December 28, 2020, 8:24 PM
 */
#define _XTAL_FREQ    8000000

#pragma config FOSC = EXTRC_CLKOUT// Oscillator Selection bits (RC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#define i2c_freq  100

int year = 20;   //2020
int month = 12;  //December
int date = 28;   //28
int hour = 10;   
int min = 00;
int sec = 01;

void I2C_Init(const unsigned long freq) {
  TRISC3 = 1;  TRISC4 = 1; 
  
  SSPCON  = 0b00101000;
  SSPCON2 = 0b00000000;
  
  SSPADD = (_XTAL_FREQ/(4*freq*100))-1;
  SSPSTAT = 0b00000000; 
}

void I2C_Hold(){
    while ((SSPCON2 & 0b00011111)||(SSPSTAT & 0b00000100)) ;
}

void I2C_Begin(){
  I2C_Hold();  
  SEN = 1;    
}


void I2C_End(){
  I2C_Hold(); 
  PEN = 1;    
}

void I2C_Write(unsigned data){
  I2C_Hold();
  SSPBUF = data; 
}

unsigned short I2C_Read(unsigned short ack){
  unsigned short incoming;
  I2C_Hold();
  RCEN = 1;
  
  I2C_Hold();
  incoming = SSPBUF; 
  
  I2C_Hold();
  ACKDT = (ack)?0:1; 
  ACKEN = 1; 
  
  return incoming;
}

void Lcd_Port(char a){
	if(a & 1)
		D4 = 1;
	else
		D4 = 0;

	if(a & 2)
		D5 = 1;
	else
		D5 = 0;

	if(a & 4)
		D6 = 1;
	else
		D6 = 0;

	if(a & 8)
		D7 = 1;
	else
		D7 = 0;
}
void Lcd_Cmd(char a){
	RS = 0;             
	Lcd_Port(a);
	EN  = 1;             
        __delay_ms(4);
        EN  = 0;            
}

Lcd_Clear(){
	Lcd_Cmd(0);
	Lcd_Cmd(1);
}

void Lcd_Set_Cursor(char a, char b){
	char temp,z,y;
	if(a == 1){
	  temp = 0x80 + b - 1;
		z = temp>>4;
		y = temp & 0x0F;
		Lcd_Cmd(z);
		Lcd_Cmd(y);
	}
	else if(a == 2){
		temp = 0xC0 + b - 1;
		z = temp>>4;
		y = temp & 0x0F;
		Lcd_Cmd(z);
		Lcd_Cmd(y);
	}
}

void Lcd_Init(){
  Lcd_Port(0x00);
   __delay_ms(20);
  Lcd_Cmd(0x03);
	__delay_ms(5);
  Lcd_Cmd(0x03);
	__delay_ms(11);
  Lcd_Cmd(0x03);

  Lcd_Cmd(0x02);
  Lcd_Cmd(0x02);
  Lcd_Cmd(0x08);
  Lcd_Cmd(0x00);
  Lcd_Cmd(0x0C);
  Lcd_Cmd(0x00);
  Lcd_Cmd(0x06);
}

void Lcd_Write_Char(char a){
   char temp,y;
   temp = a&0x0F;
   y = a&0xF0;
   RS = 1;             
   Lcd_Port(y>>4);           
   EN = 1;
   __delay_us(40);
   EN = 0;
   Lcd_Port(temp);
   EN = 1;
   __delay_us(40);
   EN = 0;
}

void Lcd_Write_String(char *a){
	int i;
	for(i=0;a[i]!='\0';i++)
	   Lcd_Write_Char(a[i]);
}

int  b2d(int to_convert){
   return (to_convert >> 4) * 10 + (to_convert & 0x0F); 
}

int d2b (int to_convert){
   return ((to_convert / 10) << 4) + (to_convert % 10);
}

void setTime(){
   I2C_Begin();       
   I2C_Write(0xD0); 
   I2C_Write(0);  
   I2C_Write(d2b(sec)); 
   I2C_Write(d2b(min)); 
   I2C_Write(d2b(hour)); 
   I2C_Write(1);
   I2C_Write(d2b(date));
   I2C_Write(d2b(month));
   I2C_Write(d2b(year));
   I2C_End();
}

void updateTime(){
    I2C_Begin();       
    I2C_Write(0xD0); 
    I2C_Write(0);    
    I2C_End(); 

    I2C_Begin();
    I2C_Write(0xD1);     
    sec = b2d(I2C_Read(1));    
    min = b2d(I2C_Read(1)); 
    hour = b2d(I2C_Read(1));  
    I2C_Read(1);
    date = b2d(I2C_Read(1));  
    month = b2d(I2C_Read(1));  
    year = b2d(I2C_Read(1));  
    I2C_End(); 
     
    I2C_Begin();
    I2C_Write(0xD1);                   
    I2C_Read(1);    
    I2C_End(); 
}


void main(void) {
    
    TRISD = 0x00;
    Lcd_Init();
    I2C_Init(i2c_freq); 
    setTime();
    
    while(1) {
        
        updateTime(); 
        char sec0 = sec%10;
        char sec1 = (sec/10);
        char min0 = min%10;
        char min1 = min/10;
        char hour0 = hour%10;
        char hour1 = hour/10;
        char date0 = date%10;
        char date1 = date/10;
        char month0 = month%10;
        char month1 = month/10;
        char year0 = year%10;
        char year1 = year/10;
     
        Lcd_Clear();
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("Time: ");
        Lcd_Write_Char(hour1+'0');
        Lcd_Write_Char(hour0+'0');
        Lcd_Write_Char(':');
        Lcd_Write_Char(min1+'0');
        Lcd_Write_Char(min0+'0');
        Lcd_Write_Char(':');
        Lcd_Write_Char(sec1+'0');
        Lcd_Write_Char(sec0+'0');
        
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String("Date: ");
        Lcd_Write_Char(date1+'0');
        Lcd_Write_Char(date0+'0');
        Lcd_Write_Char(':');
        Lcd_Write_Char(month1+'0');
        Lcd_Write_Char(month0+'0');
        Lcd_Write_Char(':');
        Lcd_Write_Char(year1+'0');
        Lcd_Write_Char(year0+'0');
        __delay_ms(500); 
    }
}

