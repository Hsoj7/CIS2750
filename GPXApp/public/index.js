let waypointList = new Array;
let numWaypoints = 0;
let GPXViewFile = new Array;
let displayRoutesFile = new Array;
let displayAllPointsRoute = new Array;
let displayPointsFile = new Array;

//ready function for file log table
$(document).ready(function() {
  console.log("Finding files on server");
  document.getElementById("storeFiles").disabled = true;
  document.getElementById("clearFiles").disabled = true;
  document.getElementById("displayStatus").disabled = true;
  document.getElementById("routeDisplayBtn").disabled = true;
  document.getElementById("routeDisplaySpecificBtn").disabled = true;
  document.getElementById("query3").disabled = true;
  document.getElementById("pointFromFileBtn").disabled = true;

  $('#loginConfirmation').append("<br><p style='color:#E10E0E;'>Logged out</p>");

  //let x = new FormData();
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/filesOnServer',
        //data: {filename:"simple.gpx"},
        //data: "simple.gpx",
          //JSON.parse("simple.gpx");

        success: function (data) {
          // console.log("Found all files on server =>");
          // console.log(data);
          let string = data.returnData;
          let CSVString = string.split(" ");
          var numFiles = CSVString.length - 1;
          //CSVString is always 1 more than the # of files becuase of how we add spacing in app.js,
          //Thus, we need to do the - 1

          if(string.length > 0){
            if(numFiles > 2){
              document.getElementById("divFileLogTable").style.overflowY = "scroll";
            }

            var i = 0;
            while(i < numFiles){
              let version = 0.0;
              let fileNameString = CSVString[i];
              // console.log("Current File: " + fileNameString);

              var FLTableObject = new Object();
              //CAN I HAVE A FAIL: CLUASE HERE? IT CRASHES IF I DO
              $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/FLtableElements',
                async: false,
                data: {
                  fileName: fileNameString
                },
                success: function (data) {
                  //console.log(data.returnData);
                  FLTableObject = JSON.parse(data.returnData);

                }
              });

              $('#FLBody').append("<tr>");
              $('#FLBody').append("<td><a href='/uploads/"+CSVString[i]+"'>"+CSVString[i]+"</a></td>");
              $('#FLBody').append("<td>" + FLTableObject.version + "</td>");
              $('#FLBody').append("<td>" + FLTableObject.creator + "</td>");
              $('#FLBody').append("<td>" + FLTableObject.numWaypoints + "</td>");
              $('#FLBody').append("<td>" + FLTableObject.numRoutes + "</td>");
              $('#FLBody').append("<td>" + FLTableObject.numTracks + "</td>");
              $('#FLBody').append("</tr>");

              i++;
            }

            // myTable+="</table>";
            // document.getElementById('divFileLogTable').innerHTML = myTable;


            //This is for GPX View Panel drop down list
            let string2 = data.returnData;
            let CSVString2 = string2.split(" ");

            let j = 0;
            while(j < numFiles){
              $('#GPXViewDropList').append("<option value='" + CSVString2[j] + "'>" + CSVString2[j] + "</option>");
              $('#addRouteList').append("<option value='" + CSVString2[j] + "'>" + CSVString2[j] + "</option>");
              j++;
            }

            //initialize the GPX table with document thats first alphabetically
            let obj = {
              value: CSVString2[0]
            }
            gpxDisplay(obj);
            routeChooseFile(obj);

          }
          else{
            $('#FLBody').append("<tr><td>No files found</td></tr>");
            document.getElementById("addWptSubmit").disabled = true;
            document.getElementById("addRteSubmit").disabled = true;
            document.getElementById("findBetween").disabled = true;
            document.getElementById("renameSubmit").disabled = true;
          }
        },
        fail: function(error) {
            console.log("Error finding files on the server");
            //$('#test').html("On page load, received error from server");
            console.log(error);
        }
    });

});
//function for retriving + writeing to GPX View Panel
function gpxDisplay(obj){
  let file = obj.value;

  GPXViewFile = [];
  GPXViewFile = JSON.parse(JSON.stringify(file));

  $('#GPXViewPanelBody').empty();

  $.ajax({
      //async: false,
      type: 'get',
      dataType: 'json',
      url: '/GPXViewElements',
      data: {
        fileName: file
      },
      success: function (data) {
        let RouteListString = data.returnData.route;
        let GPXTableRouteObject = JSON.parse(RouteListString);

        let TrackListString = data.returnData.track;
        let GPXTableTrackObject = JSON.parse(TrackListString);

        if(GPXTableRouteObject.length == 0 && GPXTableTrackObject.length == 0){
          $('#GPXViewPanelBody').append("<tr><td>No data found</td></tr>");
        }

        if(GPXTableRouteObject.length > 0){
          let obj = {
            name: "test"
          }
          let i = 0;
          while(i < GPXTableRouteObject.length){
            $('#GPXViewPanelBody').append("<tr>");
            i++;
            //Component
            $('#GPXViewPanelBody').append("<td>Route " + i + "</td>");
            i--;
            //Name
            $('#GPXViewPanelBody').append("<td>" + GPXTableRouteObject[i].name + "</td>");
            //NumPoints
            $('#GPXViewPanelBody').append("<td>" + GPXTableRouteObject[i].numPoints + "</td>");
            //Length
            $('#GPXViewPanelBody').append("<td>" + GPXTableRouteObject[i].len + "</td>");
            //Loop
            $('#GPXViewPanelBody').append("<td>" + GPXTableRouteObject[i].loop + "</td>");
            //attribute button
            $('#GPXViewPanelBody').append("<td><input type='submit' value='Display' id='displayRoute' style='display:block;margin:auto;' onClick='showAttributesRoute(\""+GPXTableRouteObject[i].name+"\")'></td>");
            //onClick='test1(GPXTableRouteObject[i])'
            $('#GPXViewPanelBody').append("</tr>");
            i++;
          }
        }


        if(GPXTableTrackObject.length > 0){
          let i = 0;
          while(i < GPXTableTrackObject.length){
            $('#GPXViewPanelBody').append("<tr>");
            //Component
            i++;
            $('#GPXViewPanelBody').append("<td>Track " + i + "</td>");
            i--;
            //Name
            $('#GPXViewPanelBody').append("<td>" + GPXTableTrackObject[i].name + "</td>");
            //NumPoints
            $('#GPXViewPanelBody').append("<td>N/A</td>");
            //Length
            $('#GPXViewPanelBody').append("<td>" + GPXTableTrackObject[i].len + "</td>");
            //Loop
            $('#GPXViewPanelBody').append("<td>" + GPXTableTrackObject[i].loop + "</td>");

            //attribute button
            $('#GPXViewPanelBody').append("<td><input type='submit' value='Display' id='displayTrack' style='display:block;margin:auto;' onClick='showAttributesTrack(\""+GPXTableTrackObject[i].name+"\")'></td>");

            $('#GPXViewPanelBody').append("</tr>");
            i++;
          }
        }
      }
    });
}

