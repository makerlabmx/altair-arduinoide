#ifndef CONFIG_H
#define CONFIG_H

// Project configuration

// Size of hardware EEPROM
#define EEPROM_SIZE 8192

// Baudrate of serial communications (if any)
#define BAUD 57600

// Max EEPROM bytes used for protocol configuration storage.
#define PROTOCOL_PERSIST_SIZE 1024

#define PROTOCOL_MAXACTIONS 100
#define PROTOCOL_MAXEVENTS 100

// Use secured conections.
#define PHY_SECURITY_ENABLED 0

// Max data size
//AQUILAMESH_MAXPAYLOAD
#define MAX_DATA_SIZE 105

#endif	//CONFIG_H