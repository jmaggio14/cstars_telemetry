#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART

#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
// sudo chmod a+rw /dev/ttyAMA0


#define SLEEP_TIME (1e5) //microseconds
#define TIME_TILL_INACTIVE (1e6) //microseconds

#define UART_DEVICE "/dev/ttyPS1"
#define UART_INPUT "uart_input.cstars"
#define ON_TARGET_PIPE "/sys/class/gpio/gpio54/value"
#define SHUTTER_DOOR_PIPE "/sys/class/gpio/gpio55/value"

// masks
#define ALIVE_MASK (uint16_t)0x8000
#define BUSY_MASK (uint16_t)0x4000
#define LOCKED_MASK (uint16_t)0x2000
#define ROLL_MASK (uint16_t)0x1000
#define TARGET_MASK (uint16_t)0x0800
#define DOOR_MASK (uint16_t)0x0400

#define UART_OUTPUT_SIZE 21

struct UartOutput {
	char header1; //0
	char header2; //1
	uint32_t frame_counter; //2,3,4,5
	uint16_t status_word; //6,7
	int	x; //8,9,10,11
	int y; //12,13,14,15
	int z; //16,17,18,19
	char checksum; //20
};




char update_checksum(struct UartOutput* output_ptr);
uint16_t construct_status_word(unsigned short errorout[1024], int alive);
int send_over_uart(struct UartOutput* output_ptr);


int main(int argc, char const *argv[])
//argv[1] --> INPUT NAME
//argv[2] --> DEVICE TO WRITE TO
{
	time_t last_time_active;
	last_time_active = time(NULL);

	struct UartOutput output;
	unsigned short errorout[1024];
	int e;
	int alive = 0;
	usleep(.5); //necessary on boot

	//SETTING DEFAULT VALUES FOR OUTPUT
	// strcpy(output.header, 'C'); //never changes
	// strcpy(output.header, 'T'); //never changes
	output.header1 = 'C';
	output.header2 = 'T';
	output.frame_counter = 0; //increments by 1 every new image input
	output.status_word = 0x0000;
	output.x = 0; //updates with each new image
	output.y = 0; //updates with each new image
	output.z = 0; //updates with each new image
	output.checksum = update_checksum(&output); //updates with each new iteration

	// int index = 0;
	while(1){
		// printf("%d\n",index++);
		// printf("%s\n","SLEEPING");
		// printf("%s\n","AWAKE");
		for(e=0;e<1024;e++){
			errorout[e] = 0;  // reseting values to zero
		}
		// printf("%s\n","ERROROUT SET TO 0");

		//CHECKING FOR IMAGE
 	// 	int fifo_d = open(UART_INPUT, O_RDONLY | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
		// fcntl(fifo_d, F_SETFL, O_NONBLOCK);
		// ssize_t x = read(fifo_d, &errorout, 1024);
		// close(fifo_d);
		// printf("FILE HANDLE %d\n",fifo_d);
		// int print_i;
		// for(print_i=0;print_i<1024;print_i++) printf("%u\n",errorout[print_i]);

		//IMPLEMENTING non-blocking fifo --> should exit if there isn't an issue
		// FILE* image_fifo = popen(UART_INPUT,"r");
		// FILE* image_fifo = open(UART_INPUT,"r");
		// printf("EOF: %zd\n",eof_loc );
		// int image_fifo_d = fileno(image_fifo);
		int image_fifo_d = open(UART_INPUT, O_RDONLY | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
		// fcntl(image_fifo_d,F_SETFL,O_NONBLOCK);
		usleep(SLEEP_TIME); //sleeping at 10Hz
		// printf("errno: %d\n",errno );
		ssize_t x = read(image_fifo_d,&errorout,2048);
		// printf("errno: %d\n",errno );

		close(image_fifo_d);

		// printf("x: %zd\n",x);
		if ( (x < 0 && errno == EAGAIN) ){ //data is unavailable
				printf("NO IMAGE AVAILABLE\n");
				if( (time(NULL) - last_time_active) > TIME_TILL_INACTIVE){
					alive = 0;
				}
				else{
					alive = 1;
				}
				output.status_word = construct_status_word(errorout,alive);
				output.checksum = update_checksum(&output);
			}

		else{ //WE HAVE AN IMAGE
				// printf("WE HAVE AN IMAGE\n");
				output.frame_counter++; //new frame
				//assigning proper inputs from errorout
				output.status_word = construct_status_word(errorout,1);
				output.x = errorout[4];
				output.y = errorout[5];
				output.z = errorout[6];
				output.checksum	= update_checksum(&output);
				last_time_active = time(NULL); //reseting alive clock
			}
		// printf("frame_counter: %d\n",output.frame_counter );
		send_over_uart(&output);
		// break;
	}
	return 0;
}

char update_checksum(struct UartOutput* output_ptr){
		// int i;
		// char sum = 0x00;
		// for(i=0;i<21;i++){
		// 	sum = sum + (char)output_ptr[i];
		// }
		// size_t len = sizeof(struct UartOutput);
		size_t len = UART_OUTPUT_SIZE;
		unsigned char* data;
		data = output_ptr;
		unsigned char sum = 0;
		while(len--){
			 sum += *data++;
				// printf("length: %lu checksum: %x\n",20-len,sum);
			}
		// printf("This should be 20: %d\n",i);

		return sum;
	}

uint16_t construct_status_word(unsigned short errorout[1024], int alive){
	uint16_t status_word = 0x0000; //all zeros initially
	uint16_t num_stars = errorout[1];

	if (alive) status_word = status_word | ALIVE_MASK;

	if(num_stars < 3)	{
		status_word = status_word | BUSY_MASK;
	} //NOT ENOUGH STARS TO COMPUTE
	else{
		status_word = status_word | LOCKED_MASK;//we will always be "locked" if there are 3 stars
		status_word = status_word | ROLL_MASK; //we can always detect roll if we have 3 or more stars
	}

	//CHECKING ON-TARGET (non-blocking)
	char on_target = '0';
	int fifo_target = open(UART_DEVICE, O_RDONLY | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	/*size_t t = */read(fifo_target, &on_target, 1);
	// printf("OPENING TARGET PIPE. status: %c\n",on_target);
	close(fifo_target);
	if (on_target!='0') status_word = status_word | TARGET_MASK;

	//SHUTTER DOOR SIGNALS (non-blocking)
	char shutter_door = '0';
	int fifo_door = open(UART_DEVICE, O_RDONLY | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	/*size_t s = */read(fifo_door, &shutter_door, 1);
	// printf("OPENING SHUTTER PIPE. status: %c\n",shutter_door);
	close(fifo_door);
	if (shutter_door!='0') status_word = status_word | DOOR_MASK;
	return status_word;
}


int send_over_uart(struct UartOutput* output_ptr){
	//-------------------------
	//----- SETUP USART 0 -----
	//-------------------------
	//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
	int uart0_filestream = -1;
	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	uart0_filestream = open(UART_DEVICE, O_WRONLY | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		printf("%s\n","NO UART DEVICE AVAILABLE" );
		//ERROR - CAN'T OPEN SERIAL PORT
		 printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}

	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);

	if (uart0_filestream != -1)
	{
		int count = write(uart0_filestream, output_ptr, 21);		//Filestream, bytes to write, number of bytes to write
		if (count < 0)
		{
			 printf("UART TX error\n");
		}
	}


	//----- CLOSE THE UART -----
	close(uart0_filestream);

	return 0;
}