function showAttributesRoute(obj){
  let routeName = obj;
  let file = JSON.parse(JSON.stringify(GPXViewFile));
  // console.log("RouteName = "+routeName+" file = "+ file);

  $.ajax({
    type: 'get',
    dataType: 'json',
    url: '/routeAttributes',
    data: {
      routeName: routeName,
      file: file,
    },
    success: function (data) {
      let JSONStringAttrs = data.returnData;
      let attrObj = JSON.parse(JSONStringAttrs);
      let i = 0;
      let string = new Array();

      while(attrObj[i] != null){
        // console.log(attrObj[i].name + attrObj[i].value);
        i++;
        string.push("Attribute " + i + ":\n");
        i--;
        string.push("Name: " + attrObj[i].name);
        string.push("\n");
        string.push("Value: " + attrObj[i].value);
        string.push("\n\n");
        i++;
      }
      if(string.length > 0){
        alert(string);
      }
      else{
        alert("No attributes found");
      }
    },
    fail: function(error) {
      console.log(error);
    }
  });
}

function showAttributesTrack(obj){
  let trackName = obj;
  let file = JSON.parse(JSON.stringify(GPXViewFile));
  console.log("RouteName = "+trackName+" file = "+ file);

  $.ajax({
    type: 'get',
    dataType: 'json',
    url: '/trackAttributes',
    data: {
      trackName: trackName,
      file: file,
    },
    success: function (data) {
      let JSONStringAttrs = data.returnData;
      let attrObj = JSON.parse(JSONStringAttrs);
      let i = 0;
      let string = new Array();

      while(attrObj[i] != null){
        // console.log(attrObj[i].name + attrObj[i].value);
        i++;
        string.push("Attribute " + i + ":\n");
        i--;
        string.push("Name: " + attrObj[i].name);
        string.push("\n");
        string.push("Value: " + attrObj[i].value);
        string.push("\n\n");
        i++;
      }
      if(string.length > 0){
        alert(string);
      }
      else{
        alert("No attributes found")
      }
    },
    fail: function(error) {
      console.log(error);
    }
  });
}

function changeRouteFile(obj){
  displayRoutesFile = obj.value;
}

function changePointsRoute(obj){
  displayAllPointsRoute = obj.value;
}

function changePointsFile(obj){
  displayPointsFile = obj.value;
}

function displayDatabase(){
  $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/displayDBStatus',
      success: function (data) {
        let numFiles = 0;
        let numRoutes = 0;
        let numPoints = 0;
        if(data.returnData == 0){
          console.log("Error connecting");
        }
        else{
          if(data.returnData.file){
            numFiles = data.returnData.file;
          }
          if(data.returnData.route){
            numRoutes = data.returnData.route;
          }
          if(data.returnData.point){
            numPoints = data.returnData.point;
          }

          alert("Database has "+numFiles+" files, "+numRoutes+" routes, and "+numPoints+" points");
        }
      }
    });
}

