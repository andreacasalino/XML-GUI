var addon = require('./JS-xml-addon/build/Release/xml-addon.node');

const http = require('http')
const fs = require('fs')

let XMLparser = new addon.xmlJS();

const server = http.createServer((req, res) => {
    console.log("request url: " + req.url);
    if(req.url.localeCompare('/') === 0){
        res.writeHead(200, { 'content-type': 'text/html' });
        fs.createReadStream('XML-GUI.html').pipe(res);
        console.log("");
        return;
    }
    if(req.url.localeCompare('/favicon.ico') === 0){
        console.log("");
        return;
    }

    function process(onLoaded){
        let body = [];
        req.on('data', (chunk) => {
            body.push(chunk);
        }).on('end', () => {
            body = Buffer.concat(body).toString();
            const resBody = onLoaded(body); 
            if(resBody === null) {
                res.writeHead(404);
                return;
            }
            res.writeHead(200, { 'content-type': 'text' });
            res.write(resBody);
            res.end();
        });
    }

    // check that is a file to be served
    function getFileExtension(){
        let dotPosition = null;
        for(let p=0; p<(req.url.length-1); ++p) {
            if(req.url[p] === '.') dotPosition = p; 
        }    
        if(dotPosition === null) return null;
        return req.url.slice(dotPosition + 1)
    }
    const extension = getFileExtension();
    if(extension !== null){
        let contentType = 'text/' + extension; 
        if(extension === 'svg'){
            contentType = 'image/svg+xml';
        }
        res.writeHead(200, { 'content-type': contentType  });
        console.log("serving static file of content type" + contentType);
        console.log("");
        fs.createReadStream("." + req.url).pipe(res);
        return;
    }

    process((request)=>{
        console.log("respond to GET request");
        console.log(request);
        vals = JSON.parse(request);
        console.log("req bodies:");
        console.log(vals);

        function logAndReturn(result){
            console.log("result: " + result);
            console.log("");
            return result;
        }

        if(vals.length == 0){
            return logAndReturn(XMLparser.ProcessRequest(req.url));
        }
        else if(vals.length == 1){
            return logAndReturn(XMLparser.ProcessRequest(req.url, vals[0]));
        }
        else if(vals.length == 2){
            return logAndReturn(XMLparser.ProcessRequest(req.url, vals[0], vals[1]));
        }
        else {
            console.log("too many arguments in request: ");
            console.log("");
            return null;
        }
    });
})

server.listen(process.env.PORT || 3000);
