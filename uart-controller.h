#define SLEEP_TIME (1e5) //microseconds
#define TIME_TILL_INACTIVE (1e6) //microseconds

#define UART_DEVICE "/dev/ttyPS1"
#define UART_INPUT "uart_input.cstars"
#define ON_TARGET_PIPE "/sys/class/gpio/gpio54/value"
#define SHUTTER_DOOR_PIPE "/sys/class/gpio/gpio55/value"
#define LOG_FILE "status_log.cstars"
#define TELEMETRY_STASH "telemetry_stash.cstars"


// masks
#define ALIVE_MASK (uint16_t)0x8000
#define BUSY_MASK (uint16_t)0x4000
#define LOCKED_MASK (uint16_t)0x2000
#define ROLL_MASK (uint16_t)0x1000
#define TARGET_MASK (uint16_t)0x0800
#define DOOR_MASK (uint16_t)0x0400

#define UART_OUTPUT_SIZE 21
#define SHOULD_LOG_TO_FILE 1

//structs
struct UartOutput {
        char header1; //0
        char header2; //1
        uint32_t frame_counter; //2,3,4,5
        uint16_t status_word; //6,7
        int     x; //8,9,10,11
        int y; //12,13,14,15
        int z; //16,17,18,19
        char checksum; //20
}; //21 BYTES TOTAL



//prototypes
char update_checksum(struct UartOutput* output_ptr);
uint16_t construct_status_word(uint16_t num_stars, int alive);
int send_over_uart(struct UartOutput* output_ptr);
int logToFile(int func_id, char message[]);
int logTelemetryToFile(struct UartOutput telemetry);