$('#createGPX').on("submit", function(e) {
  e.preventDefault();
  let versionString = document.getElementById("CGPXVersion").value;
  // console.log("Version = " + versionString.value);
  let creatorString = document.getElementById("CGPXCreator").value;
  // console.log("Creator = " + creatorString.value);
  let fileNameString = document.getElementById("CGPXFilename").value;
  // console.log("File name = " + fileNameString.value);

  let string = new Array;
  let csvString = new Array;
  let numFiles = 0;
  let good = 1;
  $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/filesOnServer',
      async: false,
      success: function (data) {
        // console.log(data.returnData);
        string = JSON.parse(JSON.stringify(data.returnData));
        csvString = string.split(" ");
        numFiles = csvString.length - 1;
      },
      fail: function(error) {
        console.log(error);
      }
    });
    // console.log("version = "+versionString);
    // console.log("HERE"+versionString.toString().localeCompare("1.1"));
    if(versionString.toString().localeCompare("1.1") != 0){
      alert("GPX verison must be '1.1'. Other versions are not supported at this time");
      good = 0;
    }
    if(creatorString.length < 1){
      alert("Must include a creator");
      good = 0;
    }
    if(fileNameString.length < 1){
      alert("Must include a file name");
      good = 0;
    }
    let extension = fileNameString.split('.').pop();
    if(extension.toString().localeCompare("gpx") != 0){
      alert("File extension must be '.gpx'");
      good = 0;
    }
    for(let i = 0; i < numFiles; i++){
      if(fileNameString.toString().localeCompare(csvString[i]) == 0){
        alert("File name already exists on server");
        good = 0;
      }
    }

    if(good == 1){
      $.ajax({
          type: 'get',
          dataType: 'json',
          url: '/createGPX',
          data: {
            version: versionString,
            creator: creatorString,
            filename: fileNameString
          },
          success: function (data) {
            let string = data.returnData;
            console.log("/createGPX" + string);
            location.reload();
          },
          fail: function(error) {
            console.log(error);
          }
        });
      }

});
//This function puts the choosen file at beginning of waypointList Array
//-That we will later parse
function routeChooseFile(obj){
  waypointList = [];
  waypointList.push(obj.value);
  //if user tried to add any wpts to a file, reset the counter + array from two lines above
  $('#numWpts').empty();
  numWaypoints = 0;

  // let i = 0;
  // for(i = 0; i < waypointList.length; i++){
  //   console.log("array[" + i + "] = " +  waypointList[i]);
  // }
}
//function to add wpts to array of waypoint objects
$('#addWptSubmit').on("click", function(e) {
  e.preventDefault();
  let myWptName = document.getElementById("wptName");
  // console.log("Version = " + versionString.value);
  let myLat = document.getElementById("lat");
  // console.log("Creator = " + creatorString.value);
  let myLon = document.getElementById("lon");
  // console.log(wptName.value);
  // console.log(lat.value);
  // console.log(lon.value);

  if(myWptName.value.length < 1){
    myWptName.value = "";
  }

  if(myLat.value.length < 1){
    alert("Must include latitude");
  }
  else if(myLat.value < -90 || myLat.value > 90){
    alert("Invalid latitude. Range is -90 to +90");
  }
  else if(myLon.value < -180 || myLon.value > 180){
    alert("Invalid longitude. Range is -180 to +180");
  }
  else if(myLon.value.length < 1){
    alert("Must include longitude");
  }
  else{
    let obj = {
      name: myWptName.value,
      lat: myLat.value,
      lon: myLon.value
    }

    waypointList.push(obj);
    //clear numWpts div and replace with current count
    numWaypoints++;
    $('#numWpts').empty();
    $('#numWpts').append("Total wpts: " + (numWaypoints));
  }

});
//function that gets called when hitting submit route
$('#addRteSubmit').on("click", function(e) {
  e.preventDefault();
  let myRouteNameObj = document.getElementById("rteName");
  let myRouteName = myRouteNameObj.value;
  // console.log("Route name: " + myRouteName.value);

  // for (i = 0; i < waypointList.length; i++){
  //   console.log(localWaypointList[i]);
  // }
  // let fileName = JSON.parse(JSON.stringify(waypointList[0]));
  // console.log("File name = " + fileName);

  let testObj = new Object;
  testObj = waypointList[1];

  if(testObj == null){
    alert("Must add a waypoint to your route");
  }
  else{
    let localNumWpts = numWaypoints;
    // let fileName = waypointList[0];
    // console.log("Length = " + waypointList.length);
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/addRoute',
        async: false,
        data: {
          wptObj: waypointList,
          rteName: myRouteName,
          numWpts: localNumWpts
        },
          success: function (data) {
            //clear wpt name, lat, lon boxes, clear wpt counter, clear route name box
            location.reload();
          },
          fail: function(error) {
            // console.log(error);
          }
      });
  }
  // location.reload(); force refresh
});

