//TODO: This is currently a dummy implementation for the driver class

#ifndef HEADER_DEVICE_DRIVER
#define HEADER_DEVICE_DRIVER
class DeviceDriver{

public:
    //Default Constructor
    DeviceDriver();

    //Copy constructor
    DeviceDriver(const DeviceDriver &driver);

    //Destructor
    ~DeviceDriver();

    int send(char* msg);

    int recv(int byteToRead, char* buffer);

private:

};

#endif