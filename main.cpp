#include "mbed.h"
#include "wave_player.h"
#include "PinDetect.h"
#include "USBHostMSD.h"
#include "C12832.h"
#include "ReadTime.h"
#include <vector>
#include <string>

//Set up LEDs
DigitalOut led1( LED1 );
DigitalOut led2( LED2 );
DigitalOut led3( LED3 );
DigitalOut led4( LED4 );

//Setup RGB led
PwmOut r (p23); //RGB LED pins
PwmOut g (p24);
PwmOut b (p25);

using namespace std;

C12832 lcd(p5, p7, p6, p8, p11);

//Joystick controller
PinDetect pb1(p13);//joyleft
PinDetect pb2(p16);//joyright
PinDetect pb3(p12);//joyup
PinDetect pb4(p15);//joydown
PinDetect pb5(p14);//center

Timer timer; //timer

AnalogOut DACout(p18); //set up speaker
wave_player waver(&DACout); //set up wave player library
int pos = 0; // song index
int vol = 0; // volume level
 
bool playing = false; //controlling playing state
bool firstplay = false; //variable for first play
int total_time = 0;
vector<string> songnames; //array to store name of songs

void read_file_names(char *dir) // function that reads in file names from sd cards
{
    DIR *dp;
    struct dirent *dirp;
    dp = opendir(dir);
    //read all directory and then put file paths into a vector
    while((dirp = readdir(dp)) != NULL) {
        songnames.push_back(string(dirp->d_name));
    }
}

//interrupt handler for shifting farward to next song 
void pb1_next_song (void)
{ 
    int l = songnames.size();
    if (pos < (l-1)) {
        pos++;
    } else if (pos == (l-1)) {
        pos = 0;
    }
    led1 = 1;
    led2 = 0;
    led3 = 0;
    led4 = 0;
}
//interrupt handler for shifting back to privious song 
void pb2_previous_song (void)
{
    int l = songnames.size();
    if (pos > 0) {
        pos--;
    } else if (pos == 0 ) {
        pos = l-1;
    }
    led1 = 0;
    led2 = 1;
    led3 = 0;
    led4 = 0;
}

//interrupt handler for volume up
void pb3_volume_up (void){
    vol = (vol+1) % 16;
    led1 = 0;
    led2 = 0;
    led3 = 1;
    led4 = 0;
}

//interrupt handler for volume down 
void pb4_volume_down (void){
    if (vol > 1) {
        vol = (vol-1) % 16;
    }
    led1 = 0;
    led2 = 0;
    led3 = 0;
    led4 = 1;
}

//interrupt handler for play/pause
void pb5_play_pause (void)
{
    //this interrupt handler changes the play to pause mode or vice versa
    if (playing == false && firstplay == false) {
        playing = true; 
        firstplay = true;
        r = 1;
    } else if (playing == true) {
        string songname = songnames[pos];
        playing = false;
        firstplay = false;
        g = 1;
    }
}

void show_information(const void *argv) {
    while (1) {
        int t = timer.read();//playing time
        int left_time = total_time - t;//left time
        int bar_len = 21 * t / total_time;//length of bar
        lcd.cls();
        lcd.locate(0,0);

        lcd.locate(100,0);
        lcd.printf("vol: %d", 15 - vol);
        lcd.locate(0,20);
        for (int i = 0; i < bar_len; i++) {
            lcd.printf("_");
        }
        int min1 = t / 60;
        int sec1 = t % 60;
        int min2 = left_time / 60;
        int sec2 = left_time % 60;
        lcd.locate(0,18);
        lcd.printf("%02d:%02d", min1, sec1);
        lcd.locate(105,18);
        lcd.printf("%02d:%02d", min2, sec2);
        Thread::wait(500);
    }
}


int main()
{
    //test LCD display
    lcd.cls();
    lcd.locate(0,3);
    lcd.printf("MBED Music Player");

    pb1.mode(PullDown);
    pb2.mode(PullDown);
    pb3.mode(PullDown);
    pb4.mode(PullDown);
    pb5.mode(PullDown);

    wait(.01);
    // Setup Interrupt callback functions for 5-way joystick
    pb1.attach_deasserted(&pb1_next_song);
    pb2.attach_deasserted(&pb2_previous_song);
    pb3.attach_deasserted(&pb3_volume_up);
    pb4.attach_deasserted(&pb4_volume_down);
    pb5.attach_deasserted(&pb5_play_pause);
    // Start sampling 
    pb1.setSampleFrequency();
    pb2.setSampleFrequency();
    pb3.setSampleFrequency();
    pb4.setSampleFrequency();
    pb5.setSampleFrequency();

    lcd.cls();
    USBHostMSD msc("msc");
    // Check if a USB is connected
    while(!msc.connect()) {
        lcd.locate(0,0);
        lcd.printf("Insert USB");
    }    
    // Read the songs array, please note that your wav files have to be stored in the same directory
    read_file_names("/msc/music_wav");
    
    while(1) {    
        lcd.cls();
        lcd.locate(0,2);
        lcd.printf("Press joystick to play");
        //while pb3 is low, press fire button to start playing a song
        while(playing == true && firstplay == false) { 
            string songname = songnames[pos];
            string a = "/msc/music_wav/";
            string fname = a + songname; //retrieves the file name
            FILE *wave_file; 
            lcd.cls();
            total_time = (int)ReadTime(fname.c_str());
            wave_file = fopen(fname.c_str(),"r"); //opens the music file
            Thread thread(show_information);
            timer.start();
            waver.play(wave_file);
            timer.stop();   
            timer.reset();
            fclose(wave_file);
        }
        firstplay = false;
        // if device disconnected, try to connect again
        if (!msc.connected())
            break;
    }
}

