/*
 * CC1101.h
 *
 *  Created on: Mar 30, 2025
 *      Author: admin
 */

#ifndef INC_CC1101_HPP_
#define INC_CC1101_HPP_
#include "main.h"

typedef enum{
	CC1101_STATUS_CONNECTED,
	CC1101_STATUS_DISCONNECTED,
	CC1101_STATUS_ERROR
}CC1101_CONNECTION_STATE;
typedef enum{
	CC1101_FIFO_UNDERFLOW,
	CC1101_FIFO_OK
}CC1101_FIFO_UNDERFLOW_STATE;
typedef enum{
	CC1101_CRC_OK,
	CC1101_CRC_ERROR
}CC1101_CRC_STATE;
typedef enum{
	CC1101_NO_ADDRESS_CHECK,
	CC1101_ADDRESS_CHECK_NO_BROADCAST,
	CC1101_ADDRESS_CHECK_AND_BROADCAST,
	CC1101_ADDRESS_CHECK_AND_BROADCAST_AND_FF
}CC1101_ADDRESS_CHECK_STATE;
typedef enum{
	CC1101_315MHz,
	CC1101_433MHz,
	CC1101_868MHz,
	CC1101_915MHz
}CC1101_FREQUENCY;
typedef enum{
	CC1101_10dBm,
	CC1101_7dBm,
	CC1101_5dBm,
	CC1101_0dBm,
	CC1101_N10dBm,
	CC1101_N15dBm,
	CC1101_N20dBm,
	CC1101_N30dBm
}CC1101_POWER;
typedef enum {
    CC1101_SLEEP,
    CC1101_IDLE,
    CC1101_XOFF,
    CC1101_VCOON_MC,
    CC1101_REGON_MC,
    CC1101_MANCAL,
    CC1101_VCOON,
    CC1101_REGON,
    CC1101_STARTCAL,
    CC1101_BWBOOST,
    CC1101_FS_LOCK,
    CC1101_IFADCON,
    CC1101_ENDCAL,
    CC1101_RX,
    CC1101_RX_END,
    CC1101_RX_RST,
    CC1101_TXRX_SWITCH,
    CC1101_RXFIFO_OVERFLOW,
    CC1101_FSTXON,
    CC1101_TX,
    CC1101_TX_END,
    CC1101_RXTX_SWITCH,
    CC1101_TXFIFO_UNDERFLOW
} CC1101_STATE_MACHINE;

typedef enum{
	CC1101_2_FSK,
	CC1101_GFSK,
	CC1101_4_FSK
}CC1101_MODULATION_FORMAT_STATE;

typedef enum{
	CC1101_NO_PREAMBLE_SYNC,
	CC1101_15_16_SYNC,
	CC1101_16_16_SYNC,
	CC1101_30_32_SYNC,
	CC1101_NO_PREAMBLE_SYNC_CARRIER,
	CC1101_15_16_CARRIER,
	CC1101_16_16_CARRIER,
	CC1101_30_32_CARRIER
}CC1101_SYNC_MODE_STATE;

typedef enum{
	CC1101_OFF_MODE_IDLE,
	CC1101_OFF_MODE_FSTXON,
	CC1101_OFF_MODE_TX,
	CC1101_OFF_MODE_RX
}CC1101_OFF_MODE_STATE;

class CC1101{
	public:
		//Init
		CC1101(SPI_HandleTypeDef *hspi, GPIO_TypeDef* CS_PORT, uint16_t CS_PIN);
		void defaultConfiguration();
		void setChannel(uint8_t channel);
		void setAddress(uint8_t address);
		void setFrequency(CC1101_FREQUENCY frequency);
		void setPower(CC1101_POWER power);
		void setSyncWord(uint8_t msb, uint8_t lsb);
		void setAutoFlush(uint8_t value);
		void setAddressCheck(CC1101_ADDRESS_CHECK_STATE value);
		void setModulationFormat(CC1101_MODULATION_FORMAT_STATE value);
		void setSyncMode(CC1101_SYNC_MODE_STATE value);
		void setManchesterEncoding(uint8_t value);
		void setNumberPreamble(uint8_t value);
		void setRX_Off_Mode(CC1101_OFF_MODE_STATE value);
		void setTX_Off_Mode(CC1101_OFF_MODE_STATE value);

		//State
		CC1101_CONNECTION_STATE isConnected();
		CC1101_STATE_MACHINE state();
		float getRSSI();

		//transmit
		void transmit(uint8_t *data, uint8_t len);
		void clearFIFOTX();
		uint8_t lenFIFOTX();
		CC1101_FIFO_UNDERFLOW_STATE isFIFOTXUnderflow();

		//reseive
		CC1101_CRC_STATE isCRC_OK();
		void clearFIFORX();
		uint8_t lenFIFORX();
		void readFIFO(uint8_t *data, uint8_t len);
		void enableRX();
		CC1101_FIFO_UNDERFLOW_STATE isFIFORXUnderflow();

	private:
		//variable
		CC1101_FREQUENCY frequency = CC1101_868MHz;
		CC1101_POWER power = CC1101_0dBm;
		SPI_HandleTypeDef *hspi;
		GPIO_TypeDef* CS_PORT;
		uint16_t CS_PIN;
		//command
		void sendCommand(uint8_t *commandBuf, uint8_t commandLen, uint8_t *responseBuf, uint8_t responseLen);
		void writeFIFO(uint8_t *data, uint8_t len);
		//strobe
		void SRES();
		void SFSTXON();
		void SXOFF();
		void SCAL();
		void SRX();
		void STX();
		void SIDLE();
		void SWOR();
		void SPWD();
		void SFRX();
		void SFTX();
		void SWORRST();
		void SNOP();
};



#endif /* INC_CC1101_HPP_ */
