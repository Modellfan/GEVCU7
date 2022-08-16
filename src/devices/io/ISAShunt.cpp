
/*
 * ISAShunt.cpp
 *
 * Interface to ISA IVT Shunts
 *
Copyright (c) 2022 Felix Schuster

Based on library written by Jack Rickard of EVtv - http://www.evtv.me

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ISAShunt.h"

ISAShunt::ISAShunt() : Device(), CanObserver() {
    commonName = "ISA IVT Shunt";
    shortName = "ISA Shunt";
}

void ISAShunt::earlyInit()
{
    prefsHandler = new PrefHandler(ISASHUNT_ID);
}

void ISAShunt::setup() {
    tickHandler.detach(this);

    Logger::info("add device: ISA IVT Shunt (id: %X, %X)", ISASHUNT_ID, this);

    loadConfiguration();

    Device::setup(); // run the parent class version of this function

    ISAShuntConfiguration *config = (ISAShuntConfiguration *)getConfiguration();

    ConfigEntry entry;
    //        cfgName                 helpText                               variable ref        Type                   Min Max Precision Funct
    entry = {"ISA Shunt", "Set which CAN bus to connect to (0-2)", &config->canbusNum, CFG_ENTRY_VAR_TYPE::BYTE, 0, 2, 0, nullptr};
    cfgEntries.push_back(entry);

    setAttachedCANBus(config->canbusNum);

    //Relevant BMS messages are 0x300 - 0x30F
    attachedCANBus->attach(this, 0x300, 0b011111110000, false);

    tickHandler.attach(this, CFG_TICK_INTERVAL_ISA);
    crashHandler.addBreadcrumb(ENCODE_BREAD("ISA") + 0);
}


void ISAShunt::initialize()
{
    CAN_message_t output;

  
  	firstframe=false;
	STOP();
	delay(700); //!!!!

	for(int i=0;i<9;i++)
    {
        output.len = 8;
        output.id = 0x411;
        output.flags.extended = 0; //standard frame
        output.flags.remote = 1; //No request
        output.buf[0]=(0x20+i);
        output.buf[1]=0x42;  
        output.buf[2]=0x02;
        output.buf[3]=(0x60+(i*18));
        output.buf[4]=0x00;
        output.buf[5]=0x00;
        output.buf[6]=0x00;
        output.buf[7]=0x00;
        attachedCANBus->sendFrame(output);
     
	   delay(500); //!!!!
       sendSTORE();
       delay(500);//!!!!
    }
    //  delay(500);
      START();
      delay(500);//!!!!
      lastAs=As;
      lastWh=wh;                     
}

void ISA::STOP() //Send Stop
{
    output.len = 8;
    output.id = 0x411;
    output.flags.extended = 0; //standard frame
    output.flags.remote = 1; //No request
    output.buf[0]=0x34;
    output.buf[1]=0x00;  
    output.buf[2]=0x01;
    output.buf[3]=0x00;
    output.buf[4]=0x00;
    output.buf[5]=0x00;
    output.buf[6]=0x00;
    output.buf[7]=0x00;
    attachedCANBus->sendFrame(output);
    
    Logger::debug(ISASHUNT_ID, "Stop sent");
} 

void ISA::sendSTORE() //Send Store
{
    output.len = 8;
    output.id = 0x411;
    output.flags.extended = 0; //standard frame
    output.flags.remote = 1; //No request
    output.buf[0]=0x32;
    output.buf[1]=0x00;  
    output.buf[2]=0x00;
    output.buf[3]=0x00;
    output.buf[4]=0x00;
    output.buf[5]=0x00;
    output.buf[6]=0x00;
    output.buf[7]=0x00;
    attachedCANBus->sendFrame(output);
    
    Logger::debug(ISASHUNT_ID, "Store sent");        
}   

void ISA::START() //Send Start
{
    output.len = 8;
    output.id = 0x411;
    output.flags.extended = 0; //standard frame
    output.flags.remote = 1; //No request
    output.buf[0]=0x34;
    output.buf[1]=0x01;  
    output.buf[2]=0x01;
    output.buf[3]=0x00;
    output.buf[4]=0x00;
    output.buf[5]=0x00;
    output.buf[6]=0x00;
    output.buf[7]=0x00;
    attachedCANBus->sendFrame(output);
    
    Logger::debug(ISASHUNT_ID, "Start sent");   
}

void ISA::RESTART() //Has the effect of zeroing AH and KWH
{
    output.len = 8; 
    output.id = 0x411; // Set our transmission address ID
    output.flags.extended = 0; //standard frame
    output.flags.remote = 1; //No request
    output.buf[0]=0x3F;
    output.buf[1]=0x00;  
    output.buf[2]=0x00;
    output.buf[3]=0x00;
    output.buf[4]=0x00;
    output.buf[5]=0x00;
    output.buf[6]=0x00;
    output.buf[7]=0x00;
    attachedCANBus->sendFrame(output);
    
    Logger::debug(ISASHUNT_ID, "Restart sent");   
}


void ISA::deFAULT() //Returns module to original defaults 
{
    output.len = 8; 
    output.id = 0x411; // Set our transmission address ID
    output.flags.extended = 0; //standard frame
    output.flags.remote = 1; //No request
    output.buf[0]=0x3D;
    output.buf[1]=0x00;  
    output.buf[2]=0x00;
    output.buf[3]=0x00;
    output.buf[4]=0x00;
    output.buf[5]=0x00;
    output.buf[6]=0x00;
    output.buf[7]=0x00;
    attachedCANBus->sendFrame(output);
    
    Logger::debug(ISASHUNT_ID, "Resturn to default sent"); 
}


void ISA::initCurrent()
{
	STOP();
	delay(500);
	
    output.len = 8; 
    output.id = 0x411; // Set our transmission address ID
    output.flags.extended = 0; //standard frame
    output.flags.remote = 1; //No request
    output.buf[0]=0x21;
    output.buf[1]=0x42;  
    output.buf[2]=0x01;
    output.buf[3]=0x61;
    output.buf[4]=0x00;
    output.buf[5]=0x00;
    output.buf[6]=0x00;
    output.buf[7]=0x00;
    attachedCANBus->sendFrame(output);
    
    Logger::debug(ISASHUNT_ID, "Initialize sent"); 

	delay(500);
    sendSTORE();
    delay(500);
    START();
    delay(500);
    lastAs=As;
    lastWh=wh;                      
}

void CANIODevice::loadConfiguration() {
    CanIODeviceConfiguration *config = (CanIODeviceConfiguration *) getConfiguration();

    if (!config) { // as lowest sub-class make sure we have a config object
        config = new CanIODeviceConfiguration();
        setConfiguration(config);
    }

    Device::loadConfiguration(); // call parent

    prefsHandler->read("CanbusNum", &config->canbusNum, 1);
}

/*
 * Store the current configuration to EEPROM
 */
