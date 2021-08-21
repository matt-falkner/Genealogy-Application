'use strict'

//// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const bodyParser = require('body-parser');
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');
const mysql = require('mysql');
var port = 5000;

app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());
app.use(fileUpload());

app.use('/app', express.static(__dirname + '/app'));


const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

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
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }
    console.log("Uploaded file");
    res.redirect('/');
  });
});


app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

/* THE GLUE */
let parser = ffi.Library('./libparser', {
  'helloWorld' : [ 'string', [] ],
  'getFileDetails' : ['string', ['string']],
  'getIndividualsInFile' : ['string', ['string']],
  'writeGEDCOMfromJSON' : ['string', ['string', 'string']],
  'injectIndividualIntoFile' : ['string', ['string', 'string']],
  'getDescendantsForPersonInFile' : ['string', ['string', 'string', 'int']],
  'getAncestorsForPersonInFile' : ['string', ['string', 'string', 'int']],

});

let string = parser.helloWorld();
console.log("ffiTest.js: returns  "+ string +"\n");


var indexJS = require("./public/index.js");


//******************** Your code goes here ********************

app.get('/someendpoint', function(req , res){

const testFolder = 'uploads/';
var combined = [];

fs.readdir(testFolder, (err, files) => {
  files.forEach(file => {

    let JSONresult = parser.getFileDetails("./uploads/"+file);

    if (JSONresult[0] === '{')
    {

      let GEDJSONobj = JSON.parse(JSONresult);
      GEDJSONobj["filename"] = file;

      combined.push(GEDJSONobj);
    }
    else {
        console.log("error: " + JSONresult);
      }
    });


    res.send({
      json: JSON.stringify(combined)
    });

  })

});

app.post('/getTableSize', function(req,res) {

  let username = req.body["username"];
  let password = req.body["password"];
  let database = req.body["database"];

 var connection = mysql.createConnection({
        host     : 'dursley.socs.uoguelph.ca',
        user     : username,
        password : password,
        database : database
    });

 connection.connect( function(err)
 {
    if (err)
    {
		//connection.end();
      res.send({
        connection:"FAILURE",
        response:err.code
       });
    }
    else {

      connection.query("SELECT COUNT(*) FROM FILE", function (err, rows, fields) {

          if (err) {

            //connection.end();
            res.send({
              connection:"FAILURE",
              response:err.code
            });


        }
        else {

          console.log(rows);


          let row = rows[0];
          var numOfFiles = 0;
          var numOfIndividuals = 0;


          Object.keys(row).forEach(key => {
              numOfFiles = row[key];

               });


      connection.query("SELECT COUNT(*) FROM INDIVIDUAL", function (err, rows, fields) {

          if (err) {

            //connection.end();
            res.send({
              connection:"FAILURE",
              response:err.code
            });


        }
        else {

          let row = rows[0];


          Object.keys(row).forEach(key => {
              numOfIndividuals = row[key];

               });

           //connection.end();
            res.send({
              connection:"SUCCESS",
              numOfFiles: numOfFiles,
              numOfIndividuals: numOfIndividuals
            });
        }
      });

        }
      });
    }
 });
});

app.get('/getIndividuals/:name', function(req,res) {

  let file_loc = "uploads/" + req.params.name;
  let individualsJSON = parser.getIndividualsInFile(file_loc);
 res.send({
    json: individualsJSON
  });

});

app.get('/getDescendants/:name', function(req,res) {
  let file_location = "uploads/" + req.params.name;
  let personJSON = req.query.json;
  let number = req.query.number;
  let getDescendantsJSON = parser.getDescendantsForPersonInFile(file_location, personJSON, number);

  res.send( {
    json: getDescendantsJSON
  });

});

app.get('/getAncestors/:name', function(req,res) {
  let file_location = "uploads/" + req.params.name;
  let personJSON = req.query.json;
  let number = req.query.number;

  let getAncestorsJSON = parser.getAncestorsForPersonInFile(file_location, personJSON, number);

  res.send( {
    json: getAncestorsJSON
  });

});


app.post('/create', function(req, res){

  let json = JSON.stringify(req.body);
  let filename = "uploads/" + req.body["filename"];

  let result = parser.writeGEDCOMfromJSON(filename, json);

  res.send({
    foo:"bar"
  });

});

