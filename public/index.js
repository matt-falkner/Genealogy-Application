
var selectedFilelink = "";
var selectedFilename = "";


//Function I wrote to remove CSS for selection because aparantly nobody on the internet has done this manually 
function hover(element)
{
    if($(element).hasClass('selected'))
    {
        console.log("already selected");

      $(element).siblings().removeClass('hover');  

    }
    else {
       $(element).addClass('hover').siblings().removeClass('hover');   
    }
}


//Saves me a line of code somewhere I swear
function isEmpty(string) 
{
    return (!string || string.length === 0);
}




//Basically if the tab is selected hide other tabs and show the current active one. 
//Based off function example from W3SCHOOLS.com, 
function openTab(evt, tabName) 
{
    // Declare all variables
    var content = document.getElementsByClassName("tabcontent");

    for (var i = 0; i < content.length; i++) 
    {
        content[i].style.display = "none";
    }

    var links = document.getElementsByClassName("tablinks");
    for (var i = 0; i < links.length; i++) 
    {
        links[i].className = links[i].className.replace(" active", "");
    }

    //Select the tab you want to select
    document.getElementById(tabName).style.display = "block";

    evt.currentTarget.className += " active";
} 



//Create an individual, sanitize the input and send it out for creation and refreshment.
function sanitizeIndividualCreationInput()
{
    var givenName = document.getElementById("givenNameInd").value;
    var surname = document.getElementById("surnameInd").value;
    var sex = document.getElementById("sex");
    var sex_value = sex.options[sex.selectedIndex].text;


     if (isEmpty(givenName))
     {
        alert("Missing Data: Firstname.");
        return;
     }
     if (isEmpty(surname))
     {
        alert("Missing Data: Lastname.");
        return;
     }

     var IndividualJSON = {};
     IndividualJSON["givenName"] = givenName;
     IndividualJSON["surname"] = surname;
     IndividualJSON["sex"] = sex_value;
     IndividualJSON["filename"] = selectedFilename;

    var individualModal = document.getElementById('indModal');
    individualModal.style.display = "none";


    insertIndividual(IndividualJSON);
}

//When the user creates a file, clean the input for bad input. Require the user to give valid input
function sanitizeFileCreationInput()
{

    var filename = document.getElementById("filename").value;
    var givenName = document.getElementById("givenName").value;
    var surname = document.getElementById("surname").value;
    var address = document.getElementById("address").value;
    var source = document.getElementById("source").value;

    var e = document.getElementById("encoding");
    var encoding = e.options[e.selectedIndex].text;

    if (isEmpty(filename))
    {
        alert("Error: Missing Filename");
        return;
    }
    else {
        var ext =  filename.split('.').pop();

        if (ext !== "ged" && ext !== "GED")
        {
            alert("Error: Incorrect File extension. Should be .GED or .ged");
            return;
        }
    }
    if (isEmpty(givenName))
    {
        alert("Error: Missing Firstname");
        return;
    }
    if (isEmpty(surname))
    {
        alert("Error: Missing Lastname");
        return;
    }
    if (isEmpty(address))
    {
        alert("Error: Missing address");
        return;
    }
    if (isEmpty(source))
    {
        alert("Error: Missing source");
        return;
    }

    var GEDCOMjson = {};
    GEDCOMjson["filename"] = filename;
    GEDCOMjson["subName"] = givenName + " " + surname;
    GEDCOMjson["subAddress"] = address;
    GEDCOMjson["source"] = source;
    GEDCOMjson["gedcVersion"] = "5.5.5";
    GEDCOMjson["encoding"] = encoding;


    var modal = document.getElementById('myFileModal');
    modal.style.display = "none";

    createFile(GEDCOMjson);
}

function createFile(withJsonConfig)
{
        $.ajax({ 
        type: "post",
        dataType: "json",
        data: withJsonConfig,
        url: "/create",
        success: function(data){  
            //Daisy chaining ajax calls for refreshing page correctly.
            GET_Files();
          }
        });

}

function insertIndividual(withIndividualJSON)
{
     $.ajax({ 
        type: "post",
        dataType: "json",
        data: withIndividualJSON,
        url: "/insertIndividual",
        success: function(data){  
            let object = JSON.parse(data.json);
            //Manually refreshing the Individuals View with the current file that is selected, displaying the TRUE results.
            updateIndividualsView(object.filename)
            addRowToTerminal("Success", "Inserted " + object.givenName + " "+ object.surname + " into " + object.filename);

        }
        });
}

