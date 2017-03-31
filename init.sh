#generating a run-time date for logging purposes
DATE_FILE="/etc/run_dates.cstars"
DATE=$(date -u) #generates date in UTC time
echo $DATE >> $DATE_FILE


XILLYBUS_IMAGE="/dev/xillybus_image"
UART_INPUT="/etc/uart_input"
DMA_INPUT="/etc/dma_input"

#starting uart telemetry controller
uartTMC&
UART_PID=$!

#sleeping for 1 second to ensure that uart Telemetry Controller is online before star_tracker
sleep 1

#STARting star tracker
star_tracker&
TRACKER_PID=$!

#dma_controller&
#DMA_PID=$!