$('#findBetween').on("click", function(e) {
  e.preventDefault();

  $('#betweenTableBody').empty();

  let srcLat = document.getElementById("latSrc").value;
  let srcLon = document.getElementById("lonSrc").value;
  let destLat = document.getElementById("latDest").value;
  let destLon = document.getElementById("lonDest").value;
  let delta = document.getElementById("delta").value;

  if(srcLat < -90 || srcLat > 90){
    alert("Invalid starting latitude. Range is -90 to +90");
  }
  else if(srcLon < -180 || srcLon > 180){
    alert("Invalid starting longitude. Range is -180 to +180");
  }
  else if(destLat < -90 || destLat > 90){
    alert("Invalid ending latitude. Range is -90 to +90");
  }
  else if(destLon < -180 || destLon > 180){
    alert("Invalid ending longitude. Range is -180 to +180");
  }
  else if(delta < 0){
    alert("Delta must be positive");
  }
  else if(delta > 40075000){
    alert("Earth's circumference is roughly 40,075,000m. Delta too large.");
  }
  else{
    let string = new Array;
    let csvString = new Array;
    let numFiles = 0;
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/filesOnServer',
      async: false,
      success: function (data) {
        // console.log(data.returnData);
        string = JSON.parse(JSON.stringify(data.returnData));
        csvString = string.split(" ");
        numFiles = csvString.length - 1;
      },
      fail: function(error) {
        console.log(error);
      }
    });
    // console.log("string "+string+"CSV = "+csvString+"numFiles " +  numFiles);
    if(numFiles > 0){
      let i = 0;
      while(i < numFiles){
        $.ajax({
          type: 'get',
          dataType: 'json',
          url: '/findBetweenRoute',
          async: false,
          data: {
            file: csvString[i],
            srcLat: srcLat,
            srcLon: srcLon,
            destLat: destLat,
            destLon: destLon,
            delta: delta
          },
          success: function (data) {
            // console.log("Success /findBetween " + data.returnData);
            let obj = JSON.parse(data.returnData);
            let file = data.file
            if(obj == null){
              // $('#betweenTableBody').empty();
              // $('#betweenTableBody').append("<tr><td>No data found</td></tr>");
            }
            else{
              let j = 0;
              while(j < obj.length){
                $('#betweenTableBody').append("<tr>");

                $('#betweenTableBody').append("<td>"+file+"</td>");
                $('#betweenTableBody').append("<td>Route</td>");
                $('#betweenTableBody').append("<td>"+obj[j].name+"</td>");
                $('#betweenTableBody').append("<td>"+obj[j].numPoints+"</td>");
                $('#betweenTableBody').append("<td>"+obj[j].len+"</td>");
                $('#betweenTableBody').append("<td>"+obj[j].loop+"</td>");
                $('#betweenTableBody').append("<td>N/A</td>");

                $('#betweenTableBody').append("</tr>");
                j++;
              }
            }
          },
          fail: function(error) {
            console.log(error);
          }
        });

        $.ajax({
          type: 'get',
          dataType: 'json',
          url: '/findBetweenTrack',
          async: false,
          data: {
            file: csvString[i],
            srcLat: srcLat,
            srcLon: srcLon,
            destLat: destLat,
            destLon: destLon,
            delta: delta
          },
          success: function (data) {
            let obj = JSON.parse(data.returnData);
            let file = data.file
            if(obj != null){
              let j = 0;
              while(j < obj.length){
                $('#betweenTableBody').append("<tr>");

                $('#betweenTableBody').append("<td>"+file+"</td>");
                $('#betweenTableBody').append("<td>Track</td>");
                $('#betweenTableBody').append("<td>"+obj[j].name+"</td>");
                $('#betweenTableBody').append("<td>N/A</td>");
                $('#betweenTableBody').append("<td>"+obj[j].len+"</td>");
                $('#betweenTableBody').append("<td>"+obj[j].loop+"</td>");
                $('#betweenTableBody').append("<td>N/A</td>");

                $('#betweenTableBody').append("</tr>");
                j++;
              }
            }
          },
          fail: function(error) {
            console.log(error);
          }
        });

        i++;
      }
    }
    else{
      alert("Error, no files on the server to search");
    }

    let x = document.getElementById("betweenTableBody").rows.length;
    if(x == 0){
      $('#betweenTableBody').append("<tr><td>No Data Found</td></td>");
    }
  }
});

$('#renameSubmit').on("click", function(e) {
  e.preventDefault();

  let oldName = document.getElementById("oldName").value;
  let newName = document.getElementById("newName").value;
  let type = [];
  // console.log("Old name = "+oldName);
  // console.log("New name = "+newName);
  // console.log("File name = "+GPXViewFile);
  let i = 0;
  let elements = document.getElementsByName("TR");
  for(i = 0, l = elements.length; i < l; i++){
    if(elements[i].checked){
      type = elements[i].value;
    }
  }

  if(type.length < 1){
    alert("Must specify route or track to rename");
  }
  else if(oldName.length < 1){
    alert("Must include the name you wish to change");
  }
  else if(newName.length < 1){
    alert("Must include the new name");
  }
  else{
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/rename',
      data: {
        oldName: oldName,
        newName: newName,
        file: GPXViewFile,
        type: type
      },
      success: function (data) {
        if(data.returnData == 1){
          console.log("success /rename "+type);
          location.reload();
        }
        else if(data.returnData == 0){
          alert("Could not find "+type+" with name "+oldName)
        }
      },
      fail: function(error) {
        console.log(error);
      }
    });
  }

});

