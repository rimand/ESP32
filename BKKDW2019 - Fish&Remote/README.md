# BKKDW2019 Fish - Remote
## Remote

Client Connect Server
```
if (udp.connect(IPAddress(192, 168, 1, 155), PORT_CH)) {
    Serial.println("UDP connected");
    udp.onPacket([](AsyncUDPPacket packet) {
        // ----------- do something -----------
    }];
    udp.print("Hello Server!");
 }
```

ฺSend broadcast string data 
```
ss << [String];
std::string str = ss.str();
const char *cstr = str.c_str();
udp.broadcastTo(cstr, PORT_CH);
```

## Fish
Receive string data and convert to int
```
if (udp.listen(PORT_CH)) {      
  udp.onPacket([](AsyncUDPPacket packet) { 
    String myString = (const char*)packet.data();
    int data = myString.toInt();
    //---------- do something ------------
  });
}
```
