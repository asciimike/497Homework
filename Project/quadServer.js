#!/usr/bin/node
// From Getting Started With node.js and socket.io 
// http://codehenge.net/blog/2011/12/getting-started-with-node-js-and-socket-io-v0-7-part-2/
// This is a general server for the various web frontends
"use strict";

var port = 1337, // Port to listen on
    http = require('http'),
    url = require('url'),
    fs = require('fs'),
    b = require('bonescript'),
    child_process = require('child_process'),
    server,
    connectCount = 0,	// Number of connections to server
    errCount = 0;	// Counts the AIN errors.
    

// Initialize various IO things.
function initIO() {
}

function send404(res) {
    res.writeHead(404);
    res.write('404');
    res.end();
}

initIO();

server = http.createServer(function (req, res) {
// server code
    var path = url.parse(req.url).pathname;
    console.log("path: " + path);
    if (path === '/') {
        path = '/quadServer.html';
    }

    fs.readFile(__dirname + path, function (err, data) {
        if (err) {return send404(res); }
//            console.log("path2: " + path);
        res.write(data, 'utf8');
        res.end();
    });
});

server.listen(port);
console.log("Listening on " + port);

// socket.io, I choose you
var io = require('socket.io').listen(server);
io.set('log level', 2);

// See https://github.com/LearnBoost/socket.io/wiki/Exposed-events
// for Exposed events

// on a 'connection' event
io.sockets.on('connection', function (socket) {

    console.log("Connection " + socket.id + " accepted.");

    socket.on('disconnect', function () {
        console.log("Connection " + socket.id + " terminated.");
        connectCount--;
        console.log("connectCount = " + connectCount);
    });

    connectCount++;
    console.log("connectCount = " + connectCount);
});
