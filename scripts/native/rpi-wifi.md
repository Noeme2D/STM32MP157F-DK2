### Wifi connection 

```
#For general Linux devices (using Network Manager CLI version)
# Step 1: Enabling Wifi Device
Use the command: nmcli dev status to see the status of all network interfaces.
To check if WIFI device is enabled or not use the command: nmcli radio wifi
If WIFI is disabled, enable it using command: nmcli radio wifi on

Note:
If the wifi card is blocked use the command:
echo "blacklist hp_wmi" | sudo tee /etc/modprobe.d/hp.conf
sudo rfkill unblock all


#Step 2: Identify the network connection 
Use command: nmcli dev wifi list to scan for nearby WIFI networks

#Step 3: Connect to WIFI
Use the command: nmcli dev wifi connect <network-ssid> //network-ssid will b ethe name of the network to join
If the WIFI has password security use the command: nmcli dev wifi connect <network-ssid> password <network password>

#Step 3: Test
use ping command to test your connection
```

### Wifi diconnect
// Force disconnect: Use command: sudo ifconfig wlan0 down //This will turn the WIFI device driver off. To turn it back on use command: sudo iconfig wlan0 up followed by:  sudo dhclient wlan0 // here wlan0 is not the only thing that can be put, it depends on what connection is goin on
// Another way: use Command: nmcli c down id NAME' - Disconnects the connection NAME
                             nmcli c up   id NAME' - Connects back
                             
        

