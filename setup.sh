#compiling the code for current directory
gcc -Wall attitude.c detect.c main.c track.c util.c -o star_tracker -lm;
gcc -Wall uart-controller.c -o uartTMC -lm;
#gcc -Wall dma-controller.c -o dmaTMC -lm;
clear;clear; 

#compiling the code as a system command
echo '====================STAR TRACKER==========================';
sudo gcc -Wall attitude.c detect.c main.c track.c util.c -o /bin/star_tracker -lm;
echo '====================UART CONTROLLER=======================';
sudo gcc -Wall uart-controller.c -o /bin/uartTMC -lm;
echo '====================DMA CONTROLLER========================';
#sudo gcc -Wall dma-controller.c -o /bin/dmaTMC -lm;

#creating the UART pipe
mkfifo uart_input.cstars
mkfifo dma_input.cstars

#removing the previous telemetry stash
rm telemetry_stash.cstars