function GET_Files()
{
    $.ajax({ 
   type: "get",
   dataType: "json",
   url: "/someendpoint",
   success: function(data){        
    
    console.log(data);
    let object = JSON.parse(data.json);

          $("#files-screen tr").remove(); 

    if (object.length == 0)
    {
        alert("Alert: There are NO files on the server. Please UPLOAD or CREATE a GEDCOM FILE")
        addRowToTerminal("Warning", "No files found on server. Try Uploading or Creating one!");

    }



   for(var i = 0; i < object.length; i++) {
        var obj = object[i];

        var encoding = object[i].encoding;
        var filename = object[i].filename;
        var source = object[i].source;
        var gedcVerson = object[i].gedcVerson;
        var numOfFamilies = object[i].numOfFamily;
        var numOfIndividuals = object[i].numOfINDI;
        var submitterName = object[i].submitterName;
        var submitterAddress = object[i].submitterAddress;
        var filelink = "./uploads/" + filename;
        
        addRowToFilesView(filelink, filename, source, encoding, submitterName, submitterAddress, numOfIndividuals, numOfFamilies);
        
        addRowToFileSelector(filename);


        addRowToTerminal("Success", "Found Valid Gedcom: " + filename + " in server uploads");
    }


   }
});  

}

function addRowToFileSelector(filename)
{
      var selectFile =  document.getElementById("files-input");

      var  option3 = document.createElement( 'option' );
      option3.value = filename;
      option3.text = filename;
     

      selectFile.add( option3);
}



var valid_username = "";
var valid_password = "";
var valid_database = "";

function getTableSize()
{

    console.log("get table size function");

 $.ajax({ 
        type: "post",
        dataType: "json",
        data: {username:valid_username, password:valid_password, database:valid_database },
        url: "/getTableSize",
        success: function(res)
        { 
            if (res.connection == "SUCCESS"){
                let numOfFiles = res.numOfFiles;
                let numOfIndividuals = res.numOfIndividuals;

                printToDatabaseConsole(res.connection, "STATUS: there are " + numOfFiles + " Files and " + numOfIndividuals + " Individuals");

            }
        }
    });


}



function saveAllFiles()
{


 $.ajax({ 
   type: "get",
   dataType: "json",
   url: "/someendpoint",
   success: function(data){        
         $.ajax({ 
                type: "post",
                dataType: "json",
                data: {username:valid_username, password:valid_password, database:valid_database, json:data.json },
                url: "/saveAllFiles",
                success: function(result)
                { 
                    if (result.connection === "SUCCESS")
                    {
                       $("#saveAllItems").prop("disabled", true);

                        printToDatabaseConsole(result.connection, "Saved all files from /uploads into INDIVUDAL table.");

                    }
                    else {
                       printToDatabaseConsole(result.connection, "Failed To save all files. Possible bad creds. try refreshing.");
                    }
                }
            });
        }    
    });
}

function clearDatabase() {
    $.ajax({ 
        type: "post",
        dataType: "json",
        data: {username:valid_username, password:valid_password, database:valid_database},
        url: "/clearDatabase",
        success: function(result)
        { 
            if (result.connection === "SUCCESS")
            {
                printToDatabaseConsole(result.connection, "Cleared FILE and INDIVIDUAL tables. Empty Now.");
                $("#saveAllItems").removeAttr('disabled');
            }
            else {
                printToDatabaseConsole(result.connection, "Failed to clear tables. Possible bad creds. try refreshing.");
            }
        }
    });
}


