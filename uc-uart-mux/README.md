# uc-uart-mux
UART mutiplexer, microcontroller based. 

Basically, create a ringbuffer in the uc that buffers UART commands and can send them to one of multiple possible outputs. 

Use case is to be able to switch between multiple serial ports without plugging/unplugging cables.