void CANIODevice::saveConfiguration() {
    CanIODeviceConfiguration *config = (CanIODeviceConfiguration *) getConfiguration();

    Device::saveConfiguration(); // call parent

    prefsHandler->write("CanbusNum", config->canbusNum);
    prefsHandler->saveChecksum();
}

void ISAShunt::handleCanFrame(const CAN_message_t &frame) {
    int temp;
    crashHandler.addBreadcrumb(ENCODE_BREAD("ISA") + 1);
    switch (frame.id) {
		case 0x511:
			break;
		case 0x521:    
			handle521(frame);
			break;
		case 0x522:    
			handle522(frame);
			break;
		case 0x523:    
			handle523(frame);
			break;
		case 0x524:    
			handle524(frame);
			break;
		case 0x525:    
			handle525(frame);	
			break;
		case 0x526:    
			handle526(frame);	
			break;
		case 0x527:    
			handle527(frame);	
			break;
		case 0x528:
			handle528(frame);   
			break;
		default:
        	Logger::warn(ISASHUNT_ID, "Received unknown frame id %X", frame.id);
	} 
	crashHandler.addBreadcrumb(ENCODE_BREAD("ISA") + 2);
}

//Private functions below
void ISAShunt::handle521(const CAN_message_t &frame)  //AMperes
{	
	framecount++;
    Amperes = (long)((frame.buf[5] << 24) | (frame.buf[4] << 16) | (frame.buf[3] << 8) | (frame.buf[2]))/1000.0f;

	Logger::debug(ISASHUNT_ID, "Current: %fA, Framecount: %u", Amperes, framecount );   	
}

