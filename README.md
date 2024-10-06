# Transmission-Speed-Test
It is a simple program for transmission speed test (). The code also reveal basic socket programming.



$ ./server_side 
Server side listening on port 8080

Received 1048577 bytes

Send 1048576 bytes


$ ./client_side 

Average latency (micro second): 37.00

Send 1048576 bytes

Received 1048577 bytes

Average latency (micro second): 37.00
Upload Speed (Mbps): 11848.32
Download Speed (Mbps): 199.66

#

wget -O speedtest-cli https://raw.github.com/sivel/speedtest-cli/master/speedtest.py

chmod +x speedtest-cli

./speedtest-cli

https://blog.gtwang.org/linux/speedtest-cli-linux-command/