$('#submitLogin').on("click", function(e) {
  e.preventDefault();

  let userName = document.getElementById("userName").value;
  let password = document.getElementById("password").value;
  let DBName = document.getElementById("DBName").value;

  if(userName.length < 1){
    alert("Must enter username");
  }
  else if(password.length < 1){
    alert("Must enter password");
  }
  else if(DBName.length < 1){
    alert("Must enter database name")
  }
  else{
    $.ajax({
      async: false,
      type: 'get',
      dataType: 'json',
      url: '/DBConnect',
      data: {
        userName: userName,
        password: password,
        DBName: DBName
      },
      success: function (data) {
        if(data.returnData == 0){
          alert("Connection failed, re-enter credentials");
        }
        else if(data.returnData == 1){
          console.log("Database connected");
          document.getElementById("storeFiles").disabled = false;
          document.getElementById("clearFiles").disabled = false;
          document.getElementById("displayStatus").disabled = false;
          document.getElementById("routeDisplayBtn").disabled = false;
          document.getElementById("routeDisplaySpecificBtn").disabled = false;
          document.getElementById("query3").disabled = false;
          document.getElementById("pointFromFileBtn").disabled = false;
          document.getElementById("submitLogin").disabled = true;

          $('#loginConfirmation').empty();
          $('#loginConfirmation').append("<br><p style='color:#1aff1a;'>Logged in</p>");

          let empty = 0;

          $.ajax({
              async: false,
              type: 'get',
              dataType: 'json',
              url: '/displayDBStatus',
              success: function (data) {
                empty = data.returnData.file;
              }
            });

            if(empty == 0){
              // alert("Database is currently empty");
            }
            else{
              //server call to get all files in data base, populate routeFromFileList with them
              $.ajax({
                async: false,
                type: 'get',
                dataType: 'json',
                url: '/selectAllFromFILE',
                success: function (data) {
                  let rows = data.returnData;
                  let i = 0;
                  displayRoutesFile = rows[0].file_name;
                  displayPointsFile = rows[0].file_name;
                  while(rows[i]){
                    $('#routeFromFileList').append("<option value='" +rows[i].file_name+ "'>" +rows[i].file_name+ "</option>");
                    $('#pointFromFileList').append("<option value='" +rows[i].file_name+ "'>" +rows[i].file_name+ "</option>");
                    i++;
                  }
                }
              });

              $.ajax({
                async: false,
                type: 'get',
                dataType: 'json',
                url: '/displayRouteByName',
                success: function (data) {
                  let rows = data.returnData;
                  let i = 0;
                  displayAllPointsRoute = rows[0].route_name;
                  while(rows[i]){
                    $('#allPointsList').append("<option value='" +rows[i].route_name+ "'>" +rows[i].route_name+ "</option>");
                    i++;
                  }
                }
              });
            }

            $.ajax({
                async: false,
                type: 'get',
                dataType: 'json',
                url: '/displayDBStatus',
                success: function (data) {
                  let numFiles = 0;
                  let numRoutes = 0;
                  let numPoints = 0;
                  if(data.returnData == 0){
                    console.log("Error connecting");
                  }
                  else{
                    if(data.returnData.file){
                      numFiles = data.returnData.file;
                    }
                    if(data.returnData.route){
                      numRoutes = data.returnData.route;
                    }
                    if(data.returnData.point){
                      numPoints = data.returnData.point;
                    }

                    alert("Connection success\n\nDatabase has "+numFiles+" files, "+numRoutes+" routes, and "+numPoints+" points");
                  }
                }
              });
        } //else success block
      },
      fail: function(error) {
        console.log(error);
      }
    });

    //display databsae status
  }

});

