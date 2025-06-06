#include "mbed.h"

BufferedSerial uart0(P1_7, P1_6,38400);
I2C i2c(P0_5,P0_4);     // sda, scl

float tim,alt;
uint8_t month,day,hour,minu,sec;
uint16_t year, alt_i;
char ns,ew; //north/south, east/west
uint8_t lat_deg,lat_min,lon_deg,lon_min,lat_sec,lon_sec,pos_stat,sat;    //latitude, longitude, positioning status, satellites
uint16_t lat_sec_f,lon_sec_f;
double lat_f,lon_f,temp;
uint8_t finish_flag;
char char_read();
char char2char();
double char2flac();
void nmea_parser();

const uint8_t contrast=40;  //lcd contrast
const uint8_t lcd_addr=0x7C;   //lcd i2c addr 0x7C
const uint8_t mux_addr=0b11100000;   //mux i2c addr 0b11100000
const uint8_t lcd1_ch=0b1100;       //lcd1 mux channel
const uint8_t lcd2_ch=0b1000;       //lcd2 mux channel
void lcd_init(uint8_t addr, uint8_t contrast);     //lcd init func. default contrast=32.
void sw_lcd1(uint8_t addr);
void sw_lcd2(uint8_t addr);
char buf[17];  //i2c buffer for oled

