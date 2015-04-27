var http = require('http');

http.createServer(function (req, res) {
    res.writeHead(200, {'Content-Type': 'text/plain'});
    res.end('Hello Lamobo-D1!\n');
}).listen(8080, '0.0.0.0');

console.log('Server running at http://0.0.0.0:8080/');

