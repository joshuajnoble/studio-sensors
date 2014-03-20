var pg = require('pg')
  , app = require('http').createServer(handler)
  , url = require('url')
  , path = require('path')
  , fs = require('fs')
  , querystring = require('querystring'); 
//or native libpq bindings
//var pg = require('pg').native

var conString = "postgres://postgres:godzilla1@localhost:5432/sensordata";

var mimeTypes = {
    "html": "text/html",
    "jpeg": "image/jpeg",
    "jpg": "image/jpeg",
    "png": "image/png",
    "js": "text/javascript",
    "css": "text/css"};

var client = new pg.Client(conString);
client.connect(function(err) {
  if(err) {
    return console.error('could not connect to postgres', err);
  }
});

app.listen(3000);

function handler (req, res) {

  req.on('error', function (err) {
    console.log(" this is stupid ");
  });

  var uri = url.parse(req.url).pathname;
  
  console.log(uri);
  
  if(uri == '/') 
  {
    var q = querystring.parse(url.parse(req.url).query);
    if(q && q.i)
    {
      console.log(q);
      var query = "INSERT INTO readings (sensor_id, time, light, sound, temp, movement, humidity, brightness) VALUES (" + parseInt(q.i,16) + ",current_timestamp," + parseInt(q.l,16) + "," + parseInt(q.s,16) + "," + parseInt(q.t,16) + "," + parseInt(q.m,16) + "," + parseInt(q.h,16) +",0);";

     console.log(query);
     client.query(query,  function(err, result) {
        if(err){
          return console.log(" error " + err);
        }
        if(result) {
            res.writeHead(200, {'Content-Type': 'text/plain'});
            res.write('done');
            return res.end();
        }
        });
    } else {
      return res.end();
    }
  }
  else {

    var filename = path.join(process.cwd(), uri);

    fs.exists(filename, function(exists) {
        if(!exists) {
            // console.log("not exists: " + filename);
            res.writeHead(200, {'Content-Type': 'text/plain'});
            res.write('404 Not Found\n');
            res.end();
            return;
        }
        var mimeType = mimeTypes[path.extname(filename).split(".")[1]];
        res.writeHead(200, mimeType);

        var fileStream = fs.createReadStream(filename);
        fileStream.pipe(res);

    }); //end path.exists
}
 
}


