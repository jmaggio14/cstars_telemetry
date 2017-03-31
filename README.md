1) Navigate to directory containing the source code

2) ./setup.sh (compiles with 1 inconsequential warning)


*_TO RUN CODE ON STARTUP_*
	add the following to */etc/rc.local* (before the "exit 0")
	
	"INSERT_PATH_TO_SOURCE_CODE_HERE/init.sh"


*to run startracker*
	$ star_tracker
*to run the uart telemetry controller*
	$ uartTMC

_OTHER NOTES_
file paths are defined using precompiled instructions
	
	*main.c* 
		#define INPUT_IMAGE_PIPE "/dev/xillybus_image"

	*uart-controller.c*
		#define UART_DEVICE "/dev/ttyPS1"
		#define UART_INPUT "uart_input.cstars"
		#define ON_TARGET_PIPE "/sys/class/gpio/gpio54/value"
		#define SHUTTER_DOOR_PIPE "/sys/class/gpio/gpio55/value"

	*util.c*
		#define UART_OUTPUT "uart_input.cstars"
