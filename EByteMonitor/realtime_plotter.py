'''    
    Copyright 2020, Network Research Lab at the University of Toronto.

    This file is part of CottonCandy.

    CottonCandy is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CottonCandy is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CottonCandy.  If not, see <https://www.gnu.org/licenses/>.
'''

'''
This is a utility Python script for monitoring network activities among nearby
nodes built with Ebyte E22 tranceivers. To use it, an Ebyte E22 device must be 
first configured to use:

1. The correct channel and air rate.
2. Transparent mode.

This monitoring device shall be connected to the PC directly via a USB port. 
The program uses the monitoring device to log the packets sent among nodes,
and uses networkx to visualize the network topology. 
'''

import datetime as dt
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import networkx as nx
import serial
import time
import binascii
import sys
from datetime import datetime
from networkx.drawing.nx_agraph import write_dot, graphviz_layout

# A list of nodes in the network
node_list=[]

# A dictionary for keeping the connection and connection timestamp for plotting
edge_labels={}

G = nx.Graph()

node_info={}

MESSAGE_TYPE_STRING = ["MESSAGE_JOIN", "MESSAGE_JOIN_ACK", "MESSAGE_JOIN_CFM",
                       "MESSAGE_CHECK_ALIVE", "MESSAGE_REPLY_ALIVE",
                       "MESSAGE_GATEWAY_REQ", "MESSAGE_NODE_REPLY", "MESSAGE_MULTIHOP"]

TYPE_MESSAGE_JOIN = 1
TYPE_MESSAGE_JOIN_ACK = 2
TYPE_MESSAGE_JOIN_CFM = 3
TYPE_MESSAGE_CHECK_ALIVE = 4
TYPE_MESSAGE_REPLY_ALIVE = 5
TYPE_MESSAGE_GATEWAY_REQ = 6
TYPE_MESSAGE_NODE_REPLY = 7

# Debugging message only used in special tests
TYPE_MESSAGE_MULTIHOP = 8

MY_NODE_SIZE = 1200

def plot():
    #Map the color (Gateway has a different color)
    color_map = []
    for node in G:
        if int(node, 16) > 0x8000:
            #This is a gateway
            color_map.append('g')
        else:
            color_map.append('y')

    plt.clf()
    write_dot(G,'test.dot')
    pos=graphviz_layout(G, prog='neato')
    nx.draw(G,pos, with_labels=True, node_color=color_map, node_size = MY_NODE_SIZE)
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, label_pos=0.5)

