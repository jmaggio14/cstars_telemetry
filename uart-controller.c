#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART

#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
// sudo chmod a+rw /dev/ttyAMA0
#include "uart-controller.h"


//global vars
int global_log_count = 0;
int global_loop_index = 0;


//setting up starting log

int main(int argc, char const *argv[])
{
	logToFile(0, "\n\n////////////////// uartTMC START ///////////////////////\n\n");

	// logToFile("+++++++++++++++++++++ main() +++++++++++++++++++++++++++++");
	time_t last_time_active;
	last_time_active = time(NULL);

	struct UartOutput output;
	int e;
	unsigned short errorout[1024];
	int alive = 0;
	usleep(TIME_TILL_INACTIVE); //--> necessary for star_tracker ALIVE signal determination to work

	logToFile(0, "building initial telemetry struct");
	output.header1 = 'C'; //constant
	output.header2 = 'T'; //constant
	output.frame_counter = 0; //increments by 1 every new image input
	output.status_word = 0b0000000000000000; //will update with each new image
	output.x = 0; //updates with each new image
	output.y = 0; //updates with each new image
	output.z = 0; //updates with each new image
	output.checksum = update_checksum(&output); //updates with each new iteration


	//checking if telemetry_stash exists -- should only exist if crash occurs
	//if it exists, read in old telemetry
	if ( access(TELEMETRY_STASH,F_OK) != -1 ){
		logToFile(0, "loading in telemetry stash from file");
		FILE* initial_telemetry = fopen(TELEMETRY_STASH,"r");
		fread(&output,1,UART_OUTPUT_SIZE,initial_telemetry);
		// memcpy(&output,buff,UART_OUTPUT_SIZE);
		fclose(initial_telemetry);
	}

	// int index = 0;
	logToFile(0, "|||||||| beginning persistent loop |||||||");
	while(1){
		// logToFile(0,"\n BEGIN NEW LOOP THROUGH");
		logToFile(0, "setting input buffer to all zeros");
		for(e=0;e<1024;e++){
			errorout[e] = 0;  // reseting values to zero
		}
		//openning UART_INPUT in non-blocking mode
		logToFile(0, "opening uart_input.cstars in non-blocking mode");
		int image_fifo_d = open(UART_INPUT, O_RDONLY | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
		logToFile(0, "sleeping for 100 milliseconds for timing purposes");
		usleep(SLEEP_TIME); //sleeping at period of 10Hz
		logToFile(0, "attempting to read uart_input.cstars into input buffer");
		ssize_t x = read(image_fifo_d,&errorout,2048);
		close(image_fifo_d);

		// printf("x: %zd\n",x);
		if ( (x != 2048) ){ //data is unavailable
				logToFile(0, "NO INPUT DATA AVAILABLE or read error from uart_input.cstars");
				//checking last time since data read and assuming star tracker is dead
				// after TIME_TILL_INACTIVE
				if( (time(NULL) - last_time_active) > TIME_TILL_INACTIVE){
					alive = 0;
				}
				else{
					alive = 1;
				}
				logToFile(0, "'--> SENDING ZEROED TELEMETRY w/ new status_word & checksum");
				output.status_word = construct_status_word(0,alive);
				output.x = 0;
				output.y = 0;
				output.z = 0;
				output.checksum = update_checksum(&output);
			}

		else{ //WE HAVE AN IMAGE
				logToFile(0, "INPUT READ SUCCESSFUL");
				logToFile(0, "begin construction of new telemetry struct");
				output.frame_counter++; //new frame
				//assigning proper inputs from errorout
				output.status_word = construct_status_word(errorout[1],1);
				output.x = errorout[4];
				output.y = errorout[5];
				output.z = errorout[6];
				output.checksum	= update_checksum(&output);
				last_time_active = time(NULL); //reseting alive clock
			}
		// printf("frame_counter: %d\n",output.frame_counter );
		send_over_uart(&output);

		//WRITE TO TELEMETRY_STASH
		logToFile(0, "saving current telemetry to telemetry stash");
		FILE* telemetry_stash = fopen(TELEMETRY_STASH,"w");
		fwrite(&output,1,UART_OUTPUT_SIZE,telemetry_stash);
		fclose(telemetry_stash);

		global_loop_index++;

		logTelemetryToFile(output);
	} //end while
	return 0;
}

char update_checksum(struct UartOutput* output_ptr){
		// logToFile("++++++++++++++ update_checksum() +++++++++++++++++++++");
		// int i;
		// char sum = 0x00;
		// for(i=0;i<21;i++){
		// 	sum = sum + (char)output_ptr[i];
		// }
		// size_t len = sizeof(struct UartOutput);
		size_t len = UART_OUTPUT_SIZE;
		unsigned char* data;
		data = (unsigned char*) output_ptr;

		logToFile(2,"computing checksum");
		unsigned char sum = 0;
		while(len--){
			 sum += *data++;
				// printf("length: %lu checksum: %x\n",20-len,sum);
			}
		// printf("This should be 20: %d\n",i);

		return sum;
	}

uint16_t construct_status_word(uint16_t num_stars, int alive){
	// logToFile("+++++++++++++++ construct_status word() +++++++++++++++++");
	uint16_t status_word = 0b0000000000000000; //all zeros initially

	if (alive == 1){
		logToFile(1,"input recieved recently, star_tracker considered ALIVE");
		status_word = status_word | ALIVE_MASK;
	}
	else{
		logToFile(1,"no recent input, star_tracker considered DEAD");
	}

	if(num_stars < 3)	{
		status_word = status_word | BUSY_MASK;
	} //NOT ENOUGH STARS TO COMPUTE
	else{
		status_word = status_word | LOCKED_MASK;//we will always be "locked" if there are 3 stars
		status_word = status_word | ROLL_MASK; //we can always detect roll if we have 3 or more stars
	}

	//CHECKING ON-TARGET (non-blocking)
	logToFile(1,"checking ON-TARGET signal");
	char on_target = '0';
	int fifo_target = open(UART_DEVICE, O_RDONLY | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	/*size_t t = */read(fifo_target, &on_target, 1);
	// printf("OPENING TARGET PIPE. status: %c\n",on_target);
	close(fifo_target);
	if (on_target!='0') status_word = status_word | TARGET_MASK;

	//SHUTTER DOOR SIGNALS (non-blocking)
	logToFile(1,"checking SHUTTER-DOOR signal");
	char shutter_door = '0';
	int fifo_door = open(UART_DEVICE, O_RDONLY | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	/*size_t s = */read(fifo_door, &shutter_door, 1);
	// printf("OPENING SHUTTER PIPE. status: %c\n",shutter_door);
	close(fifo_door);
	if (shutter_door!='0') status_word = status_word | DOOR_MASK;
	return status_word;
}


int send_over_uart(struct UartOutput* output_ptr){
	// logToFile("+++++++++++++++++ send_over_uart() +++++++++++++++++++++++");
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
	logToFile(3,"opening uart device in non-blocking mode");
	uart0_filestream = open(UART_DEVICE, O_WRONLY | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		logToFile(3,"UNABLE TO CONNECT TO UART DEVICE");
		//ERROR - CAN'T OPEN SERIAL PORT
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
		logToFile(3,"connected to uart device");
		int count = write(uart0_filestream, output_ptr, 21);		//Filestream, bytes to write, number of bytes to write
		if (count < 0)
		{
			logToFile(3,"**********UART WRITE FAILURE***************");
		}
		else{
			logToFile(3,"*********UART WRITE SUCCESSFUL*************");
		}
	}


	//----- CLOSE THE UART -----
	close(uart0_filestream);

	return 0;
}




int logToFile(int func_id, char message[]){
	//opening log in append mode
	//FORMAT: [loop_index]: message
	char func[25];
	if (func_id == 0) {
		strcpy(func,"         main()        ");
	}
	else if(func_id == 1){
		strcpy(func,"construct_status_word()");
	}
	else if(func_id == 2){
		strcpy(func,"   update_checksum()   ");
	}
	else{
		strcpy(func,"    send_over_uart()   ");
	}

	if(SHOULD_LOG_TO_FILE){
		FILE* error_log = fopen(LOG_FILE,"a");
		fprintf(error_log, "[%i] loop#%i | %s |%s\n", global_log_count,
		 													global_loop_index, func, message);
		fclose(error_log);
	}
		global_log_count++;
	return 0;
}


int logTelemetryToFile(struct UartOutput telemetry){
	if(SHOULD_LOG_TO_FILE){
		FILE* error_log = fopen(LOG_FILE,"a");

		fprintf(error_log,
				"header: %c%c\nframe_counter: %u\nstatus_word: %hu\ndelta x: %i\ndelta y: %i\ndelta z: %i\nchecksum: %c\n\n\n",
				telemetry.header1,
				telemetry.header2,
				telemetry.frame_counter,
				telemetry.status_word,
				telemetry.x,
				telemetry.y,
				telemetry.z,
				telemetry.checksum);

		fclose(error_log);
	}
	return 0;
}

// //structs
// struct UartOutput {
// 	char header1; //0
// 	char header2; //1
// 	uint32_t frame_counter; //2,3,4,5
// 	uint16_t status_word; //6,7
// 	int	x; //8,9,10,11
// 	int y; //12,13,14,15
// 	int z; //16,17,18,19
// 	char checksum; //20
// }; //21 BYTES TOTAL
