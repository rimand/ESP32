# BKKDW2019 Fish - Remote
## Remote
ฺBroadcast string data 
```
ss << [String];
std::string str = ss.str();
const char *cstr = str.c_str();
udp.broadcastTo(cstr, PORT_CH);
```

## Fish
Receive string data and convert to int
```
String myString = (const char*)packet.data();
int data = myString.toInt();
```
