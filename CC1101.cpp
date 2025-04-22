/*
 * CC1101.cpp
 *
 *  Created on: Mar 30, 2025
 *      Author: admin
 */
#include <CC1101.hpp>

CC1101::CC1101(SPI_HandleTypeDef *hspi, GPIO_TypeDef* CS_PORT, uint16_t CS_PIN){

	this->hspi = hspi;
	this->CS_PORT = CS_PORT;
	this->CS_PIN = CS_PIN;
	this->SRES();
}
void CC1101::sendCommand(uint8_t *commandBuf, uint8_t commandLen, uint8_t *responseBuf, uint8_t responseLen){
	if(responseLen > 0){
		HAL_GPIO_WritePin(this->CS_PORT, this->CS_PIN, GPIO_PIN_RESET);
		HAL_SPI_Transmit(this->hspi, commandBuf, (uint16_t)commandLen, HAL_MAX_DELAY);
		HAL_SPI_Receive(this->hspi, responseBuf, (uint16_t)responseLen, HAL_MAX_DELAY);
		HAL_GPIO_WritePin(this->CS_PORT, this->CS_PIN, GPIO_PIN_SET);
	}else{
		HAL_GPIO_WritePin(this->CS_PORT, this->CS_PIN, GPIO_PIN_RESET);
		HAL_SPI_Transmit(this->hspi, commandBuf, (uint16_t)commandLen, HAL_MAX_DELAY);
		HAL_GPIO_WritePin(this->CS_PORT, this->CS_PIN, GPIO_PIN_SET);
	}
}
CC1101_CONNECTION_STATE CC1101::isConnected(){
	this->SIDLE();
	uint8_t command = 0xF1;
	uint8_t response;
	this->sendCommand(&command, 1, &response, 1);
	if(response == 0x14){
		return CC1101_STATUS_CONNECTED;
	}
	if(response == 0x00){
		return CC1101_STATUS_DISCONNECTED;
	}
	return CC1101_STATUS_ERROR;
}

void CC1101::clearFIFORX(){
	this->SFRX();
}
void CC1101::clearFIFOTX(){
	this->SFTX();
}

CC1101_FIFO_UNDERFLOW_STATE CC1101::isFIFORXUnderflow(){
	if(this->lenFIFORX() & (1 << 7)){
		return(CC1101_FIFO_UNDERFLOW);
	}else{
		return(CC1101_FIFO_OK);
	}
}

CC1101_FIFO_UNDERFLOW_STATE CC1101::isFIFOTXUnderflow(){
	if(this->lenFIFOTX() & (1 << 7)){
		return(CC1101_FIFO_UNDERFLOW);
	}else{
		return(CC1101_FIFO_OK);
	}
}

void CC1101::defaultConfiguration(){
	this->SIDLE();
	uint8_t defaultConfiguration[] = {0x40, 0x29, 0x2E, 0x06, 0x47, 0xD3, 0x91, 0xFF, 0x0E, 0x05, 0x00, 0x00, 0x06, 0x00, 0x21, 0x62, 0x76, 0xF5, 0x83, 0x13, 0x22, 0xF8, 0x15, 0x07, 0x30, 0x18, 0x16, 0x6C, 0x03, 0x40, 0x91, 0x87, 0x6B, 0xFB, 0x56, 0x10, 0xE9, 0x2A, 0x00, 0x1F, 0x41, 0x00, 0x59, 0x7F, 0x3F, 0x81, 0x35, 0x09};
	this->sendCommand(defaultConfiguration, sizeof(defaultConfiguration), NULL, 0);
	uint8_t PATABLE[9];
	for(uint8_t i = 0; i < 9; i++){
		PATABLE[i] = 0;
	}
	PATABLE[0] = 0x7E;
	this->sendCommand(PATABLE, 9, NULL, 0);
	this->setFrequency(this->frequency);
	this->setPower(this->power);
}

void CC1101::writeFIFO(uint8_t *data, uint8_t len){
	this->SIDLE();
	uint8_t *command = new uint8_t[len+1];
	command[0] = 0x7F;
	for(int i = 1; i < len+1; i++){
		command[i] = data[i-1];
	}
	this->sendCommand(command, len+1, NULL, 0);
	delete[] command;
}
void CC1101::transmit(uint8_t *data, uint8_t len){
	uint8_t *FIFO = new uint8_t[len+1];
	FIFO[0] = len;
	for(int i = 1; i < len+1; i++){
		FIFO[i] = data[i-1];
	}
	this->writeFIFO(FIFO, len+1);
	delete[] FIFO;
	this->SIDLE();
	this->STX();
}