$('#storeFiles').on("click", function(e) {
  e.preventDefault();

  let isFiles = 0;
  $.ajax({
      async: false,
      type: 'get',
      dataType: 'json',
      url: '/filesOnServer',
      success: function (data) {
        if(data.returnData.length > 0){
          isFiles = 1;
        }
      }
    });
    if(isFiles == 0){
      alert("No files on server to upload");
    }
    else{
      //clear the data base, then just upload everything we have. Easier than checking for existing values
      $.ajax({
        async: false,
        type: 'get',
        dataType: 'json',
        url: '/clearDB',
        success: function (data) {
          // console.log("Database cleared");
        }
      });

      let fileList;
      $.ajax({
        async: false,
        type: 'get',
        dataType: 'json',
        url: '/filesOnServer',
        success: function (data) {
          let string = data.returnData;
          let CSVString = string.split(" ");
          var numFiles = CSVString.length - 1;
          let myRouteID = 0;
          // console.log(CSVString);
          let i = 0;
          while(i < numFiles){
            let fileObj;
            //get all things needed for the file to be stored
            $.ajax({
              async: false,
              type: 'get',
              dataType: 'json',
              url: '/FLtableElements',
              data: {
                fileName: CSVString[i]
              },
              success: function (data) {
                fileObj = JSON.parse(data.returnData);
                // console.log(fileObj);

              }
            });
            //store individual files
            $.ajax({
              async: false,
              type: 'get',
              dataType: 'json',
              url: '/DBStoreFile',
              data: {
                fileObj: fileObj,
                fileName: CSVString[i]
              },
              success: function (data) {
                if(data.returnData == 1){
                  console.log("Success file upload");
                }
                else if(data.returnData == 0){
                  console.log("Store file failed");
                }
              }
            });

            //Now fill route table with all routes from file
            let routeObj;
            $.ajax({
              async: false,
              type: 'get',
              dataType: 'json',
              url: '/GPXViewElements',
              data: {
                fileName: CSVString[i]
              },
              success: function (data) {
                routeObj = JSON.parse(data.returnData.route);
                // console.log(routeObj);
              }
            });
            let j = 0; //counter for routes in a file
            while(j < routeObj.length){
              let gpxID = i + 1;
              $.ajax({
                async: false,
                type: 'get',
                dataType: 'json',
                url: '/DBStoreRoute',
                data: {
                  name: routeObj[j].name,
                  length: routeObj[j].len,
                  gpxID: gpxID
                },
                success: function (data) {
                  if(data.returnData == 0){
                    console.log("Error uploading route");
                  }
                  else{
                    console.log("Success Route upload");
                  }
                  myRouteID += 1;
                  //ajax call to get waypoints
                  //Then loop through the waypoints and insert to database
                  //use j as a counter to determine the route we want, just increment
                  $.ajax({
                    async: false,
                    type: 'get',
                    dataType: 'json',
                    url: '/wptValues',
                    data: {
                      file: CSVString[i],
                      j: j
                    },
                    success: function (data) {
                      // console.log("Success got wptListObj")
                      let wptListObj = JSON.parse(data.returnData);
                      //ajax to insert the wptListObj into point table
                      let k = 0;
                      while(k < wptListObj.length){
                        let latitude = wptListObj[k].lat;
                        let longitude = wptListObj[k].lon;
                        let wptName = wptListObj[k].name
                        // console.log("My route id = "+myRouteID);
                        $.ajax({
                          async: false,
                          type: 'get',
                          dataType: 'json',
                          url: '/DBStorePoint',
                          data: {
                            routeID: myRouteID,
                            latitude: latitude,
                            longitude: longitude,
                            wptName: wptName,
                            pointIndex: k,
                          },
                          success: function (data) {
                            if(data.returnData == 0){
                              console.log("Error inserting point");
                            }
                            else{
                              console.log("Success route point upload");
                            }
                          }
                        });
                        k++;
                      }//while loop looping through waypoints
                    }//success of wptValues
                  }); //ajax call wptValues
                } //success of store route
              });//ajax of store route
              j++;
            }//while loop looping through routes
            i++;
          }

          //put update for the drop down shits here
          alert("Store success\n\nMay need page refresh for accurate database results in database searches below");
          displayDatabase();
        },
        fail: function(error){
          console.log("Error getting files on server from #storeFiles.onClick: "+error);
        }
      });


    }//else checking if there are files on server

});

$('#clearFiles').on("click", function(e) {
  e.preventDefault();
  $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/clearDB',
      success: function (data) {
        console.log("Database cleared");
        alert("Database cleared\n\nMay need page refresh for accurate database results in database searches below");
        displayDatabase();
      }
    });

});

$('#displayStatus').on("click", function(e) {
  console.log("Status");
  e.preventDefault();
  $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/displayDBStatus',
      success: function (data) {
        let numFiles = 0;
        let numRoutes = 0;
        let numPoints = 0;
        if(data.returnData == 0){
          console.log("Error connecting");
        }
        else{
          if(data.returnData.file){
            numFiles = data.returnData.file;
          }
          if(data.returnData.route){
            numRoutes = data.returnData.route;
          }
          if(data.returnData.point){
            numPoints = data.returnData.point;
          }

          alert("Database has "+numFiles+" files, "+numRoutes+" routes, and "+numPoints+" points");
        }
      }
    });
});

