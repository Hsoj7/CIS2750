'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
// const ref = require('ref');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

const mysql = require('mysql2/promise');
let connection;
let userName = [];
let password = [];
let dbName = [];

let sharedLib = ffi.Library('./libgpxparse', {
  'parseGPX': [ 'string', ['string'] ],		//return type first, argument list second
  'parseRoute': [ 'string', ['string'] ],
  'parseTrack': [ 'string', ['string'] ],
  'writeWrapper': [ 'int', ['string','float','string'] ],
  'addRouteWrapper': [ 'int', ['string','string'] ],
  'addWaypointWrapper': [ 'void', ['string','string','float','float','int','int'] ],
  'validateUploads': [ 'int', ['string'] ],
  'findBetweenRouteWrapper': [ 'string', ['string','float','float','float','float','int'] ],
  'findBetweenTrackWrapper': [ 'string', ['string','float','float','float','float','int'] ],
  'renameRoute': [ 'int', ['string','string','string'] ],
  'renameTrack': [ 'int', ['string','string','string'] ],
  'routeAttrListToJSON': [ 'string', ['string','string'] ],
  'trackAttrListToJSON': [ 'string', ['string','string'] ],
  'wptTableInfoJson': [ 'string', ['string','int'] ],
});

// let sharedLib = ffi.Library(__dirname + '/parser/bin/libgpxparse', {
//   'parseGPX': [ 'char *', ['char *'] ],		//return type first, argument list second
// });

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(req.files.myfile == null) {
    return res.status(400).send('Error 400. No files were uploaded.');
  }

  let uploadFile = req.files.myfile;
  console.log("Upload file: " + uploadFile.name);

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });

});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(req.params.name);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      //fucking up here
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//returns a list of all files in ./uploads
app.get('/filesOnServer', function(req , res){
  let folder = path.join(__dirname+'/uploads/');
  let myPath = folder;
  //console.log(myPath);
  fs.readdir(myPath, function(err, files) {
    if(files[0] == null){
      return res.send({
        returnData: ""
      })
    }
    let filesString = files[0] + " ";
    for(var i=1; i<files.length; i++) {
      let validate = sharedLib.validateUploads(files[i]);
      if(validate == 1){
        filesString += files[i] + " ";
      }
      else{
        console.log("\nInvalid file upload\n");
        fs.unlinkSync("./uploads/"+files[i]);
      }
    }
    console.log("Files on server: " + filesString);
    res.send({
      returnData: filesString
    })
  });
});

//returns object of all needed elements in file log table
app.get('/FLtableElements', function(req , res){
  let name = req.query.fileName;
  // console.log("File name passed to server: " + name);
  // let returnValue = sharedLib.parseGPX(name);
  // console.log(returnValue);

  let FLTableString = sharedLib.parseGPX(name);
  // console.log("JSON String of file: " + FLTableString);

  res.send({
    returnData: FLTableString
  })
});

//returns objects of needed track and route elements
app.get('/GPXViewElements', function(req , res){
  let name = req.query.fileName;
  // console.log("Creating GPX View for file: " + name);
  let GPXViewTableRouteString = sharedLib.parseRoute(name);
  // console.log("JSON String of Routes: " + GPXViewTableRouteString);
  let GPXViewTableTrackString = sharedLib.parseTrack(name);
  // console.log("JSON String of Tracks: " + GPXViewTableTrackString);

  res.send({
    returnData: {
      route: GPXViewTableRouteString,
      track: GPXViewTableTrackString
    },
  })
});

//creates a new, empty gpx doc
app.get('/createGPX', function(req, res) {
  // console.log("NAME = " + req.query.filename);
  // console.log("NAME = " + req.query.version);
  // console.log("NAME = " + req.query.creator);
  let myBool = sharedLib.writeWrapper(req.query.filename, req.query.version, req.query.creator);
  // console.log("Bool = "+myBool);
  if(myBool == 1){
    console.log("Success creating user file ");
  }
  else if(myBool == 0){
    console.log("Failure creating user file");
  }

  //FIGURE OUT HOW TO RETURN

  return res.send({
    returnData: "success"
  })
});

//adds route to a gpx doc
app.get('/addRoute', function(req, res) {
  let wptList = new Array();
  wptList = req.query.wptObj;
  let routeName = req.query.rteName;
  let myNumWpts = 0;
  myNumWpts = req.query.numWpts;
  let fileName = wptList[0];
  let i = 0;
  // console.log(numWpts);

  let addRouteReturn = sharedLib.addRouteWrapper(fileName, routeName);
  // console.log("RETURNN = " + addRouteReturn);
  if(addRouteReturn != 0 && addRouteReturn != 2){
    // console.log("Num waypoints: " + myNumWpts);
    // let localNumWpts = JSON.parse(JSON.stringify(numWpts));
    for(i = 1; i < myNumWpts + 1; i++){
      //call c addWaypointWrapper function
      if(wptList[i] == null) break;

      let myObj = wptList[i];
      // console.log(myObj);
      // console.log("Wpt name: " + myObj.name);
      // console.log("Wpt lat: " + myObj.lat);
      // console.log("Wpt lon: " + myObj.lon);

      sharedLib.addWaypointWrapper(fileName, myObj.name, myObj.lat, myObj.lon, myNumWpts, i);
    }
  }
  else{
    console.log("ADD ROUTE FAILED");
  }

  return res.send({
    returnData: "success"
  })
});