function connectToDatabase(username, password, database)
{
 $.ajax({ 
        type: "post",
        dataType: "json",
        data: {username:username, password:password, database:database },
        url: "/connectToDatabase",
        success: function(result)
        { 
            if (result.connection === "SUCCESS")
            {
                valid_username = username;
                valid_password = password; 
                valid_database = database;
                 
                 $.ajax({ 
                    type: "post",
                    dataType: "json",
                    data: {username:valid_username, password:valid_password, database:valid_database },
                    url: "/createTables",
                    success: function(result)
                    {   
                        if (result.connection === "SUCCESS")
                        {
                            console.log("Logged in: Created new tables");
                            document.getElementById('loginModal').style.display='none';
                            printToDatabaseConsole(result.connection, "Connected to Database. Created [NEW] FILE and INDIVIDUAL tables.")
                            displayActions();
                        }            
                        else if (result.connection === "FAILURE")
                        {
                            console.log("Logged in 1st time: Second time: Failed to login, bad creds. This shouldn't happen");
                            alert("Invalid Creds. Try again.");
                        }
                        else if (result.connection === "NOTE")
                        {
                            console.log("Logged in: But Tables already exists in database");
                            document.getElementById('loginModal').style.display='none';
                            printToDatabaseConsole(result.connection, "Connected to Database. Note: FILE and INDIVIDUAL Tables already exist.");
                            displayActions();

                        }
                    }
                });
            
            }
            else if (result.connection === "FAILURE")
            {
                alert("Invalid Creds. Try again.");
            }
        }
    });
}



function displayActions()
{
    var actions = document.getElementById("actions");
    actions.style.display = "block";

    var loginForm = document.getElementById("login-form");
    loginForm.style.display = "none";

    document.getElementById("logged-in-text").innerHTML = "Hello, " + valid_username +  ". <small><span style='color: #428bca;'><b> You are Logged-In to your " + valid_database + " database";


}
function hideActions()
{
    var actions = document.getElementById("actions");
    actions.style.display = "none";

    var loginForm = document.getElementById("login-form");
    loginForm.style.display = "block";
}

function runQuery()
{
    let value = $('#querySelector').val();
	
	console.log("running query" + value);


    if (value === "getAllINDV") 
    {
       getAllIndividualsFromDatabase();
    }
    else if (value === "getFileINDV") 
    {
        let filename = $("#files-input option:selected").val()
        customQuery("SELECT * FROM INDIVIDUAL where source_file = (SELECT file_id FROM FILE WHERE file_name = '" + filename + "')")
    }
    else if (value === "getAllSex")
    {
         let filename = $("#files-input option:selected").val()
         let sex = $("#sex-input option:selected").val()


         customQuery("SELECT * FROM INDIVIDUAL where source_file = (SELECT file_id FROM FILE WHERE (file_name = '" + filename + "') AND (sex = '"+ sex + "'))");

    }
    else if (value === "join")
    {
        let encoding = $("#encoding-input option:selected").val()
        customQuery("SELECT sub_name,encoding,given_name FROM FILE JOIN INDIVIDUAL ON source_file = file_id WHERE encoding = '" + encoding  + "'");
    }
    else if (value === "select") 
    {
       
        //alert('Value changed to ' + value);
        let text = $('#query-input').val();
        if (isEmpty(text))
        {
            alert("You've selected Query, but your query input is empty.");
        }
        else {
            customQuery(text);
        }
    }
}


function customQuery(query)
{
    //console.log("Hello W");
	console.log("running query" + query);

   $.ajax({ 
        type: "post",
        dataType: "json",
        data: {username:valid_username, password:valid_password, database:valid_database, query:query },
        url: "/customQuery",
        success: function(result)
        {   console.log(result);
            if (result.connection === "SUCCESS")
            {
                insertObjectToTable(result.response);
            }
            else {
				$("#query-screen tr").remove(); 
				alert("Error in Query " + result.response); 

			}
        }
    });
}