app.post('/insertIndividual', function(req, res){

  let json = JSON.stringify(req.body);
  let filename = "uploads/" + req.body["filename"];

  let result = parser.injectIndividualIntoFile(filename, json);

  res.send({
    json: json
  });

});

/*)

app.post('/saveAllIndividuals', function(req, res){

  console.log("Gets this far");
  let username = req.body["username"];
  let password = req.body["password"];
  let database = req.body["database"];

 connection = mysql.createConnection({
        host     : 'dursley.socs.uoguelph.ca',
        user     : username,
        password : password,
        database : database
    });


 let json = req.body["json"];
 let object = JSON.parse(json);

  if (object.length == 0)
  {
      alert("Alert: There are NO files on the server. Please UPLOAD or CREATE a GEDCOM FILE")
      addRowToTerminal("Warning", "No files found on server. Try Uploading or Creating one!");

  }

  console.log(object);


  for(var i = 0; i < object.length; i++)
  {


      var query = "INSERT INTO INDIVIDUAL (surname, given_name, sex, fam_size, source_file) VALUES ('falknerm', 'matthew', 'M', '2',(SELECT file_id FROM FILE WHERE file_name = 'simple.ged'))";


      connection.query(query, function (err, result)
      {
        if (err) console.log(err);
        console.log("Number of records inserted: " + result.affectedRows);

      });
  //}

});*/





app.post('/saveAllFiles', function(req, res){

  console.log("Gets this far");
  let username = req.body["username"];
  let password = req.body["password"];
  let database = req.body["database"];

 var connection = mysql.createConnection({
        host     : 'dursley.socs.uoguelph.ca',
        user     : username,
        password : password,
        database : database
    });


 let json = req.body["json"];
 let object = JSON.parse(json);

  if (object.length == 0)
  {
      alert("Alert: There are NO files on the server. Please UPLOAD or CREATE a GEDCOM FILE")
      addRowToTerminal("Warning", "No files found on server. Try Uploading or Creating one!");

  }

  var combined = [];

fs.readdir('uploads/', (err, files) => {
  files.forEach(file => {

    let JSONresult = parser.getFileDetails("./uploads/"+file);

    if (JSONresult[0] === '{')
    {

      let GEDJSONobj = JSON.parse(JSONresult);




        var encoding = GEDJSONobj.encoding;
        var filename = file;
        var source = GEDJSONobj.source;
        var gedcVersion = GEDJSONobj.gedcVersion;
        var numOfFamilies = GEDJSONobj.numOfFamily;
        var numOfIndividuals = GEDJSONobj.numOfINDI;
        var submitterName = GEDJSONobj.submitterName;
        var submitterAddress = GEDJSONobj.submitterAddress;

        var sql = "INSERT INTO FILE (file_name, sub_name, sub_addr, source, encoding, version, num_individuals, num_families) VALUES ('" + filename + "','" + submitterName + "','" + submitterAddress +  "','" + source + "','" + encoding + "','" + gedcVersion + "','" + numOfIndividuals + "','" + numOfFamilies + "')";

             console.log(sql);
        console.log(filename);

 connection.query(sql, function (err, rows, fields) {
        if (err) {
          console.log(err)
          //connection.end();
          res.send({
                connection:"FAILURE"
            });
        }
        else {

        console.log(file);

        let file_loc = "uploads/" + file;
        let individualsJSON = parser.getIndividualsInFile(file_loc);
        console.log(individualsJSON);
        let individuals = JSON.parse(individualsJSON);


        if (individuals.length == 0)
        {
          console.log("No Individuals in " + filename + ".\nHowever, you can add some if you like!");
        }
        else {
               for(var i = 0; i < individuals.length; i++)
               {
                    var givenName = individuals[i].givenName;
                    var surname = individuals[i].surname;
                    var sex = individuals[i].sex;
                    var numOfFamily = individuals[i].numOfFamily;

                    var query = "INSERT INTO INDIVIDUAL (surname, given_name, sex, fam_size, source_file) VALUES ('"+ surname+"', '"+ givenName+"', '"+ sex +"', '"+ numOfFamily+"',(SELECT file_id FROM FILE WHERE file_name = '" + file + "'))";


                    connection.query(query, function (err, result)
                    {
                      if (err)  {
                       if (err.code === "ER_SUBQUERY_NO_1_ROW")
                       {
                          console.log("Duplicate in the system");
                       }
                      }
                      else {
                        console.log("Number of records inserted: " + result.affectedRows);

                      }

                    });

                }
         }
        }

    });


    }
  })

});
//connection.end();
    res.send({
      connection:"SUCCESS"
    });


});