int main(){
    i2c.frequency(400000);  //I2C clk 400kHz
    thread_sleep_for(100);  //wait for lcd power on

    //lcd1 init
    sw_lcd1(mux_addr);
    thread_sleep_for(100);
    lcd_init(lcd_addr,contrast);
    
    //lcd2 init
    sw_lcd2(mux_addr);
    thread_sleep_for(100);
    lcd_init(lcd_addr,contrast);
    
    //lcd1 display 'Boot'
    sw_lcd1(mux_addr);
    thread_sleep_for(10);
    buf[0]=0x0;
    buf[1]=0x80;   //set cusor position (0x80 means cursor set cmd)
    i2c.write(lcd_addr,buf,2);    //send cursor position
    buf[0]=0x40;   //display cmd
    buf[1]='B';
    buf[2]='o';
    buf[3]='o';
    buf[4]='t';
    i2c.write(lcd_addr,buf,5);
    
    while (true){
        nmea_parser();
        if(finish_flag==1){
            lat_deg=uint8_t(lat_f/100);
            lat_min=uint8_t(uint16_t(lat_f)-lat_deg*100);
            temp=(lat_f-lat_deg*100-lat_min)*60;    //lat. sec float type
            lat_sec=(uint8_t)temp;
            lat_sec_f=(uint16_t)((temp-lat_sec)*1000);
            lon_deg=uint8_t(lon_f/100);
            lon_min=uint8_t(uint16_t(lon_f)-lon_deg*100);
            temp=(lon_f-lon_deg*100-lon_min)*60;    //lon. sec float type
            lon_sec=(uint8_t)temp;
            lon_sec_f=(uint16_t)((temp-lon_sec)*1000);
            alt_i=(uint16_t)alt;
            hour=(uint8_t)(tim/10000);
            minu=(uint8_t)((tim-hour*10000)/100);
            sec=(uint8_t)(tim-hour*10000-minu*100);

            //lcd1 1st line
            sw_lcd1(mux_addr);
            thread_sleep_for(10);
            buf[0]=0x0;
            buf[1]=0x80;   //set cusor position (0x80 means cursor set cmd)
            i2c.write(lcd_addr,buf,2);    //send cursor position
            buf[0]=0x40;   //display cmd
            buf[1]=ns;     //north/south
            buf[2]=' ';   //space
            buf[3]=' ';   //space
            buf[4]=0x30+(lat_deg/10)%10; //latitude degree 10
            buf[5]=0x30+(lat_deg)%10; //latitude degree 1
            buf[6]=0xdf;   //degree
            buf[7]=0x30+(lat_min/10)%10;   //latitude minute 10
            buf[8]=0x30+(lat_min)%10;   //latitude minute 10
            buf[9]=0x27;   //'
            buf[10]=0x30+(lat_sec/10)%10;   //latitude sec 10
            buf[11]=0x30+(lat_sec)%10;   //latitude sec 1
            buf[12]='.';   //space
            buf[13]=0x30+(lat_sec_f/100)%10;   //latitude sec float 100
            buf[14]=0x30+(lat_sec_f/10)%10;   //latitude sec float 10
            buf[15]=0x30+(lat_sec_f)%10;   //latitude sec float 1
            buf[16]=0x22;   //"
            i2c.write(lcd_addr,buf,17);

            //lcd1 2nd line
            buf[0]=0x0;
            buf[1]=0x80+0x40;   //set cusor position (0x80 means cursor set cmd)
            i2c.write(lcd_addr,buf,2);
            buf[0]=0x40;    //display cmd
            buf[1]=ew;      //east/west
            buf[2]=' ';    //space
            buf[3]=0x30+(lon_deg/100)%10; //longitude degree 100
            buf[4]=0x30+(lon_deg/10)%10; //longitude degree 10
            buf[5]=0x30+(lon_deg)%10; //longitude degree 1
            buf[6]=0xdf;   //degree
            buf[7]=0x30+(lon_min/10)%10;   //longitude minute 10
            buf[8]=0x30+(lon_min)%10;   //longitude minute 10
            buf[9]=0x27;   //'
            buf[10]=0x30+(lon_sec/10)%10;   //longitude sec 10
            buf[11]=0x30+(lon_sec)%10;   //longitude sec 1
            buf[12]='.';   //space
            buf[13]=0x30+(lon_sec_f/100)%10;   //longitude sec float 100
            buf[14]=0x30+(lon_sec_f/10)%10;   //longitude sec float 10
            buf[15]=0x30+(lon_sec_f)%10;   //longitude sec float 1
            buf[16]=0x22;   //"
            i2c.write(lcd_addr,buf,17);

            //lcd2 1st line
            sw_lcd2(mux_addr);
            thread_sleep_for(10);
            buf[0]=0x0;
            buf[1]=0x80;   //set cusor position (0x80 means cursor set cmd)
            i2c.write(lcd_addr,buf,2);    //send cursor position
            buf[0]=0x40;   //display cmd
            buf[1]=0x30+(year/1000)%10; //year 1000
            buf[2]=0x30+(year/100)%10; //year 100
            buf[3]=0x30+(year/10)%10; //year 10
            buf[4]=0x30+(year)%10; //year 1
            buf[5]='/';   // /
            buf[6]=0x30+(month/10)%10; //month 10
            buf[7]=0x30+(month)%10; //month 1
            buf[8]='/';   // /
            buf[9]=0x30+(day/10)%10; //day 10
            buf[10]=0x30+(day)%10; //day 1
            buf[11]=' ';   //space
            buf[12]=0x30+(alt_i/1000)%10; //altitude 1000
            buf[13]=0x30+(alt_i/100)%10; //altitude 100
            buf[14]=0x30+(alt_i/10)%10; //altitude 10
            buf[15]=0x30+(alt_i)%10; //altitude 1
            buf[16]='m'; //m
            i2c.write(lcd_addr,buf,17);
            
            //lcd2 2nd line
            buf[0]=0x0;
            buf[1]=0x80+0x40;   //set cusor position (0x80 means cursor set cmd)
            i2c.write(lcd_addr,buf,2);
            buf[0]=0x40;    //display cmd
            buf[1]=0x30+(hour/10)%10; //hour 10
            buf[2]=0x30+(hour)%10; //hour 1
            buf[3]=':'; //:
            buf[4]=0x30+(minu/10)%10; //minute 10
            buf[5]=0x30+(minu)%10; //minute 1
            buf[6]=':';   //:
            buf[7]=0x30+(sec/10)%10;   //sec 10
            buf[8]=0x30+(sec)%10;   //sec 1
            buf[9]=' ';     //space
            buf[10]='S';    //S
            buf[11]=':';    //:
            buf[12]=0x30+(sat/10)%10;   //satellites 10
            buf[13]=0x30+(sat)%10;      //satellites 1
            buf[14]=' ';    //space
            if(pos_stat==1){    //3D positioning
                buf[15]='3';
                buf[16]='D';
            }else if(pos_stat==2){  //DGPS positioning
                buf[15]='D';
                buf[16]='G';
            }else if(pos_stat==4){  //RTK Fix
                buf[15]='F';
                buf[16]='X';
            }else if(pos_stat==5){  //RTK Float
                buf[15]='F';
                buf[16]='L';
            }else{                  //No positioning
                buf[15]='N';
                buf[16]='O';
            }
            i2c.write(lcd_addr,buf,17);
            
            finish_flag=0;
        }
    }
}

