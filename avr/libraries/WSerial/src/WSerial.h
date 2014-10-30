#ifndef WirelessSerial_h
#define WirelessSerial_h

#include <inttypes.h>
#include "Stream.h"

// Showld be equal or less to the LWM max payload size
#define WSERIAL_BUFFER_SIZE 96
#define WSERIAL_SEND_INTERVAL 1
#define WSERIAL_ENDPOINT 1

class WirelessSerial : public Stream
{
	private:
		unsigned long lastTimeSent;	

	public:
		void begin(uint16_t destAddr);
		void end();
		virtual int available(void);
		virtual int peek(void);
		virtual int read(void);
		virtual void flush(void);
		virtual size_t write(uint8_t);
		inline size_t write(unsigned long n) { return write((uint8_t)n); }
		inline size_t write(long n) { return write((uint8_t)n); }
		inline size_t write(unsigned int n) { return write((uint8_t)n); }
		inline size_t write(int n) { return write((uint8_t)n); }
		using Print::write; // pull in write(str) and write(buf, size) from Print
		operator bool() { return true; }

		void sendPacketNow();
		void loop();

};

extern WirelessSerial WSerial;

#endif //WirelessSerial_h