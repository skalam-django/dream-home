#include "pinMapping.h"

pinMapping::pinMapping(){

}

pinMapping::~pinMapping() {
  
}

uint16_t* pinMapping::NodeMCU_D1_Mini_Dual_Pins(String port){
	uint16_t pin0;
	uint16_t pin1;
	
	if(port == "D0"){ 
		pin0 = 16;
		pin1 = 17;
	}
	else if(port == "D2"){
		pin0 = 4;
		pin1 = 5;
	}
	else if(port == "D4"){
		pin0 = 2;
		pin1 = 0;
	}
	else if(port == "D6"){
		pin0 = 12;
		pin1 = 14;
	}
	else if(port == "D8"){
		pin0 = 15;	
		pin1 = 13;	
	}
	else {
		pin0 = 255;		
		pin1 = 255;													
	}
	uint16_t* pins = (uint16_t*)malloc(2*sizeof(uint16_t));
	pins[0] = pin0;
	pins[1] = pin1;
	return pins;
}

uint16_t pinMapping::NodeMCU_D1_Mini_Single_Pin(String port){

	if(port == "D0"){ 
		return 16;
	}
	else if(port == "D1"){
		return 5;
	}
	else if(port == "D2"){
		return 4;
	}
	else if(port == "D3"){
		return 0;
	}
	else if(port == "D4"){
		return 2;
	}
	else if(port == "D5"){
		return 14;
	}
	else if(port == "D6"){
		return 12;
	}
	else if(port == "D7"){
		return 13;
	}
	else if(port == "D8"){
		return 15;	
	}
	else if(port == "A0"){
		return 17;
	}
	else {
		return 255;															
	}
}


// WeMos D1 mini Pin Number  	Arduino IDE Pin Number
// D0 	16
// D1 	5
// D2 	4
// D3 	0
// D4 	2
// D5 	14
// D6 	12
// D7 	13
// D8 	15
// TX 	1
// RX 	3