//finds if specified lats and lons match in a given file
app.get('/findBetweenRoute', function(req, res) {
  let file = req.query.file;
  // let numFiles = req.query.numFiles;
  let srcLat = req.query.srcLat;
  let srcLon = req.query.srcLon;
  let destLat = req.query.destLat;
  let destLon = req.query.destLon;
  let delta = req.query.delta;
  // console.log("Array = "+arrayOfFiles+"numFiles = "+numFiles);

  let rteTrkJSON = sharedLib.findBetweenRouteWrapper(file, srcLat, srcLon, destLat, destLon, delta);
  // console.log("string = "+rteTrkJSON);

  return res.send({
    returnData: rteTrkJSON,
    file: file
  })
});

//finds if specified lats and lons match in a given file
app.get('/findBetweenTrack', function(req, res) {
  let file = req.query.file;
  // let numFiles = req.query.numFiles;
  let srcLat = req.query.srcLat;
  let srcLon = req.query.srcLon;
  let destLat = req.query.destLat;
  let destLon = req.query.destLon;
  let delta = req.query.delta;
  // console.log("Array = "+arrayOfFiles+"numFiles = "+numFiles);

  let rteTrkJSON = sharedLib.findBetweenTrackWrapper(file, srcLat, srcLon, destLat, destLon, delta);
  // console.log("track string = "+rteTrkJSON);

  return res.send({
    returnData: rteTrkJSON,
    file: file
  })
});

//function to rename track or route name for given file
app.get('/rename', function(req, res) {
  let oldName = req.query.oldName;
  let newName = req.query.newName;
  let file = req.query.file;
  let type = req.query.type;

  if(type.toString().localeCompare("Route") == 0){
    // console.log("Type = Route");
    // console.log("Oldname = "+oldName);
    // console.log("New name = "+newName);
    // console.log("File = "+file);
    let myBool = sharedLib.renameRoute(file, oldName, newName);

    return res.send({
      returnData: myBool,
    })
  }
  else if(type.toString().localeCompare("Track") == 0){
    // console.log("Type = Track");
    // console.log("Oldname = "+oldName);
    // console.log("New name = "+newName);
    // console.log("File = "+file);
    let myBool = sharedLib.renameTrack(file, oldName, newName);
    return res.send({
      returnData: myBool,
    })
  }
});

//Function to get the attribute list of a route with specified name
app.get('/routeAttributes', function(req, res) {
  let routeName = req.query.routeName;
  let file = req.query.file;
  let JSONStringAttrs = sharedLib.routeAttrListToJSON(file, routeName);

  return res.send({
    returnData: JSONStringAttrs,
  })
});

//function to get the attributes list of a track with specified name
app.get('/trackAttributes', function(req, res) {
  let trackName = req.query.trackName;
  let file = req.query.file;
  let JSONStringAttrs = sharedLib.trackAttrListToJSON(file, trackName);

  return res.send({
    returnData: JSONStringAttrs,
  })
});

//endpoint to get necessary info about waypoints for the database
app.get('/wptValues', function(req, res) {
  let file = req.query.file;
  let counter = req.query.j;

  let JSONStringWpt = sharedLib.wptTableInfoJson(file, counter);

  return res.send({
    returnData: JSONStringWpt,
  })
});

