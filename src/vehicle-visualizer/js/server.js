/**
 * This file contains the server logic. The server should always be launched with Node.js (i.e. node server.js).
 * The goal of the Node.js server script is to read the object information from ms-van3t (via UDP/dgram socket) and
 * pass it to the client via socket.io.
 * This Node.js code also sets up an HTTP server for the client (i.e. the browser) to connect to.
 */

// Read a possible mapbox token from the "mapbox_token" file
var mapbox_token = "none";

const fs = require('fs');
const path = require('path');

var filepath = path.join(__dirname, 'mapbox_token');
fs.readFile(filepath, 'utf8', function (err,data) {
    if (err) {
        console.log("Cannot read the mapbox_token file. Error: ",err);
        process.exit(1);
    }

    if(data.length !== 0) {
        let splitted_data = data.split(/\r\n|\r|\n/);
        mapbox_token = splitted_data[0];

        console.log("Vehicle Visualizer: specified a Mapbox token via the mapbox_token file.");
    }
});

// Read the server port as a command line option
var server_argv = process.argv.slice(2);

if(server_argv.length!=1) {
    console.error("VehicleVisualizer: Error. One argument is expected and " + server_argv.length.toString() + " were specified.");
    process.exit(1);
} else {
    console.log("VehicleVisualizer: HTTP server listening on port: " + server_argv[0]);
}

// Create a new HTTP server with express.static
const express = require('express');
const app = express();
app.use(express.static(path.join(__dirname, '/')));

const http = require('http').Server(app);

// Create a UDP socket to receive the data from ms-van3t
// As port, 48110 is used
const dgram = require('dgram');
const udpSocket = dgram.createSocket('udp4');

// Bind the socket to the loopback address/interface
udpSocket.bind({
    address: '127.0.0.1',
    port: 48110
});

// This callback is called when the UDP socket starts "listening" for new packets
udpSocket.on('listening', () => {
    const address = udpSocket.address();
    var bindaddr = address.address;
    var bindport = address.port;
    console.log('VehicleVisualizer: UDP connection ready at %s:%s',bindaddr,bindport);
});

// map draw message container -> this variable will contain a copy of the "map draw" message received by ms-van3t at the beginning
// of the simulation/emulation session
// It is needed, as, when a new client connects (and it can connect in any moment), the first message which should be sent
// is the "map" one, to let it render the map centered at the proper coordinates
// This message should indeed be received by the client before attempting to render any other moving object
var mapmsg = null;

// This callback is the most important one, as it is called every time a new UDP packet is received from ms-van3t
// As a new packet is received, its content is forwarded to the client (i.e. the browser) via socket.io
udpSocket.on('message', (msg,rinfo) => {
    // console.log('I have received from %s:%s the message: %s',rinfo.address,rinfo.port,msg);

    let msg_fields = msg.toString().split(",");

    // If a "map" initial message is received, and the content appears to be correct, save it inside "mapmsg"
    if(msg_fields[0] === "map") {
        if (msg_fields.length !== 3) {
            console.error("Error: received a corrupted map draw message from ms-van3t.");
            process.exit(1);
        } else if(msg_fields.length === 3 && mapmsg != null) {
            console.error("Error: received twice a map draw message from ms-van3t. This is not allowed");
            process.exit(1);
        } else {
            console.log("VehicleVisualizer: Map draw message received from ms-van3t.");
            mapmsg = msg.toString() + "," + mapbox_token;
        }
    // If a "terminate" message is received from ms-van3t, just close the server
    } else if(msg_fields[0] === "terminate") {
        // This message is sent to terminate the Node.js server
        console.log("VehicleVisualizer: The server received a terminate message. The execution will be terminated.");
        process.exit(0);
    } else {
    // Otherwise, forward all the other messages to the client via socket.io
        io.sockets.send(msg.toString());
    }
});

// Initialize socket.io
const io = require('socket.io')(http);

// socket.io connection callback (called every time a client connects)
io.on('connection', (socket) => {
    console.log('VehicleVisualizer: A user is connected to the web interface');

    // As soon as a client connects, send the "map" message, in order to make it correctly render the base map
    io.sockets.send(mapmsg);

    // socket.io message callback (called every time a client sends something to the server - it should
    // never be called in this web application)
    socket.on('message', (msg) => {
    });
});

// Start the HTTP server on the specified port (i.e. on port server_argv[0])
http.listen(server_argv[0], () => {
    console.log("VehicleVisualizer: Listening on *:" + server_argv[0]);
});