function viewDidLoad()
{ 

    /* Hide other options by default */
     $('#query-input').hide();
      $('#files-input').hide();
       $('#sex-input').hide();
                     $('#query-description').text('Get All Individuals on the database');
        $('#encoding-input').hide();




    document.getElementById("hometab").click();

    //only enable upload button on upload of filre, prevents crash
    $('input:file').on("change", function() {
        $('input:submit').prop('disabled', !$(this).val()); 
    });

    $('.modal-content').on('submit', function (e) {
         e.preventDefault();
        let username = document.getElementById("username").value;
        let password = document.getElementById("password").value; 
        let database = document.getElementById("database").value;
        connectToDatabase(username, password, database);

    });



$('#querySelector').on('change', function (e) {

    let val = $('#querySelector').val();




    if (val == "getAllINDV")
    {
         $('#encoding-input').hide();
            $('#query-input').hide();
            $('#files-input').hide();
             $('#sex-input').hide();
              $('#query-description').text('Get All Individuals on the database');


    }
    if (val == "getFileINDV")
    {
             $('#encoding-input').hide();
            $('#query-input').hide();
            $('#files-input').show();
             $('#sex-input').hide();
             $('#query-description').text('Get All Individuals on the database from a specific FILE');






    }
    if (val == "getAllSex")
    {
        $('#encoding-input').hide();
        $('#query-input').hide();
        $('#files-input').show();
        $('#sex-input').show();
         $('#query-description').text('Get All Individuals on the database from a specific FILE AND of a specific SEX');

    }
    if (val == "join")
    {
                 $('#encoding-input').show();
            $('#query-input').hide();
            $('#files-input').hide();
             $('#sex-input').hide();
              $('#query-description').text('Get Join of Encoding name, submitter name, and Individual name where Encoding = selected encoding');


    }


    if (val == "select")
    {
		    $('#encoding-input').hide();

            $('#query-input').show();
            $('#files-input').hide();
             $('#sex-input').hide();
          $('#query-description').text('Do any command you want, please do not drop my tables ');



    }



});



        hideActions();


    GET_Files();

    var addIndButton = document.getElementById('addIndividualModal');
    addIndButton.disabled = true;

    var desButton = document.getElementById('getDescendantsButton');
    desButton.disabled = true;

    var ansButton = document.getElementById('getAncestorsButton');
    ansButton.disabled = true;


    var individualModal = document.getElementById('indModal');
    var indSpan = document.getElementById('closeIndModal');
    var loginModal = document.getElementById('id01');


    var modal = document.getElementById('myFileModal');
    var span = document.getElementsByClassName("close")[0];

    document.getElementById("myModal").addEventListener("click", createFileModal);
    document.getElementById("cancelFileModal").addEventListener("click", cancelModal);


    document.getElementById("addIndividualModal").addEventListener("click", createIndModal);
    document.getElementById("cancelIndModal").addEventListener("click", cancelIndModal);

    
    /* Modal stuff */
    window.onclick = function(event) 
    {
        if (event.target == modal) {
            modal.style.display = "none";
        }
        if (event.target == individualModal)
        {
             individualModal.style.display = "none";
        }
        if (event.target == loginModal) 
        {
            loginModal.style.display = "none";
        }
    }
    span.onclick = function() {
       modal.style.display = "none";
    }
    indSpan.onclick = function() {
        individualModal.style.display = "none";
    }
    function createIndModal()
    {
        individualModal.style.display = "block";
    }
    function cancelIndModal()
    {
         individualModal.style.display = "none";

    }  
    function createFileModal()
    {
        modal.style.display = "block";
    }

    function cancelModal()
    {
      modal.style.display = "none"; 
    }   



// When the user clicks anywhere outside of the modal, close it
window.onclick = function(event) {
   
}

}

function printToDatabaseConsole(state, operation)
{
    if (state === "FAILURE")
    {
         var str = '<tr bgcolor="#d9534f"><td>' + state + '</td>\ <td>'+ operation +'</td>\</tr>';
     $('#database-console').append(str);
        localStorage.setItem("data", $('#database-screen').html());

    }
    if (state === "SUCCESS")
    {
         var str = '<tr bgcolor="#5cb85c"><td>' + state + '</td>\ <td>'+ operation +'</td>\</tr>';
    $('#database-console').append(str);
    localStorage.setItem("data", $('#database-screen').html());   
    }
    if (state === "NOTE")
    {
         var str = '<tr bgcolor="#5cb85c"><td>' + state + '</td>\ <td>'+ operation +'</td>\</tr>';
    $('#database-console').append(str);
    localStorage.setItem("data", $('#database-screen').html());   
    }

}

function addRowToTerminal(state, operation)
{

    if (state === "Failure")
    {
         var str = '<tr bgcolor="#d9534f"><td>' + state + '</td>\ <td>'+ operation +'</td>\</tr>';
     $('#operation-console').append(str);
        localStorage.setItem("data", $('#console-screen').html());

    }
    if (state === "Success")
    {
         var str = '<tr bgcolor="#5cb85c"><td>' + state + '</td>\ <td>'+ operation +'</td>\</tr>';
    $('#operation-console').append(str);
    localStorage.setItem("data", $('#console-screen').html());   
    }
    if (state === "Warning")
    {
         var str = '<tr bgcolor="#ffbb33"><td>' + state + '</td>\ <td>'+ operation +'</td>\</tr>';
         $('#operation-console').append(str);
        localStorage.setItem("data", $('#console-screen').html());   
    }
    if (state === "Info")
    {
         var str = '<tr bgcolor="#33b5e5"><td>' + state + '</td>\ <td>'+ operation +'</td>\</tr>';
         $('#operation-console').append(str);
        localStorage.setItem("data", $('#console-screen').html());   
    }
}