//Data base connection function + create any non existing tables
app.get('/DBConnect', async function(req, res) {
  // console.log(req.query.userName + req.query.password);
  userName = req.query.userName;
  password = req.query.password;
  dbName = req.query.DBName;

  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
    console.log("Query error: "+e);
    console.log("Closed connection");
    if(connection && connection.end) connection.end();
    return res.send({
      returnData: 0,
    })
  }

  //Create FILE table if not there yet
  try{
    let i = 0;
    const [rows1, fields1] = await connection.execute('select TABLE_NAME from INFORMATION_SCHEMA.TABLES where TABLE_NAME = "FILE"');
    for (let row of rows1){
      i++;
    }
    if(i == 0){
      try{
        let FILETable = "CREATE TABLE FILE (gpx_id INT AUTO_INCREMENT PRIMARY KEY, file_name VARCHAR(60) NOT NULL, ver DECIMAL(2,1) NOT NULL, creator VARCHAR(256) NOT NULL);";
        await connection.execute(FILETable);
      }
      catch(e){
        console.log("Query error: "+e);
      }
    }
  }catch(e){
    console.log("Query error: "+e);
  }
  //Create ROUTE table if not there yet
  try{
    let i = 0;
    const [rows1, fields1] = await connection.execute('select TABLE_NAME from INFORMATION_SCHEMA.TABLES where TABLE_NAME = "ROUTE"');
    for (let row of rows1){
      i++;
    }
    if(i == 0){
      try{
        let ROUTETable = "CREATE TABLE ROUTE (route_id INT AUTO_INCREMENT PRIMARY KEY, route_name VARCHAR(256), route_len FLOAT(15,7) NOT NULL, gpx_id INT NOT NULL, FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE);";
        await connection.execute(ROUTETable);
      }
      catch(e){
        console.log("Query error: "+e);
      }
    }
  }catch(e){
    console.log("Query error: "+e);
  }
  //Create POINT table if not there yet
  try{
    let i = 0;
    const [rows1, fields1] = await connection.execute('select TABLE_NAME from INFORMATION_SCHEMA.TABLES where TABLE_NAME = "POINT"');
    for (let row of rows1){
      i++;
    }
    if(i == 0){
      try{
        let POINTTable = "CREATE TABLE POINT(point_id INT AUTO_INCREMENT PRIMARY KEY, point_index INT NOT NULL, latitude DECIMAL(11,7) NOT NULL, longitude DECIMAL(11,7) NOT NULL, point_name VARCHAR(256), route_id INT NOT NULL, FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE);";
        await connection.execute(POINTTable);
      }
      catch(e){
        console.log("Query error: "+e);
      }
    }
  }catch(e){
    console.log("Query error: "+e);
  }

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: 1,
  })
});

//endpoint to clear all tables. This gets called before we store all. Easier to do this instead of checking for existing values
app.get('/clearDB', async function(req, res) {
  //need to connection everytime we query the DB
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    });
  }catch(e){
    console.log("Query error: "+e);
    console.log("Closed connection");
    if(connection && connection.end) connection.end();
    return res.send({
      returnData: 0,
    })
  }

  try{
    await connection.execute("DELETE from POINT;");
  }
  catch(e){
    console.log("Query error: "+e);
  }
  try{
    await connection.execute("DELETE from ROUTE;");
  }
  catch(e){
    console.log("Query error: "+e);
  }
  try{
    await connection.execute("DELETE from FILE;");
  }
  catch(e){
    console.log("Query error: "+e);
  }

  try{
    await connection.execute("ALTER TABLE FILE AUTO_INCREMENT = 0;");
  }
  catch(e){
    console.log("Query error: "+e);
  }
  try{
    await connection.execute("ALTER TABLE ROUTE AUTO_INCREMENT = 0;");
  }
  catch(e){
    console.log("Query error: "+e);
  }
  try{
    await connection.execute("ALTER TABLE POINT AUTO_INCREMENT = 0;");
  }
  catch(e){
    console.log("Query error: "+e);
  }

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: 1,
  })
});

//function to store all file contents in the FILE table
app.get('/DBStoreFile', async function(req, res) {
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
      console.log("Query error: "+e);
      console.log("Closed connection");
      if(connection && connection.end) connection.end();
      return res.send({
        returnData: 0,
      })
    }

  let fileObj = req.query.fileObj;
  let fileName = req.query.fileName;
  // console.log("Version = "+fileObj.version+" Creator = "+fileObj.creator+" Filename = "+fileName);
  try{
    await connection.execute('insert into FILE values ("'+null+'","'+fileName+'","'+fileObj.version+'","'+fileObj.creator+'")');
  }
  catch(e){
    console.log("Inesrt FILE error: "+e);
    return res.send({
      returnData: 0,
    })
  }

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: 1,
  })
});

//endpoint to store routes from given file
app.get('/DBStoreRoute', async function(req, res) {
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
      console.log("Query error: "+e);
      console.log("Closed connection");
      if(connection && connection.end) connection.end();
      return res.send({
        returnData: 0,
      })
    }

  let routeName = req.query.name;
  let routeLength = req.query.length;
  let gpxID = req.query.gpxID;

  try{
    await connection.execute('insert into ROUTE values ("'+null+'","'+routeName+'","'+routeLength+'","'+gpxID+'")');
  }
  catch(e){
    console.log("Inesrt FILE error: "+e);
    return res.send({
      returnData: 0,
    })
  }

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: 1,
  })
});