void CC1101::setSyncWord(uint8_t msb, uint8_t lsb){
	uint8_t command[2];
	command[0] = 0x04;
	command[1] = msb;
	this->sendCommand(command, 2, NULL, 0);
	command[0] = 0x05;
	command[1] = lsb;
	this->sendCommand(command, 2, NULL, 0);
}

void CC1101::setAutoFlush(uint8_t value){
	uint8_t command[2];
	command[0] = 0x87;
	uint8_t response;
	this->sendCommand(&(command[0]), 1, &response, 1);
	if(value == 0){
		response &= ~(1 << 3);
	}else{
		response |= (1 << 3);
	}
	command[0] = 0x07;
	command[1] = response;
	this->sendCommand(command, 2, NULL, 0);
}

void CC1101::setAddressCheck(CC1101_ADDRESS_CHECK_STATE value){
	uint8_t command[2];
	command[0] = 0x87;
	uint8_t response;
	this->sendCommand(&(command[0]), 1, &response, 1);
	switch(value){
		case CC1101_NO_ADDRESS_CHECK:{
			response &= ~(1 << 0);
			response &= ~(1 << 1);
			break;
		}
		case CC1101_ADDRESS_CHECK_NO_BROADCAST:{
			response |= (1 << 0);
			response &= ~(1 << 1);
			break;
		}
		case CC1101_ADDRESS_CHECK_AND_BROADCAST:{
			response &= ~(1 << 0);
			response |= (1 << 1);
			break;
		}
		case CC1101_ADDRESS_CHECK_AND_BROADCAST_AND_FF:{
			response |= (1 << 0);
			response |= (1 << 1);
			break;
		}
	}
	command[0] = 0x07;
	command[1] = response;
	this->sendCommand(command, 2, NULL, 0);
}

void CC1101::setModulationFormat(CC1101_MODULATION_FORMAT_STATE value){
	uint8_t command[2];
	command[0] = 0x92;
	uint8_t response;
	this->sendCommand(&(command[0]), 1, &response, 1);
	switch(value){
		case CC1101_2_FSK:{
			response &= ~(1 << 4);
			response &= ~(1 << 5);
			response &= ~(1 << 6);
			break;
		}
		case CC1101_GFSK:{
			response |= (1 << 4);
			response &= ~(1 << 5);
			response &= ~(1 << 6);
			break;
		}
		case CC1101_4_FSK:{
			response &= ~(1 << 4);
			response &= ~(1 << 5);
			response |= (1 << 6);
			break;
		}
	}
	command[0] = 0x12;
	command[1] = response;
	this->sendCommand(command, 2, NULL, 0);
}


void CC1101::setSyncMode(CC1101_SYNC_MODE_STATE value){
	uint8_t command[2];
	command[0] = 0x92;
	uint8_t response;
	this->sendCommand(&(command[0]), 1, &response, 1);
	switch(value){
		case CC1101_NO_PREAMBLE_SYNC:{
			response &= ~(1 << 0);
			response &= ~(1 << 1);
			response &= ~(1 << 2);
			break;
		}
		case CC1101_15_16_SYNC:{
			response |= (1 << 0);
			response &= ~(1 << 1);
			response &= ~(1 << 2);
			break;
		}
		case CC1101_16_16_SYNC:{
			response &= ~(1 << 0);
			response |= (1 << 1);
			response &= ~(1 << 2);
			break;
		}
		case CC1101_30_32_SYNC:{
			response |= (1 << 0);
			response |= (1 << 1);
			response &= ~(1 << 2);
			break;
		}
		case CC1101_NO_PREAMBLE_SYNC_CARRIER:{
			response &= ~(1 << 0);
			response &= ~(1 << 1);
			response |= (1 << 2);
			break;
		}
		case CC1101_15_16_CARRIER:{
			response |= (1 << 0);
			response &= ~(1 << 1);
			response |= (1 << 2);
			break;
		}
		case CC1101_16_16_CARRIER:{
			response &= ~(1 << 0);
			response |= (1 << 1);
			response |= (1 << 2);
			break;
		}
		case CC1101_30_32_CARRIER:{
			response |= (1 << 0);
			response |= (1 << 1);
			response |= (1 << 2);
			break;
		}
	}
	command[0] = 0x12;
	command[1] = response;
	this->sendCommand(command, 2, NULL, 0);
}