function getAllIndividualsFromDatabase()
{

    $.ajax({ 
        type: "post",
        dataType: "json",
        data: {username:valid_username, password:valid_password, database:valid_database },
        url: "/getAllIndividualsInDatabase",
        success: function(result)
        {   
            console.log(result);

            let object = result.response;
           

            if (object.length > 0)
            {
                insertObjectToTable(object);

                printToDatabaseConsole(result.connection, "Found Individuals");

               
            }/*
            else {
                 $("#query-screen tr").remove(); 
                printToDatabaseConsole(result.connection, "INDIVIDUALS TABLE EMPTY");

                if (object.length == 0 )
                {
                    alert("query returned empty result. IE. No Individuals ");
                }


            }*/

        }
    });

}

function insertObjectToTable(object)
{

			$("#query-screen tr").remove(); 

    if (object.length == 0 )
    {
        alert("query returned empty result. IE. No matches ");
    }


    for (var i = 0; i < object.length; i++)
    {
		


        if (i == 0)
        {

           let obj = object[i];
           var row = $('<tr></tr>');

           Object.keys(obj).forEach(key => {
            $('<td></td>').text(key).appendTo(row); 

           }); 
         
           $('#query-console').append(row);
           localStorage.setItem("data", $('#query-screen').html());
        }


       let obj = object[i];
       var row = $('<tr></tr>');

       Object.keys(obj).forEach(key => {
        $('<td></td>').text(obj[key]).appendTo(row); 

       }); 
     
       $('#query-console').append(row);
       localStorage.setItem("data", $('#query-screen').html());
                  
    }

}



function addRowToQueryView(givenName, surname, sex, numOfFamilies, source_file){
     var str = '<tr class = "boxType" onclick="selectedIndividual(this)"><td>' + givenName + '</td>\ <td>'+ surname +'</td>\ <td>'+sex+'</td>\ <td>'+numOfFamilies+'</td>\ <td>'+source_file+'</td>\</tr>';
    $('#query-console').append(str);
    localStorage.setItem("data", $('#query-screen').html());
    
}


//https://codepen.io/scottloway/pen/zqoLyQ
function addRowToFilesView(filelink, filename, source, encoding, submitterName, submitterAddress, numOfIndividuals, numOfFamilies){
    var str = '<tr class = "boxType" onmouseover="hover(this)" onclick="selectedFile(this)"><td class="filename">'+ "<a href=\"" + filelink + "\" class=\"filelink\">" + filename + '</td>\ <td class="source">'+ source +'</td>\ <td class="encoding">'+encoding+'</td>\ <td class="submitterName">'+ submitterName +'</td>\ <td class="submitterAddress">'+ submitterAddress+'</td>\ <td>'+ numOfIndividuals +'</td>\ <td>'+ numOfFamilies +'</td>\ </tr>';
    $('#files').append(str);
    localStorage.setItem("data", $('#screen').html());
}

function addRowToAncestorsView(genLevel, givenName, surname, sex, numOfFamilies)
{
   var str = '<tr class = "boxType"> <td>' + "+ " + genLevel + '</td>\ <td>' + givenName + '</td>\ <td>'+ surname +'</td>\ <td>'+sex+'</td>\ <td>'+numOfFamilies+'</td>\ </tr>';
    $('#ancestors').append(str);
    localStorage.setItem("data", $('#ancestors-screen').html());

}


function addRowToDescendantsView(genLevel, givenName, surname, sex, numOfFamilies)
{
   var str = '<tr class = "boxType"> <td>' + "+ " + genLevel + '</td>\ <td>' + givenName + '</td>\ <td>'+ surname +'</td>\ <td>'+sex+'</td>\ <td>'+numOfFamilies+'</td>\ </tr>';
    $('#descendants').append(str);
    localStorage.setItem("data", $('#descendants-screen').html());

}