void ISAShunt::handle522(const CAN_message_t &frame)  //Voltage
{	
	framecount++;
    
    Voltage=(long)((frame.buf[5] << 24) | (frame.buf[4] << 16) | (frame.buf[3] << 8) | (frame.buf[2]))/1000.0f;
	Voltage1=Voltage-(Voltage2+Voltage3);
	if(framecount<150)
	{
		VoltageLO=Voltage;
		Voltage1LO=Voltage1;
	}
	if(Voltage<VoltageLO &&  framecount>150)VoltageLO=Voltage;
	if(Voltage>VoltageHI && framecount>150)VoltageHI=Voltage;
	if(Voltage1<Voltage1LO && framecount>150)Voltage1LO=Voltage1;
	if(Voltage1>Voltage1HI && framecount>150)Voltage1HI=Voltage1;

	Logger::debug(ISASHUNT_ID, "Voltage: %fV, Voltage1: %fV, Framecount: %u", Voltage, Voltage1, framecount ); 	
}

void ISAShunt::handle523(const CAN_message_t &frame) //Voltage2
{	
	framecount++; 
    Voltage2=(long)((frame.buf[5] << 24) | (frame.buf[4] << 16) | (frame.buf[3] << 8) | (frame.buf[2]))/1000.0f;
    if(Voltage2>3)Voltage2-=Voltage3;
    if(framecount<150)Voltage2LO=Voltage2;
    if(Voltage2<Voltage2LO  && framecount>150)Voltage2LO=Voltage2;
    if(Voltage2>Voltage2HI&& framecount>150)Voltage2HI=Voltage2;    

    Logger::debug(ISASHUNT_ID, "Voltage2: %fV, Framecount: %u", Voltage2, framecount);   	
}

void ISAShunt::handle524(const CAN_message_t &frame)  //Voltage3
{	
	framecount++;  
    Voltage3=(long)((frame.buf[5] << 24) | (frame.buf[4] << 16) | (frame.buf[3] << 8) | (frame.buf[2]))/1000.0f;
    if(framecount<150)Voltage3LO=Voltage3;
    if(Voltage3<Voltage3LO && framecount>150 && Voltage3>10)Voltage3LO=Voltage3;
    if(Voltage3>Voltage3HI && framecount>150)Voltage3HI=Voltage3;

    Logger::debug(ISASHUNT_ID, "Voltage: %fV, Voltage3: %fV, Framecount: %u", Voltage, Voltage3, framecount );
}

void ISAShunt::handle525(const CAN_message_t &frame)  //Temperature
{	
	framecount++;   
    Temperature=(long)((frame.buf[5] << 24) | (frame.buf[4] << 16) | (frame.buf[3] << 8) | (frame.buf[2]))/10.0f;
	
	Logger::debug(ISASHUNT_ID, "Temperature: %fC°, Framecount: %u", Temperature, framecount );
}



void ISAShunt::handle526(const CAN_message_t &frame) //Kilowatts
{	
	framecount++;
	watt=0;
    watt = (long)((frame.buf[5] << 24) | (frame.buf[4] << 16) | (frame.buf[3] << 8) | (frame.buf[2]));
    KW=watt/1000.0f;
		
    Logger::debug(ISASHUNT_ID, "Power: %fW, Power: %fkW, Framecount: %u", watt, KW, framecount );    
}


void ISAShunt::handle527(const CAN_message_t &frame) //Ampere-Hours
{	
	framecount++;
	As=0;
    As = ((frame.buf[5] << 24) | (frame.buf[4] << 16) | (frame.buf[3] << 8) | (frame.buf[2]));    
    AH+=(As-lastAs)/3600.0f;
    lastAs=As;
		
    Logger::debug(ISASHUNT_ID, "Current: %fAh, Framecount: %u", AH, framecount ); 
}

void ISAShunt::handle528(const CAN_message_t &frame)  //kiloWatt-hours
{	
	framecount++;
	wh = (long)((frame.buf[5] << 24) | (frame.buf[4] << 16) | (frame.buf[3] << 8) | (frame.buf[2]));
    KWH+=(wh-lastWh)/1000.0f;
	lastWh=wh;

	Logger::debug(ISASHUNT_ID, "Energy: %fWh, Framecount: %u", wh, framecount );  
}

