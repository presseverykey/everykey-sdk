The code in this folder is taken from the microbuilder LPC1343 code base (www.microbuilder.eu). Binary firmware images for the LPC1343 must contain a valid checksum (in an otherwise unused entry in the initial vector table) in order to be accepted by the chip's USB firmware loader. This tool fixes a .bin file by replacing the correct checksum in the file so it can be uploaded to the chip. It modifies a given file.

Compiling: make

Usage: checksum <file_to_be_fixed>

To use it with the Press Any Key toolchain, compile it and either put it in a place where it can be found by your search path or set the tool's path explicitly in the project's makefile.