function addRowToIndividualsView(givenName, surname, sex, numOfFamilies)
{
    var str = '<tr class = "boxType" onclick="selectedIndividual(this)"><td>' + givenName + '</td>\ <td>'+ surname +'</td>\ <td>'+sex+'</td>\ <td>'+numOfFamilies+'</td>\ </tr>';
    $('#individuals').append(str);
    localStorage.setItem("data", $('#individuals-screen').html());
    
}


function updateIndividualsView(filename)
{



    $.ajax({ 
        type: 'get',
        url: "/getIndividuals/"+ filename,
        dataType:'json',
        success: function(data){  
            $("#individuals-screen tr").remove(); 

             let object = JSON.parse(data.json);
             if (object.length == 0)
             {
                alert("No Individuals in " + filename + ".\nHowever, you can add some if you like!");
             } 
             else {

             $('#getDSelector').empty();
             $('#getASelector').empty();
             $('#files-input').empty();


               for(var i = 0; i < object.length; i++) 
               {
                    var obj = object[i];
                    var givenName = object[i].givenName;
                    var surname = object[i].surname;
                    var sex = object[i].sex;
                    var numOfFamily = object[i].numOfFamily;
                    addRowToIndividualsView(givenName, surname, sex, numOfFamily);

                    let personJson = JSON.stringify(object[i]);

                    addRowToOtherViews(givenName, surname, sex, numOfFamily, personJson);
                    
                }


             }  
        }
        });

}


function getDescendants()
{
  var person = $('#getDSelector').find(":selected").val();
  console.log("Getting descendants of" + person);

  
let numOfDesc = document.getElementById("numOfDesc").value;

if (isEmpty(numOfDesc))
{
    alert("Invalid '# of generations. Please input a number");
}    
else if (isEmpty(person))
{
    alert("That is not a person, you cannot do that");
}
else if (person == "ERROR")
{
    alert("This is not an individual, you cannot get it's descendants");
}

else {
    var object = JSON.parse(person);
    $.ajax({ 
        type: 'get',
        url: "/getDescendants/" + selectedFilename,
        data: {json: person, number: numOfDesc},
        success: function(data){  

            $("#descendants-screen tr").remove(); 
            var object = JSON.parse(data.json);
            console.log(object)


            if (object.length > 0)
            {
                 addRowToTerminal("Success", "Found descendants for selected Person.");

                 for (var i = 0; i < object.length; i++)
                {

                    var generation = object[i];

                    for (var j = 0; j < generation.length; j++)
                    {
                        var person = generation[j];
                        //console.log(person.givenName + " in gen " + i);
                         var givenName = person.givenName;
                        var surname = person.surname;
                        var sex = person.sex; 
                        var numOfFamily = person.numOfFamily;

                        addRowToDescendantsView(i + 1, givenName, surname, sex, numOfFamily)
                    }
                 }
            }
            else {

                 addRowToTerminal("Info", "Selected Person has no descendants.");

                alert("Person has NO descendants, try selecting another Individual!");
            }
        }
    });

}

}

function getAncestors()
{
  var person = $('#getASelector').find(":selected").val();
  console.log("Getting ancestors of " + person);
 
let numOfDesc = document.getElementById("numOfAncs").value;

if (isEmpty(numOfDesc))
{
    alert("Invalid '# of generations. Please input a number");
    return;
}
if (isEmpty(person))
{
    alert("That is not a person, you cannot do that");
    return;
}

console.log(person);


if (person == "ERROR")
{
    alert("This is not an individual, you cannot get it's ancestors");
    return;
}
else 
{
  var object = JSON.parse(person);
  console.log(object);
  console.log(numOfDesc);

    $.ajax({ 
        type: 'get',
        url: "/getAncestors/" + selectedFilename,
        data: {json: person, number: numOfDesc},
        success: function(data){  

            $("#ancestors-screen tr").remove(); 
            var object = JSON.parse(data.json);
            console.log(object)

            if (object.length > 0)
            {
                 addRowToTerminal("Success", "Found descendants for selected Person.");

                 for (var i = 0; i < object.length; i++)
                 {

                    var generation = object[i];

                    for (var j = 0; j < generation.length; j++)
                    {
                        var person = generation[j];
                        //console.log(person.givenName + " in gen " + i);
                         var givenName = person.givenName;
                        var surname = person.surname;
                        var sex = person.sex; 
                        var numOfFamily = person.numOfFamily;


                        addRowToAncestorsView(i + 1, givenName, surname, sex, numOfFamily)
                    }


                 }

            }
            else {

                addRowToTerminal("Info", "Selected Person has no descendants.");

                alert("Person has NO ancestors, try selecting another individual!");
            }
        }
    });
  }

}




