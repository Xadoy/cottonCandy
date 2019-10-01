#ifndef HEADER_COMM_NODE
#define HEADER_COMM_NODE

class CommNode{

public:
    //Default Constructor
    CommNode();

    //Copy constructor
    CommNode(const CommNode &node);

    //Destructor
    ~CommNode();

    //Constructor. Requires driver and assigned addr
    //TODO: Uncomment when driver has been done
    //CommNode(DeviceDriver driver, const char[] addr);

    /**
     * Send Function
     */
    int send(const char msg[], const short nodeId);

    //Receive function
    int receive(char buffer[], unsigned short bytesToReceive);

private:
    short nodeId;
    //DeviceDriver driver;

};

#endif