void CC1101::setManchesterEncoding(uint8_t value){
	uint8_t command[2];
	command[0] = 0x92;
	uint8_t response;
	this->sendCommand(&(command[0]), 1, &response, 1);
	if(value == 0){
		response &= ~(1 << 3);
	}else{
		response |= (1 << 3);
	}
	command[0] = 0x12;
	command[1] = response;
	this->sendCommand(command, 2, NULL, 0);
}

void CC1101::setNumberPreamble(uint8_t value){
	uint8_t command[2];
	command[0] = 0x93;
	uint8_t response;
	this->sendCommand(&(command[0]), 1, &response, 1);
	switch(value){
		case 2:{
			response &= ~(1 << 4);
			response &= ~(1 << 5);
			response &= ~(1 << 6);
			break;
		}
		case 3:{
			response |= (1 << 4);
			response &= ~(1 << 5);
			response &= ~(1 << 6);
			break;
		}
		case 4:{
			response &= ~(1 << 4);
			response |= (1 << 5);
			response &= ~(1 << 6);
			break;
		}
		case 6:{
			response |= (1 << 4);
			response |= (1 << 5);
			response &= ~(1 << 6);
			break;
		}
		case 8:{
			response &= ~(1 << 4);
			response &= ~(1 << 5);
			response |= (1 << 6);
			break;
		}
		case 12:{
			response |= (1 << 4);
			response &= ~(1 << 5);
			response |= (1 << 6);
			break;
		}
		case 16:{
			response &= ~(1 << 4);
			response |= (1 << 5);
			response |= (1 << 6);
			break;
		}
		case 24:{
			response |= (1 << 4);
			response |= (1 << 5);
			response |= (1 << 6);
			break;
		}
	}
	command[0] = 0x13;
	command[1] = response;
	this->sendCommand(command, 2, NULL, 0);
}

void CC1101::setRX_Off_Mode(CC1101_OFF_MODE_STATE value){
	uint8_t command[2];
	command[0] = 0x97;
	uint8_t response;
	this->sendCommand(&(command[0]), 1, &response, 1);
	switch(value){
		case CC1101_OFF_MODE_IDLE:{
			response &= ~(1 << 2);
			response &= ~(1 << 3);
			break;
		}
		case CC1101_OFF_MODE_FSTXON:{
			response |= (1 << 2);
			response &= ~(1 << 3);
			break;
		}
		case CC1101_OFF_MODE_TX:{
			response &= ~(1 << 2);
			response |= (1 << 3);
			break;
		}
		case CC1101_OFF_MODE_RX:{
			response |= (1 << 2);
			response |= (1 << 3);
			break;
		}
	}
	command[0] = 0x17;
	command[1] = response;
	this->sendCommand(command, 2, NULL, 0);
}

void CC1101::setTX_Off_Mode(CC1101_OFF_MODE_STATE value){
	uint8_t command[2];
	command[0] = 0x97;
	uint8_t response;
	this->sendCommand(&(command[0]), 1, &response, 1);
	switch(value){
		case CC1101_OFF_MODE_IDLE:{
			response &= ~(1 << 0);
			response &= ~(1 << 1);
			break;
		}
		case CC1101_OFF_MODE_FSTXON:{
			response |= (1 << 0);
			response &= ~(1 << 1);
			break;
		}
		case CC1101_OFF_MODE_TX:{
			response &= ~(1 << 0);
			response |= (1 << 1);
			break;
		}
		case CC1101_OFF_MODE_RX:{
			response |= (1 << 0);
			response |= (1 << 1);
			break;
		}
	}
	command[0] = 0x17;
	command[1] = response;
	this->sendCommand(command, 2, NULL, 0);
}