char char_read(){
    char local_buf[1];          //local buffer
    uart0.read(local_buf,1);    //1-char read
    return local_buf[0];        //return 1-char
}

char char2char(){
    char temp[1],local_buf[1];
    uint8_t i;
    for(i=0;i<sizeof(local_buf);++i) local_buf[i]='-';
    i=0;
    while(true){
        temp[0]=char_read();
        if(temp[0]==',') break; //',' is delimiter
        local_buf[i]=temp[0];
        ++i;
    }
    return local_buf[0];
}

double char2flac(){
    char temp[1],local_buf[10];          //local buffer
    uint8_t i;
    for(i=0;i<sizeof(local_buf);++i) local_buf[i]='\0'; //init local buf
    i=0;
    while(true){
        temp[0]=char_read();
        if(temp[0]==',') break; //',' is delimiter
        local_buf[i]=temp[0];
        ++i;
    }
    return atof(local_buf);
}

void nmea_parser(){
    char ID[5];     //NMEA ID buf
    uint8_t i;
    uint32_t temp;
    while(true) if(char_read()=='$')break; //delimiter is $
    for (i=0;i<5;++i) ID[i]=char_read(); 

    if(ID[2]=='R'){ //decode GxRMC
        char_read();    //remove ','
        char2flac();    //remove time
        char2flac();    //remove V/A
        char2flac();    //remove lat
        char2flac();    //remove ns
        char2flac();    //remove lon
        char2flac();    //remove ew
        char2flac();    //remove velocity
        char2flac();    //remove heading
        temp=char2flac();    //get ddmmyy
        day=uint8_t(temp/10000);
        month=uint8_t((temp/100)-day*100);
        year=uint16_t(temp-day*10000-month*100+2000);
    }

    if((ID[2]=='G')&(ID[3]=='G')){ //decode GxGGA
        char_read();    //remove ','
        tim=char2flac();
        lat_f=char2flac();
        ns=char2char();
        lon_f=char2flac();
        ew=char2char();
        pos_stat=uint8_t(char2flac());
        sat=uint8_t(char2flac());
        char2flac();    //remove HDOP
        alt=char2flac();
        finish_flag=1;      //If GxGGA is a last sentence.
    }
}

//lcd sw funv
void sw_lcd1(uint8_t addr){
    char buf[1];
    buf[0]=lcd1_ch;
    i2c.write(addr,buf,1);
}
void sw_lcd2(uint8_t addr){
    char buf[1];
    buf[0]=lcd2_ch;
    i2c.write(addr,buf,1);
}

//LCD init func
void lcd_init(uint8_t addr, uint8_t contrast){
    char lcd_data[2];
    lcd_data[0]=0x0;
    lcd_data[1]=0x38;
    i2c.write(addr,lcd_data,2);
    lcd_data[1]=0x39;
    i2c.write(addr,lcd_data,2);
    lcd_data[1]=0x14;
    i2c.write(addr,lcd_data,2);
    lcd_data[1]=0x70|(contrast&0b1111);
    i2c.write(addr,lcd_data,2);
    lcd_data[1]=0x56|((contrast&0b00110000)>>4);
    i2c.write(addr,lcd_data,2);
    lcd_data[1]=0x6C;
    i2c.write(addr,lcd_data,2);
    thread_sleep_for(200);
    lcd_data[1]=0x38;
    i2c.write(addr,lcd_data,2);
    lcd_data[1]=0x0C;
    i2c.write(addr,lcd_data,2);
    lcd_data[1]=0x01;
    i2c.write(addr,lcd_data,2);
}