app.post('/getAllIndividualsInDatabase', function(req, res)
{

  console.log("GET INDIVILS FROM DATA");
  let username = req.body["username"];
  let password = req.body["password"];
  let database = req.body["database"];

 var connection = mysql.createConnection({
        host     : 'dursley.socs.uoguelph.ca',
        user     : username,
        password : password,
        database : database
    });


 connection.connect( function(err) {

    if (err) {
       //connection.end();
        res.send({
          connection:"FAILURE"
        });
    }
    else {

         connection.query("SELECT * FROM INDIVIDUAL ORDER BY surname", function (err, rows, fields) {
        if (err) {
          if (err.code === "ER_TABLE_EXISTS_ERROR")
          {
            console.log("table already exits");
          }
          else {
            //connection.end();
            res.send({
              connection:"FAILURE"
            });

          }
        }
        else {
          console.log(fields);
          console.log(rows);

            //connection.end();
            res.send({
              connection:"SUCCESS",
              response:rows
            });
        }
      });





    }
  });
});


app.post('/createTables', function(req, res)
{
  let username = req.body["username"];
  let password = req.body["password"];
  let database = req.body["database"];

 var connection = mysql.createConnection({
        host     : 'dursley.socs.uoguelph.ca',
        user     : username,
        password : password,
        database : database
    });


var alreadyExists = false;

 connection.connect( function(err) {

    if (err) {
       //connection.end();
        res.send({
          connection:"FAILURE"
        });
    }
    else {

      connection.query("create table FILE (file_id int not null auto_increment,  file_name char(60) not null, sub_name char(62) not null, sub_addr char(255), source char(250) not null,  encoding char(10) not null, version char(10) not null, num_individuals int, num_families int, primary key(file_id) )", function (err, rows, fields) {
        if (err) {
          if (err.code === "ER_TABLE_EXISTS_ERROR")
          {
            console.log("table already exits");
            alreadyExists = true;
          }
          else {
            //connection.end();
            res.send({
              connection:"FAILURE"
            });

          }
        }
//delete from table
    });
    connection.query("create table INDIVIDUAL (ind_id int not null auto_increment,  surname char(255) not null, given_name char(255) not null, sex char(1), fam_size int, source_file int, primary key(ind_id), foreign key(source_file) references FILE(file_id) ON DELETE CASCADE)", function (err, rows, fields) {
        if (err) {
          if (err.code === "ER_TABLE_EXISTS_ERROR")
          {
            console.log("table already exists");
            alreadyExists = 1;
          }
          else {
            //connection.end();
             res.send({
              connection:"FAILURE"
            });
          }
        }
              //connection.end();

              if (alreadyExists == 1)
              {
                   res.send({
                  connection:"NOTE"
                  });
              }
              else {
                   res.send({
                  connection:"SUCCESS"
                  });
              }
        });
    }
  });
});


app.post('/clearDatabase', function(req,res)
{
  console.log("end point hit");
  let username = req.body["username"];
  let password = req.body["password"];
  let database = req.body["database"];

   var connection = mysql.createConnection({
        host     : 'dursley.socs.uoguelph.ca',
        user     : username,
        password : password,
        database : database
    });


  connection.connect( function(err) {

    if (err) {

       //connection.end();

      console.log("Invalid Credentials");

        res.send({
          connection:"FAILURE"
        });
    }
    else
    {
      let sqlFILE = "DELETE FROM INDIVIDUAL";
      connection.query(sqlFILE, function (err, rows, fields) {
        if (err) console.log("Something went wrong. "+err);
        else {
           let sqlINDV = "DELETE FROM FILE";
      connection.query(sqlINDV, function (err, rows, fields) {
          if (err) {
            console.log("Something went wrong. "+err);
          }
          else {
            console.log("Success");
          }
      });
        }
      });


		//connection.end();
       res.send({
          connection:"SUCCESS"
        });
    }

  });


});