$('#routeDisplayBtn').on("click", function(e) {
  e.preventDefault();
  let type = [];

  let i = 0;
  let elements = document.getElementsByName("routeDisplay");
  for(i = 0, l = elements.length; i < l; i++){
    if(elements[i].checked){
      type = elements[i].value;
    }
  }
  // console.log("type = "+type);
  if(type.length < 1){
    alert("Must choose to order by length or name");
  }
  else if(type.toString().localeCompare("Length") == 0){
    $('#routeDisplayTableBody').empty();
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/displayRouteByLength',
      success: function (data) {
        // console.log("Success Length");
        let rows = data.returnData;
        let i = 0;
        // console.log(rows);

        $('#routeDisplayTableBody').append("<tr>");
        $('#routeDisplayTableBody').append("<th><b>Route ID</b></th>");
        $('#routeDisplayTableBody').append("<th><b>Route Name</b></th>");
        $('#routeDisplayTableBody').append("<th><b>Route Length</b></th>");
        $('#routeDisplayTableBody').append("<th><b>GPX ID</b></th>");
        $('#routeDisplayTableBody').append("</tr>");

        while(rows[i]){
          $('#routeDisplayTableBody').append("<tr>");
          $('#routeDisplayTableBody').append("<td>"+rows[i].route_id+"</td>");
          $('#routeDisplayTableBody').append("<td>"+rows[i].route_name+"</td>");
          $('#routeDisplayTableBody').append("<td>"+rows[i].route_len+"</td>");
          $('#routeDisplayTableBody').append("<td>"+rows[i].gpx_id+"</td>");
          $('#routeDisplayTableBody').append("</tr>");
          i++;
        }
      }
    });
  }
  else if(type.toString().localeCompare("Name") == 0){
    $('#routeDisplayTableBody').empty();
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/displayRouteByName',
      success: function (data) {
        // console.log("Success Length");
        let rows = data.returnData;
        let i = 0;
        // console.log(rows);

        $('#routeDisplayTableBody').append("<tr>");
        $('#routeDisplayTableBody').append("<th><b>Route ID</b></th>");
        $('#routeDisplayTableBody').append("<th><b>Route Name</b></th>");
        $('#routeDisplayTableBody').append("<th><b>Route Length</b></th>");
        $('#routeDisplayTableBody').append("<th><b>GPX ID</b></th>");
        $('#routeDisplayTableBody').append("</tr>");

        while(rows[i]){
          $('#routeDisplayTableBody').append("<tr>");
          $('#routeDisplayTableBody').append("<td>"+rows[i].route_id+"</td>");
          $('#routeDisplayTableBody').append("<td>"+rows[i].route_name+"</td>");
          $('#routeDisplayTableBody').append("<td>"+rows[i].route_len+"</td>");
          $('#routeDisplayTableBody').append("<td>"+rows[i].gpx_id+"</td>");
          $('#routeDisplayTableBody').append("</tr>");
          i++;
        }
      }
    });
  }
});
//this function might have a problem... Check on filesOnServer function
$('#routeDisplaySpecificBtn').on("click", function(e) {
  e.preventDefault();
  //get radio button type then copy routeDisplayBtn
  let type = [];

  let i = 0;
  let elements = document.getElementsByName("routeDisplaySpecific");
  for(i = 0, l = elements.length; i < l; i++){
    if(elements[i].checked){
      type = elements[i].value;
    }
  }
  // console.log("Type = "+type);
  let string;
  let CSVString;
  let numFiles;
  $.ajax({
      async: false,
      type: 'get',
      dataType: 'json',
      url: '/filesOnServer',
      success: function (data) {
        string = data.returnData;
        CSVString = string.split(" ");
        numFiles = CSVString.length - 1;
      }
    });

    let gpxID = 0;
    i = 0;
    while(CSVString[i].length > 0){
      if(CSVString[i].toString().localeCompare(displayRoutesFile) == 0){
        // console.log("fond, i = "+i);
        gpxID = i + 1;
      }
      i++;
    }
    // console.log("ID = "+gpxID);


  if(type.length < 1){
    alert("Must choose to order by length or name");
  }
  else if(type.toString().localeCompare("routeName") == 0){
    $('#routeDisplaySpecificTableBody').empty();
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/selectRoutesFromFILEName',
      data: {
        gpxID: gpxID
      },
      success: function (data) {
        // console.log("Success Length");
        let rows = data.returnData;
        i = 0;
        // console.log(rows);

        $('#routeDisplaySpecificTableBody').append("<tr>");
        $('#routeDisplaySpecificTableBody').append("<th><b>File Name</b></th>");
        $('#routeDisplaySpecificTableBody').append("<th><b>Route Name</b></th>");
        $('#routeDisplaySpecificTableBody').append("<th><b>Route Length</b></th>");
        $('#routeDisplaySpecificTableBody').append("</tr>");

        while(rows[i]){
          $('#routeDisplaySpecificTableBody').append("<tr>");
          $('#routeDisplaySpecificTableBody').append("<td>"+displayRoutesFile+"</td>");
          $('#routeDisplaySpecificTableBody').append("<td>"+rows[i].route_name+"</td>");
          $('#routeDisplaySpecificTableBody').append("<td>"+rows[i].route_len+"</td>");
          $('#routeDisplaySpecificTableBody').append("</tr>");
          i++;
        }
      }
    });
  }
  else if(type.toString().localeCompare("routeLength") == 0){
    $('#routeDisplaySpecificTableBody').empty();
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/selectRoutesFromFILELength',
      data: {
        gpxID: gpxID
      },
      success: function (data) {
        // console.log("Success Length");
        let rows = data.returnData;
        i = 0;
        // console.log(rows);

        $('#routeDisplaySpecificTableBody').append("<tr>");
        $('#routeDisplaySpecificTableBody').append("<th><b>File Name</b></th>");
        $('#routeDisplaySpecificTableBody').append("<th><b>Route Name</b></th>");
        $('#routeDisplaySpecificTableBody').append("<th><b>Route Length</b></th>");
        $('#routeDisplaySpecificTableBody').append("</tr>");

        while(rows[i]){
          $('#routeDisplaySpecificTableBody').append("<tr>");
          $('#routeDisplaySpecificTableBody').append("<td>"+displayRoutesFile+"</td>");
          $('#routeDisplaySpecificTableBody').append("<td>"+rows[i].route_name+"</td>");
          $('#routeDisplaySpecificTableBody').append("<td>"+rows[i].route_len+"</td>");
          $('#routeDisplaySpecificTableBody').append("</tr>");
          i++;
        }
      }
    });
  }


});