void CC1101::setFrequency(CC1101_FREQUENCY frequency){
	this->SIDLE();
	uint8_t command[4];
	command[0] = 0x4D;
	this->frequency = frequency;
	switch(frequency){
		case CC1101_315MHz:{
			command[1] = 0x0C;
			command[2] = 0x1D;
			command[3] = 0x89;
			break;
		}
		case CC1101_433MHz:{
			command[1] = 0x10;
			command[2] = 0xA7;
			command[3] = 0x62;
			break;
		}
		case CC1101_868MHz:{
			command[1] = 0x21;
			command[2] = 0x62;
			command[3] = 0x76;
			break;
		}
		case CC1101_915MHz:{
			command[1] = 0x23;
			command[2] = 0x31;
			command[3] = 0x3B;
			break;
		}
		default:{
			command[1] = 0x21;
			command[2] = 0x62;
			command[3] = 0x76;
			this->frequency = CC1101_868MHz;
			break;
		}
	}
	this->sendCommand(command, 4, NULL, 0);
	this->setPower(this->power);
}
void CC1101::setPower(CC1101_POWER power){
	this->SIDLE();
	if(power > CC1101_N30dBm){
		this->power = CC1101_0dBm;
	}
	uint8_t powerTable[4][8] = {
		{0xC2,0xCB,0x85,0x51,0x34,0x1C,0x0D,0x12},
		{0xC0,0xC8,0x84,0x60,0x34,0x1D,0x0E,0x12},
		{0xC5,0xCD,0x86,0x50,0x26,0x1D,0x17,0x03},
		{0xC3,0xCC,0x84,0x8E,0x27,0x1E,0x0E,0x03},
	};
	uint8_t commandRead = 0xFE;
	uint8_t response[9];
	for(uint8_t i = 0; i < 9; i++){
		response[i] = 0;
	}
	this->sendCommand(&commandRead, 1, &(response[1]), 8);

	response[0] = 0x7E;
	response[1] = powerTable[this->frequency][this->power];
	this->sendCommand(response, 9, NULL, 0);
}
uint8_t CC1101::lenFIFORX(){
	uint8_t command = 0xFB;
	uint8_t len;
	this->sendCommand(&command, 1, &len, 1);
	return(len);
}
uint8_t CC1101::lenFIFOTX(){
	uint8_t command = 0xFA;
	uint8_t len;
	this->sendCommand(&command, 1, &len, 1);
	return(len);
}
CC1101_CRC_STATE CC1101::isCRC_OK(){
	uint8_t command = 0xF8;
	uint8_t response;
	this->sendCommand(&command, 1, &response, 1);
	if(response & (1 << 7)){
		return(CC1101_CRC_OK);
	}else{
		return(CC1101_CRC_ERROR);
	}
}
float CC1101::getRSSI(){
	uint8_t command = 0xF4;
	uint8_t response;
	this->sendCommand(&command, 1, &response, 1);
	if(response >= 128){
		return(((response-256)/2.0)-74);
	}else{
		return((response/2.0)-74);
	}
}
void CC1101::readFIFO(uint8_t *data, uint8_t len){
	this->SIDLE();
	uint8_t command = 0xFF;
	this->sendCommand(&command, 1, data, len);
}

void CC1101::enableRX(){
	this->SIDLE();
	this->SFRX();
	this->SRX();
}

CC1101_STATE_MACHINE CC1101::state(){
	uint8_t command = 0xF5;
	uint8_t response;
	this->sendCommand(&command, 1, &response, 1);
	return((CC1101_STATE_MACHINE)response);
}

void CC1101::setChannel(uint8_t channel){
	uint8_t command[2];
	command[0] = 0x0A;
	command[1] = channel;
	this->sendCommand(command, 2, NULL, 0);
}

void CC1101::setAddress(uint8_t address){
	uint8_t command[2];
	command[0] = 0x09;
	command[1] = address;
	this->sendCommand(command, 2, NULL, 0);

}


//Strobe
void CC1101::SRES(){
	uint8_t command = 0x30;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SFSTXON(){
	uint8_t command = 0x31;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SXOFF(){
	uint8_t command = 0x32;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SCAL(){
	uint8_t command = 0x33;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SRX(){
	uint8_t command = 0x34;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::STX(){
	uint8_t command = 0x35;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SIDLE(){
	uint8_t command = 0x36;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SWOR(){
	uint8_t command = 0x38;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SPWD(){
	uint8_t command = 0x39;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SFRX(){
	uint8_t command = 0x3A;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SFTX(){
	uint8_t command = 0x3B;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SWORRST(){
	uint8_t command = 0x3C;
	this->sendCommand(&command, 1, NULL, 0);
}
void CC1101::SNOP(){
	uint8_t command = 0x3D;
	this->sendCommand(&command, 1, NULL, 0);
}




