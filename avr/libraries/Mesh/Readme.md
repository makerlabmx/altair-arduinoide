# Aquila Mesh

based upon atmel's lwm. see src/lwm/licence.txt for it's licence

Endpoints:

Atmel's LWM provides Endpoints, that work in a similar fashion as TCP/UDP Ports. There are 16 endpoints available.

With Aquila, endpoints 0 to 7 are free for user's custom implementations, endpoints 8-16 are reserved as follows:

15: Short Address collision discovery
14: WSerial
13: Aquila Protocol
12: Services
8 - 11: Reserved