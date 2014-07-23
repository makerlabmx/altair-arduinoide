#ifndef CONFIG_H
#define CONFIG_H

// Project configuration

// Baudrate of serial communications (if any)
#define BAUD 57600

// Use secured conections.
#define PHY_SECURITY_ENABLED 0

// Specify address for devices without MAC/EUI chip, should be unique.
#define DUMMY_ADDR {1,2,0,0,0,0,0,0}

// #define SEC_KEY {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}


#endif	//CONFIG_H
