#!/bin/bash
echo "beginning init" >> /root/cstars-start.log

#starting uart telemetry controller
uartTMC&
#/root/scripts/cstars_telemetry/uartTMC &
UART_PID=$!

#sleeping for 1 second to ensure that uart Telemetry Controller is online before star_tracker
sleep 1

#STARting star tracker
#/root/scripts/cstars_telemetry/star_tracker &
#TRACKER_PID=$!

#dma_controller&
#DMA_PID=$!

echo "done init" >> /root/cstars-start.log

echo "initiating uartTMC watchdog"
#constantly checks UART_PID and restarts if necessary
while :
do
if ps -p $UART_PID > /dev/null ; then
        :
else
        uartTMC&
	#/root/scripts/cstars_telemetry/uartTMC &
        UART_PID=$!
fi
done


