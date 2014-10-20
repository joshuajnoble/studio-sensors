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
  else {
    var q = "set session time zone '-07';";
    client.query(q,  function(err, result) {
        if(err){
          console.log(" error " + err);
        }
        if(result) {
          console.log("timezone set ");
        }
      });
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
        var query = "INSERT INTO readings (sensor_id, studio, zone, time, light, sound, temp, movement, humidity, brightness) VALUES (" + 
          parseInt(q.i,10) + ","  + q.studio + "," + q.zone + ",current_timestamp," + parseInt(q.l,10) + "," + parseInt(q.s,10) + "," + parseInt(q.t,10) + "," + 
          parseInt(q.m,10) + "," + parseInt(q.h,10) +",0);";

       console.log(query);
       client.query(query,  function(err, result) {
          if(err)
          {
            console.log(" error " + err );
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" DB error ");
            return res.end();
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
  else if( uri == "/view")
  {
      var q = querystring.parse(url.parse(req.url).query);
      if(q && q.begintime && q.endtime) 
      {
        var query = "select * from readings where time BETWEEN to_timestamp('"+q.begindate+ " " + q.begintime +
          "', 'DDMMYYYY HH24MI') AND to_timestamp('"+q.enddate+" " +q.endtime + "', 'DDMMYYYY HH24MI')";
        client.query(query,  function(err, result) {
          if(err){
            console.log(" error " + err);
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" error " + err);
            return res.end();
          }
          if(result) {
            var json = JSON.stringify(result.rows);
            res.writeHead(200, {'content-type':'application/json', 'content-length':json.length}); 
            res.end(json); 
          }
        });
      }
      if(q && q.zone && !q.last) 
      {
        var query = "select * from readings where zone = "+q.zone;
        client.query(query,  function(err, result) {
          if(err){
            console.log(" error " + err);
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" error " + err);
            return res.end();
          }
          if(result) {
            var json = JSON.stringify(result.rows);
            res.writeHead(200, {'content-type':'application/json', 'content-length':json.length}); 
            res.end(json);
          }
        });
      }
      if(q && q.zone && q.last) 
      {
        var query = "select * from readings where zone = " + q.zone + " order by time desc limit 1";
        client.query(query,  function(err, result) {
          if(err){
            console.log(" error " + err);
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" error " + err);
            return res.end();
          }
          if(result) {
            var json = JSON.stringify(result.rows);
            res.writeHead(200, {'content-type':'application/json', 'content-length':json.length}); 
            res.end(json);
          }
        });
      }
      if( q && q.sensorid ) 
      {
        var query = "select * from readings where sensor_id = " + q.sensorid;
        client.query(query,  function(err, result) {
          if(err){
            console.log(" error " + err);
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" error " + err);
            return res.end();
          }
          if(result) {
            var json = JSON.stringify(result.rows);
            res.writeHead(200, {'content-type':'application/json', 'content-length':json.length}); 
            res.end(json);
          }
        });
      }
      if(q && q.recent)
      {
          var query = "select * from readings where zone = " + q.recent + " and time > (now() - interval '1 hour');"
          client.query(query,  function(err, result) {
          if(err){
            console.log(" error " + err);
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" error " + err);
            return res.end();
          }
          if(result) {
            var json = JSON.stringify(result.rows);
            res.writeHead(200, {'content-type':'application/json', 'content-length':json.length});
            res.end(json);
          }
        });
      }
      if(q && q.studio && q.begindate && q.begintime)
      {
          var query = "select * from readings where studio = " + q.studio + 
            " AND BETWEEN to_timestamp('"+q.begindate+ " " + q.begintime +"', 'DDMMYYYY HH24MI') AND to_timestamp('"+
              q.enddate+" " +q.endtime + "', 'DDMMYYYY HH24MI');"
          client.query(query,  function(err, result) {
          if(err){
            console.log(" error " + err);
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" error " + err);
            return res.end();
          }
          if(result) {
            var json = JSON.stringify(result.rows);
            res.writeHead(200, {'content-type':'application/json', 'content-length':json.length});
            res.end(json);
          }
        });
      }
      else if(q && q.now)
      {
          var query = "select * from readings where zone = " + q.now + " and time > (now() - interval '2 minute');"
          client.query(query,  function(err, result) {
          if(err){
            console.log(" error " + err);
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" error " + err);
            return res.end();
          }
          if(result) {
            var json = JSON.stringify(result.rows);
            res.writeHead(200, {'content-type':'application/json', 'content-length':json.length});
            res.end(json);
          }
        });
      } 
      else if(q && q.last)
      {
          var query = "SELECT * from readings where zone=" + q.last + " order by time desc limit 1;";
          client.query(query,  function(err, result) {
          if(err){
            console.log(" error " + err);
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" error " + err);
            return res.end();
          }
          if(result) {
            var json = JSON.stringify(result.rows);
            res.writeHead(200, {'content-type':'application/json', 'content-length':json.length});
            res.end(json);
          }
        });
      }
  }
  else if( uri == "/get_id" ) // first thing is to get an ID to use, this lets you know what ID this device is
  {

    var q = querystring.parse(url.parse(req.url).query);
    if( q.ip && q.studio && q.zone )
    {
      var query = "SELECT id FROM sensors ORDER BY id DESC LIMIT 1;";
      client.query(query, function( err, result )
      {
         if(err)
         {
           console.log(" error " + err );
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" DB error ");
            return res.end();
         }
         if( result )
         {
            res.writeHead(200, {'Content-Type': 'text/plain'});
            var id = result.rows[0];
            console.log(" returning id " + id );
            console.log( result );
            var json = JSON.stringify(id);
            res.write( (id.zone + 1).toString()); // everybody gets a new ID
            //now actually write the record
            var updateQuery = "INSERT INTO sensors(id, studio, zone, ip) "+id+""+q.studio+""+q.zone+""+q.ip+";";
            client.query(updateQuery, function( err2, result2 )
            {
               if(err2)
               {
                  console.log(" error " + err );
                  res.writeHead(409, {'Content-Type': 'text/plain'});
                  res.write(" DB error on updateQuery ");
                  return res.end();
               }
               if( result2 )
               {
                  return;
               }
             });

            return res.end();
         }
      });
    }
    else
    {
      res.writeHead(409, {'Content-Type': 'text/plain'});
      res.write("Needs ip, studio, && zone");
      return res.end();
    }
  }
  else if( uri == "/update_ip" ) // update the IP when something reconnects (going to happen *a lot*)
  {

    var q = querystring.parse(url.parse(req.url).query);
    if( q.id && q.ip)
    {
      //UPDATE films SET kind = 'Dramatic' WHERE kind = 'Drama';
      var query = "UPDATE sensors SET ip = " + q.ip + " WHERE id = " + q.id + ";";
      client.query(query, function( err, result )
      {
         if(err)
         {
            console.log(" error " + err );
            res.writeHead(409, {'Content-Type': 'text/plain'});
            res.write(" DB error ");
            return res.end();
         }
         if( result )
         {
            res.writeHead(200, {'Content-Type': 'text/plain'});
            res.write("success"); // it's okd
            return res.end();
         }
      });
    }
    else
    {
      res.writeHead(409, {'Content-Type': 'text/plain'});
      res.write("Needs machine id");
      return res.end();
    }
  }
  else
  {

    // post the data
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
