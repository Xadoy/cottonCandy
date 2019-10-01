#include "commNode.h"

//Default Constructor
CommNode::CommNode(){
    nodeId = 0;
}

CommNode::~CommNode(){
    //Stop the current transmission and clean up
}

int CommNode::send(const char msg[], const short nodeId){
    //TODO: Implement the sending
    return 0;
}

int CommNode::receive(char buffer[], unsigned short byteToRead){
    return 0;
}