# This function is called periodically from FuncAnimation
def animate(i, ser, file):

    if ser.in_waiting > 0:
        c = ser.read(1)
    else:
        plot()
        return

    msg_type = int.from_bytes(c, 'big')

    if msg_type < TYPE_MESSAGE_JOIN or msg_type > TYPE_MESSAGE_MULTIHOP:
        print("Unknown message type:" + str(msg_type) + ". Discarded")
        return

    # Read the src and dest address in the header
    src_addr = ser.read(2).hex().upper()
    dest_addr = ser.read(2).hex().upper()

    current_time = datetime.now().strftime("%Y-%m-%d-%H:%M:%S")

    log_string = current_time + " " + MESSAGE_TYPE_STRING[msg_type - 1] + ": sent from 0x" + str(src_addr) + " to 0x" \
        + str(dest_addr)

    print(log_string)
    log_file.write(log_string)
    log_file.write('\n')

    edge = (src_addr, dest_addr)

    # Start to process the message
    if msg_type == TYPE_MESSAGE_JOIN or msg_type == TYPE_MESSAGE_JOIN_ACK:

        # Read the extra byte in a JOIN_ACK message
        if msg_type == TYPE_MESSAGE_JOIN_ACK:
            ser.read(1)

        # Keep a record of all discovered nodes, a newly discovered node should be plotted as well
        if src_addr not in node_list:
            node_list.append(src_addr)
            G.add_node(src_addr)

        # If a previously discovered node starts join again, it means the connection with parent is down
        # Thus we need to find and remove this edge
        if msg_type == TYPE_MESSAGE_JOIN and src_addr in node_list and len(G.edges(src_addr)) > 0:
            parent = node_info.get(src_addr)

            edge_to_remove = (src_addr, parent)

            G.remove_edge(src_addr, parent)
            del edge_labels[edge_to_remove]

    elif msg_type == TYPE_MESSAGE_JOIN_CFM:

        # Read the extra byte in a JOIN_CFM message
        if msg_type == TYPE_MESSAGE_JOIN_CFM:
            ser.read(1)

        # Receiving a JOIN_CFM message means that the dest node becomes the parent of the src node

        # If the parent node is not yet recorded, record it
        if dest_addr not in node_list:
            node_list.append(src_addr)


        edge_labels[edge] = 'Last Seen: ' + current_time

        node_info[src_addr] = dest_addr

        G.add_edge(src_addr, dest_addr)

    elif msg_type == TYPE_MESSAGE_CHECK_ALIVE:
        # Read the extra byte in a CHECK_ALIVE message
        if msg_type == TYPE_MESSAGE_CHECK_ALIVE:
            ser.read(1)

    elif msg_type == TYPE_MESSAGE_REPLY_ALIVE:

        # Receiving a REPLY_ALIVE message means that the dest node has confirmed its aliveness to the src node
        if edge_labels.get(edge) == None:
            # This connection has never been seen before, add it to the graph
            node_list.append(src_addr)
            node_list.append(dest_addr)

            G.add_edge(src_addr, dest_addr)

        edge_labels[edge] = 'Connected: ' + current_time

    elif msg_type == TYPE_MESSAGE_NODE_REPLY:

        if src_addr not in node_list:
            node_list.append(src_addr)
            G.add_node(src_addr)

        if dest_addr not in node_list:
            node_list.append(dest_addr)
            G.add_node(dest_addr)

        seq_num = int.from_bytes(ser.read(1),byteorder='big')
        datalen = int.from_bytes(ser.read(1), byteorder='big')

        '''
        Note that the code here can result in wrong topology
        if the program is started when the message is forwarded
        by intermediate nodes. This is because the node reply
        that is being forwarded has the source address of the
        original source node. Thus, the program may create a
        edge between the original source node and a node that 
        is multiple hops away from the original source node.
        A temporary solution is to always start the program
        before starting the network or at an idle period
        when there is no REQ nor REPLY messages on the air.
        '''
        if not nx.algorithms.has_path(G, src_addr, dest_addr):
            if edge_labels.get(edge) == None:
                G.add_edge(src_addr, dest_addr)

        if edge_labels.get(edge) != None:
            edge_labels[edge] = 'Last Seen: ' + current_time

        node_info[src_addr] = dest_addr

        print("Reply for SeqNum " + str(seq_num) + " = " + ser.read(datalen).hex().upper())

    elif msg_type == TYPE_MESSAGE_GATEWAY_REQ:
        
        if src_addr not in node_list:
            node_list.append(src_addr)
            G.add_node(src_addr)

        seq_num = int.from_bytes(ser.read(1),byteorder='big')
        next_req_time = int.from_bytes(ser.read(4), byteorder='little')

        backoff_time = int.from_bytes(ser.read(4), byteorder='little')
        print("Next Gateway REQ (" + str(seq_num + 1) + ") in " + str(next_req_time) + "ms. Backoff time = " + str(backoff_time) + "ms")

    # Update the plot
    plot()



if __name__ == "__main__":
    
    if len(sys.argv) < 2:
        print("Usage: realtime_plotter.py <serial_port_path> [gateway address]")
        exit(1)

    port = sys.argv[1]

    print("Try to open serial port: " + port)
    ser = serial.Serial(port, 9600, timeout=1)
    log_file =open("log.txt", "w")

    gateway = "FFFF"
    if len(sys.argv) == 3:
        gateway = sys.argv[2]
        print("User-defined gateway is 0x" + gateway)
        node_list.append(gateway)
        G.add_node(gateway)


    # Create figure for plotting
    fig = plt.figure(figsize=(12,9))
    plt.title("LoRa Multi-hop Network Visualizer")
    plot()

    # Set up plot to call animate() function periodically
    ani = animation.FuncAnimation(fig, animate, fargs=(ser, log_file), interval=1000)
    plt.show()