app.post('/customQuery', function(req, res){

  let username = req.body["username"];
  let password = req.body["password"];
  let database = req.body["database"];
  let query = req.body["query"];

   var connection = mysql.createConnection({
        host     : 'dursley.socs.uoguelph.ca',
        user     : username,
        password : password,
        database : database
    });

  connection.connect( function(err)
  {
    if (err) {
       //connection.end();
		/*
      console.log("Invalid Credentials");

        res.send({
          connection:"FAILURE"
        });
        * */
    }
    else {

      connection.query(query, function (err, rows, fields) {
        if (err) {
           //connection.end();
          //console.log("Something went wrong. "+err);
           //connection.end();

             res.send({
              connection:"FAILURE",
              response: err.code
            });

        }
        else {
          //console.log(rows);
          //connection.end();
          //connection.end();
           res.send({
              connection:"SUCCESS",
              response: rows
          });

        }
      });

    }

  });

});




app.post('/connectToDatabase', function(req, res){

  let username = req.body["username"];
  let password = req.body["password"];
  let database = req.body["database"];

   var connection = mysql.createConnection({
        host     : 'dursley.socs.uoguelph.ca',
        user     : username,
        password : password,
        database : database
    });
  connection.connect( function(err) {

    if (err) {

       connection.end();

      console.log("Invalid Credentials");

        res.send({
          connection:"FAILURE"
        });
    }
    else {
       //connection.end();

      console.log("Logged in");
        res.send({
          connection:"SUCCESS"
        });
    }

  }





  );

});





app.listen(port, function() {
    console.log("Listening on port", + port);
});

console.log('Running app at localhost' + port);


/*
 for(var i = 0; i < object.length; i++)
  {
        var obj = object[i];

        var encoding = object[i].encoding;
        var filename = object[i].filename;
        var source = object[i].source;
        var gedcVerson = object[i].gedcVersion;
        var numOfFamilies = object[i].numOfFamily;
        var numOfIndividuals = object[i].numOfINDI;
        var submitterName = object[i].submitterName;
        var submitterAddress = object[i].submitterAddress;
        var filelink = "./uploads/" + filename;

        var sql = "INSERT INTO FILE (file_name, sub_name, sub_addr, source, encoding, version, num_individuals, num_families) VALUES ('" + filename + "','" + submitterName + "','" + submitterAddress +  "','" + source + "','" + encoding + "','" + gedcVerson + "','" + numOfIndividuals + "','" + numOfFamilies + "')";
        console.log(sql);
        console.log(filename);

 connection.query(sql, function (err, rows, fields) {
        if (err) {
          console.log(err)
          res.send({
                connection:"FAILURE"
            });
        }
        else {
            console.log(filename);
        }

    });

       // addRowToFilesView(filelink, filename, source, encoding, submitterName, submitterAddress, numOfIndividuals, numOfFamilies);

        //addRowToTerminal("Success", "Found Valid Gedcom: " + filename + " in server uploads");
    }*/

    /*
  connection.query("create table FILE (file_id int not null auto_increment,  file_name char(60) not null, sub_name char(62) not null, sub_addr char(255), source char(250) not null,  encoding char(10) not null, version char(10) not null, num_individuals int, num_families int, primary key(file_id) )", function (err, rows, fields) {
        if (err) console.log("Something went wrong. "+err);
        else {
          console.log("Success");
        }

    });
    connection.query("create table INDIVIDUAL (ind_id int not null auto_increment,  surname char(255) not null, given_name char(255) not null, sex char(1), fam_size int, primary key(ind_id) )", function (err, rows, fields) {
        if (err) console.log("Something went wrong. "+err);
        else {
          console.log("Success");
        }

    });


        connection.query("INSERT INTO INDIVIDUAL (surname, given_name, sex) VALUES ('Falkner','Matthew','M')", function (err, rows, fields) {
        if (err) console.log("Something went wrong. "+err);
        else {
          console.log("Success");
        }

    });
    */

