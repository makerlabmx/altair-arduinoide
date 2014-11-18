#ifndef CONFIG_H
#define CONFIG_H

// Project configuration

// Size of hardware EEPROM
#define EEPROM_SIZE 8192

// Baudrate of serial communications (if any)
#define BAUD 9600

// Max EEPROM bytes used for protocol configuration storage.
#define PROTOCOL_PERSIST_SIZE 1024

#define PROTOCOL_MAXACTIONS 100
#define PROTOCOL_MAXEVENTS 100

// Use secured conections.
#define PHY_SECURITY_ENABLED 0

// Specify address for devices without MAC/EUI chip, should be unique.
// #define DUMMY_ADDR {7,1,2,3,4,5,6,7}

// #define SEC_KEY {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}


#endif	//CONFIG_H