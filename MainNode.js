var addon = require('./JS-xml-addon/build/Release/....');

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

    if(req.url.localeCompare('/import') === 0){
        process((body)=>{
            res = XMLparser.Import(body);
        });
    }
    else if(req.url.localeCompare('/export') === 0){
        process((body)=>{
            XMLparser.Export(body);
        });
    }
})

server.listen(process.env.PORT || 3000);