//endpoint to store points from given routes in given file
app.get('/DBStorePoint', async function(req, res) {
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
      console.log("Query error: "+e);
      console.log("Closed connection");
      if(connection && connection.end) connection.end();
      return res.send({
        returnData: 0,
      })
    }

    let routeID = req.query.routeID;
    // console.log("ROUTEID = "+routeID);
    let latitude = req.query.latitude;
    let longitude = req.query.longitude;
    let wptName = req.query.wptName;
    let pointIndex = req.query.pointIndex;

    try{
      await connection.execute('insert into POINT values ("'+null+'","'+pointIndex+'","'+latitude+'","'+longitude+'","'+wptName+'","'+routeID+'")');
    }
    catch(e){
      console.log("Inesrt POINT error: "+e);
      return res.send({
        returnData: 0,
      })
    }

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: 1,
  })
});

//endpoint to return information needed to display DB status
app.get('/displayDBStatus', async function(req, res) {
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
      console.log("Query error: "+e);
      console.log("Closed connection");
      if(connection && connection.end) connection.end();
      return res.send({
        returnData: 0,
      })
    }
    // console.log("logged in");
    // console.log(connection);
    // await connection.connect();

    let fileTableLength = 0;
    let routeTableLength = 0;
    let pointTableLength = 0;
    //get count for file tables
    let [rows1, fields1] = await connection.query("SELECT COUNT(*) as num FROM FILE");
    for (let row of rows1){
      fileTableLength = row.num;
    }
    //get count for route table
    [rows1, fields1] = await connection.query("SELECT COUNT(*) as num FROM ROUTE");
    for (let row of rows1){
      routeTableLength = row.num;
    }

    [rows1, fields1] = await connection.query("SELECT COUNT(*) as num FROM POINT");
    for (let row of rows1){
      pointTableLength = row.num;
    }

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: {
      file: fileTableLength,
      route: routeTableLength,
      point: pointTableLength
    }
  })
});

//endpoint to order routes by length
app.get('/displayRouteByLength', async function(req, res) {
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
      console.log("Query error: "+e);
      console.log("Closed connection");
      if(connection && connection.end) connection.end();
      return res.send({
        returnData: 0,
      })
    }

  const [rows2, fields2] = await connection.execute('SELECT * from `ROUTE` ORDER BY `route_len`');

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: rows2,
  })
});

//endpoint to get order of routes by name
app.get('/displayRouteByName', async function(req, res) {
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
      console.log("Query error: "+e);
      console.log("Closed connection");
      if(connection && connection.end) connection.end();
      return res.send({
        returnData: 0,
      })
    }

  const [rows2, fields2] = await connection.execute('SELECT * from `ROUTE` ORDER BY `route_name`');

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: rows2,
  })
});

//endpoint to get everything from file table
app.get('/selectAllFromFILE', async function(req, res) {
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
      console.log("Query error: "+e);
      console.log("Closed connection");
      if(connection && connection.end) connection.end();
      return res.send({
        returnData: 0,
      })
    }

  const [rows2, fields2] = await connection.execute('SELECT * from `FILE` ORDER BY `file_name`');

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: rows2,
  })
});

//end point to get routes from a file ordered by name
app.get('/selectRoutesFromFILEName', async function(req, res) {
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
      console.log("Query error: "+e);
      console.log("Closed connection");
      if(connection && connection.end) connection.end();
      return res.send({
        returnData: 0,
      })
    }

  const [rows2, fields2] = await connection.execute('SELECT * from `ROUTE` where ROUTE.gpx_id = '+req.query.gpxID+' ORDER BY `route_name`');

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: rows2,
  })
});

//end point to get routes from a file ordered by name
app.get('/selectRoutesFromFILELength', async function(req, res) {
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
      console.log("Query error: "+e);
      console.log("Closed connection");
      if(connection && connection.end) connection.end();
      return res.send({
        returnData: 0,
      })
    }

  const [rows2, fields2] = await connection.execute('SELECT * from `ROUTE` where ROUTE.gpx_id = '+req.query.gpxID+' ORDER BY `route_len`');

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: rows2,
  })
});

//endpoint to get all points from a route
app.get('/selectPointsFromRoute', async function(req, res) {
  try{
    connection = await mysql.createConnection({
      host : 'dursley.socs.uoguelph.ca',
      user : userName,
      password : password,
      database : dbName
    })
  }catch(e){
      console.log("Query error: "+e);
      console.log("Closed connection");
      if(connection && connection.end) connection.end();
      return res.send({
        returnData: 0,
      })
    }

  const [rows2, fields2] = await connection.execute('SELECT * from `POINT` where route_id = '+req.query.routeID+' ORDER BY `point_index`');

  if(connection && connection.end) connection.end();
  return res.send({
    returnData: rows2,
  })
});



//******************** Your code goes here ********************
//THIS GETS INVOKED AFTER THE WEBPAGE FINISHES LOADING
//Sample endpoint
app.get('/someendpoint', function(req , res){
  let retStr = req.query.name1 + " " + req.query.name2;
  res.send({
    foo: retStr
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
