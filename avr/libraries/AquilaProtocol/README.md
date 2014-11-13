# Aquila Protocol

Events, actions and interactions based protocol.

## Changes in this new version:

Now works over Mesh library.

Internally uses short address, EUI Long address is used as unique identifier and for events.

New Command:

PROTOCOL_COM_EUI 14

gets EUI, like get name, but with raw address.

Now we use Aquila.anounce(BROADCAST), and send EUI Address instead of class on end of setup

In event emmit, the EUI Address is appended to the message after param