function selectedFile(element)
{

    var addIndButton = document.getElementById('addIndividualModal');

    if ( $(element).hasClass('hover') )
    {
        $(element).removeClass('hover');
    }

   $(element).addClass('selected').siblings().removeClass('selected');   

    var source = element.getElementsByClassName('source').item(0).innerHTML;
    var encoding = element.getElementsByClassName('encoding').item(0).innerHTML;
    var submitterName = element.getElementsByClassName('submitterName').item(0).innerHTML;
    var filename = element.getElementsByClassName('filelink').item(0).innerHTML;
    var filelink = element.getElementsByClassName('filelink').item(0).getAttribute("href");


    addIndButton.disabled = false;
    addIndButton.innerHTML = "Insert Individual into: " + filename;


    var newTitleHTML = "<h2 href=\"" + filelink + "\" id=\"selectedFileTitle\" style=\" display: inline-block;\"> Individuals in selected File </h2>"


    document.getElementById("selectedFileTitle").innerHTML = "Individuals in  <span style='color: #428bca;'><b>" + filename+"</b></span>";

    document.getElementById("selectedFileTitleDecs").innerHTML = "Get decendants for <span style='color: #428bca;'><b>" + filename+"</b></span>";


    document.getElementById("selectedFileTitleAnces").innerHTML = "Get ancestors for <span style='color: #428bca;'><b>" + filename+"</b></span>";


    var desButton = document.getElementById('getDescendantsButton');
    desButton.disabled = false;

    var ansButton = document.getElementById('getAncestorsButton');
    ansButton.disabled = false;

  $("#descendants-screen tr").remove(); 

    selectedFilelink = filelink;
    selectedFilename = filename;

    $.ajax({ 
        type: 'get',
        url: "/getIndividuals/"+ filename,
        dataType:'json',
        success: function(data){  
            $("#individuals-screen tr").remove(); 

             let object = JSON.parse(data.json);
             if (object.length == 0)
             {
                alert("No Individuals in " + filename + ".\nHowever, you can add some if you like!");


                $('#getDSelector').empty();
                $('#getASelector').empty();

                addDefaultTextToOtherViews();   


             } 
             else 
             {

                $('#getDSelector').empty();
                $('#getASelector').empty();

               for(var i = 0; i < object.length; i++) 
               {
                    var obj = object[i];
                    var givenName = object[i].givenName;
                    var surname = object[i].surname;
                    var sex = object[i].sex;
                    var numOfFamily = object[i].numOfFamily;
                    addRowToIndividualsView(givenName, surname, sex, numOfFamily);


                    let personJson = JSON.stringify(object[i]);


                    addRowToOtherViews(givenName, surname, sex, numOfFamily, personJson);
                }
             }  
        }
        });
}

function addDefaultTextToOtherViews()
{

      var selectD = document.getElementById("getDSelector");
      var selectA = document.getElementById("getASelector");
     
      
     var  option1 = document.createElement( 'option' );
      option1.value = "ERROR";
      option1.text = "NO INDIVIDUALS IN SELECTED FILE";
     
      selectD.add( option1 );

      var  option2 = document.createElement( 'option' );
      option2.value = "ERROR";
      option2.text = "NO INDIVIDUALS IN SELECTED FILE";
     
      selectA.add( option2 );
      //selectA.add( option );

}



    
function addRowToOtherViews(givenName, surname, sex, numOfFamily, personJson)
{
    console.log("DOES THIS HAPPEN");
    console.log(personJson);

      var selectD = document.getElementById("getDSelector");
      var selectA = document.getElementById("getASelector");

     
      
     var  option1 = document.createElement( 'option' );
      option1.value = personJson
      option1.text = givenName + " " + surname + "   (SEX: " + sex + ")  Found in " + numOfFamily + " families" ;
     
      selectD.add( option1 );

      var  option2 = document.createElement( 'option' );
      option2.value = personJson;
      option2.text = givenName + " " + surname + "   (SEX: " + sex + ")  Found in " + numOfFamily + " families" ;
     

      selectA.add( option2 );












}



function selectedIndividual(element)
{
    alert("You selected individual index: " + element.rowIndex);
}