$('#query3').on("click", function(e) {
  e.preventDefault();
  //displayAllPointsRoute -current routeName
  let routeID = 0;
  $.ajax({
      async: false,
      type: 'get',
      dataType: 'json',
      url: '/displayRouteByName',
      success: function (data) {
        let rows = data.returnData;
        let i = 0;
        while(rows[i]){
          if(rows[i].route_name.toString().localeCompare(displayAllPointsRoute) == 0){
            routeID = rows[i].route_id;
          }
          i++;
        }
      }
    });
    // console.log("Found routeID = "+routeID);
    $.ajax({
        async: false,
        type: 'get',
        dataType: 'json',
        url: '/selectPointsFromRoute',
        data: {
          routeID: routeID
        },
        success: function (data) {
          let rows = data.returnData;
          let i = 0;
          $('#query3TableBody').empty();
          $('#query3TableBody').append("<tr>");
          $('#query3TableBody').append("<th><b>Point Index</b></th>");
          $('#query3TableBody').append("<th><b>Point Name</b></th>");
          $('#query3TableBody').append("<th><b>Latitude</b></th>");
          $('#query3TableBody').append("<th><b>Longitude</b></th>");
          $('#query3TableBody').append("</tr>");

          while(rows[i]){
            $('#query3TableBody').append("<tr>");
            $('#query3TableBody').append("<td>"+rows[i].point_index+"</td>");
            $('#query3TableBody').append("<td>"+rows[i].point_name+"</td>");
            $('#query3TableBody').append("<td>"+rows[i].latitude+"</td>");
            $('#query3TableBody').append("<td>"+rows[i].longitude+"</td>");
            $('#query3TableBody').append("</tr>");
            i++;
          }
        }
      });


});

$('#pointFromFileBtn').on("click", function(e) {
  e.preventDefault();

  let string;
  let CSVString;
  let numFiles;
  $.ajax({
      async: false,
      type: 'get',
      dataType: 'json',
      url: '/filesOnServer',
      success: function (data) {
        string = data.returnData;
        CSVString = string.split(" ");
        numFiles = CSVString.length - 1;
      }
    });

    let gpxID = 0;
    let i = 0;
    while(CSVString[i].length > 0){
      if(CSVString[i].toString().localeCompare(displayPointsFile) == 0){
        // console.log("fond, i = "+i);
        gpxID = i + 1;
      }
      i++;
    }
    // console.log("GPX_ID = "+gpxID);
    let k = 1;
    $.ajax({
        async: false,
        type: 'get',
        dataType: 'json',
        url: '/selectRoutesFromFILEName',
        data: {
          gpxID: gpxID
        },
        success: function (data) {
          let routeRows = data.returnData;
          if(!routeRows[0]){
            $('#query4TableBody').empty();
            alert("File: "+displayPointsFile+" has no routes or points");
          }
          else{
            $('#query4TableBody').empty();
            $('#query4TableBody').append("<tr>");
            $('#query4TableBody').append("<th><b>Route Name</b></th>");
            $('#query4TableBody').append("<th><b>Point Name</b></th>");
            $('#query4TableBody').append("<th><b>Point Index</b></th>");
            $('#query4TableBody').append("<th><b>Latitude</b></th>");
            $('#query4TableBody').append("<th><b>Longitude</b></th>");
            $('#query4TableBody').append("</tr>");
            i = 0;
            while(routeRows[i]){
              $.ajax({
                async: false,
                type: 'get',
                dataType: 'json',
                url: '/selectPointsFromRoute',
                data: {
                  routeID: routeRows[i].route_id
                },
                success: function (data) {
                  let pointRows = data.returnData;
                  let j = 0;
                  let unnamed = 0;
                  while(pointRows[j]){
                    $('#query4TableBody').append("<tr>");

                    let string = JSON.parse(JSON.stringify(routeRows[i].route_name));
                    // console.log("String = "+string);

                    if(string.toString().localeCompare("None") != 0){
                      $('#query4TableBody').append("<td>"+routeRows[i].route_name+"</td>");
                    }
                    else{
                      $('#query4TableBody').append("<td>Unnamed Route "+k+"</td>");
                      unnamed = 1;
                    }
                    $('#query4TableBody').append("<td>"+pointRows[j].point_name+"</td>");
                    $('#query4TableBody').append("<td>"+pointRows[j].point_index+"</td>");
                    $('#query4TableBody').append("<td>"+pointRows[j].latitude+"</td>");
                    $('#query4TableBody').append("<td>"+pointRows[j].longitude+"</td>");
                    $('#query4TableBody').append("</tr>");
                    j++;
                  }
                  if(unnamed == 1){
                    k++;
                    unnamed = 0;
                  }
                }
              }); //getPointsFromRouteId ajax
              i++;
            }  //while(routeRows[i]){
          }//else checker for routes
        } //selectRoutesFromFILEName success
      });


});
