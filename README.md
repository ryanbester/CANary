# CANary

CAN bus reverse engineering and packaging tool.

## Command Line System

CANary is built around a command line system. Not a typical CLI application, but every command in the GUI is equivalent
to a textual command.

This has the benefit that commands can be chained and CANary ran headless to generate container files.

## Development Hardware

CANary supports many connection types for receiving packets, such as socketcand, can0 interface, to direct connections
to MCP251x chips through SPI.

CANaryd can also be used if the transceiver device is remote, for developing pin detection systems. CANaryd is not a
replacement of socketcand, and is typically used alongside.

## Development Database

Rather than sending continuous queries to the central CANary database, CANary can download a local development database,
which can add features such as code completion for inheritance, etc.

## Container Format

Zip/gzip/zlib format containing DBC, pin detection, and action definition files, designed to be distributed over the
network.

### container.json

Registers filenames used.
Contains metadata such as name of vehicle, etc.
Also specifies what bus is used for OBD II PIDs.
Container format version. See Container Format Updates section below.
Specific container version. See Container Updates section below.

### Pin Detection System

What bus is on what pins.
Characteristics of each pin, such as volts, time period, frequency, etc.
Bus name, type (single wire, normal), and baudrate, etc.
EasySMU?

### DBC Files

Should be named [busname].dbc. Actual filename is specified in container.json file.

Define CAN IDs for that bus and signals attached to them.

### Action Definition Files

Add-on to DBC files and specifies what messages/signals are sent on what bus for each action.

For example, locking doors could send multiple different CAN messages on different buses.

Specifies what bus and protocol is used for what action, e.g. speed -> message 0x452 on hs-can bus, rpm -> OBD II PID on
hs-can bus.

### DTC Codes

Contains manufacturer specific DTC codes.

Always inherits from `__base`, which is a built-in database containing standard DTC codes.

### Inheritance

A container file can inherit from other containers in the CANary database.

1. Add reference to container.json "inherits" section, in form brand:model:id, giving it an id for reference in the
   container.
2. DBC files can be wholly inherited by naming them id/dbcfilename.dbc in the container.json instead of file name
3. Or parts can be inherited with #inherit directive inside the file.

A good use of inheritance is manufacturer DTC codes, which are usually the same per manufacturer.

### __base tag

Using `__base` instead of model or other tags specifies the container as a base to be inherited from.
For example, a brand could use the same OBD pinout in all vehicles, so each vehicle can inherit the pin detection from
`brand:__base`.

### Container Format Updates

The container format is designed to be expandable in future updates.
Future updates could support SAE J1939, or even the manufacturer's proprietary programming protocols.

### Container Updates

The CANary Database features auto update for vehicle containers, so if a container for a vehicle receives an update,
that container is automatically downloaded onto the client.

## CANary Database

The CANary database stores containers for each vehicle, and uses the VIN number and pin detection system to match what
container is for what vehicle, i.e. plug-n-play.

CANary database indexes pin detection definitions, for easy lookup.

Example:

1. Connected to vehicle
2. Pin 2 gives xx Volts
3. Looks up in Pin detection database
4. CANary database responds with: Could be these containers: ...
5. Finds what bus is used for OBD II PIDs and what baudrate
6. Connects to bus and sends VIN PID
7. Looks up VIN in database and retrieves correct container format

If a container contains inheritance, a merged container is generated on download and that generated container is stored.

### VIN Database

The VIN database is a separate entity to container files, and usually needs to be requested to be edited, and allocate a
range of VIN numbers.
