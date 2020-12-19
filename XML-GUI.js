var addon = require('./xml-addon/build/Release/xml-addon.node');

const http = require('http')
const fs = require('fs')

let XMLparser = new addon.xmlJS();

const server = http.createServer((req, res) => {
    console.log(req.url);
    if(req.url.localeCompare('/') === 0){
        res.writeHead(200, { 'content-type': 'text/html' });
        fs.createReadStream('index.html').pipe(res);
        return;
    }
    if(req.url.localeCompare('/favicon.ico') === 0){
        return;
    }

    function process(onLoaded){
        let body = [];
        req.on('data', (chunk) => {
            body.push(chunk);
        }).on('end', () => {
            body = Buffer.concat(body).toString();
            res.writeHead(200, { 'content-type': 'text' });
            res.write(onLoaded(body));
            res.end();
        });
    }

    process((request)=>{
        res = XMLparser.ProcessRequest(request);
    });
})

server.listen(process.env.PORT || 3000);
