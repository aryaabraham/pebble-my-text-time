var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var xhrPost = function (url, type, params, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  xhr.setRequestHeader("Content-length", params.length);
  xhr.setRequestHeader("Connection", "close");
  xhr.send(params);
};


function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

function locationSuccess(pos) {
  // Construct URL
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = '' + Math.round((json.main.temp - 273.15)*9/5+32) + '\u00B0F';
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log('Conditions are ' + conditions);
      
      var id = json.weather[0].id;
      console.log('Id is ' + id + ' ' + iconFromWeatherId(id));

      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_WEATHER_ICON': iconFromWeatherId(id),
        'KEY_WEATHER_TEMPERATURE': temperature,
      };
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );

      
    }      
  );
}

function iconFromWeatherId(weatherId) {
  if (weatherId < 600) {
    return 2; //RAIN
  } else if (weatherId < 700) {
    return 3; //SNOW
  } else if (weatherId > 800) {
    return 1; //CLOUD
  } else {
    return 0; //SUNNY
  }
}

var g_device_code;
var g_access_token;
var g_refresh_token;

function getCalendarItem()
{
////https://developers.google.com/identity/protocols/OAuth2ForDevices
  // Construct URL
  var url = 'https://accounts.google.com/o/oauth2/device/code'; 
  var params = 'client_id=675931514419-q1ii3bckisthjkremho7v9bns70loek1.apps.googleusercontent.com&scope=https://www.googleapis.com/auth/calendar.readonly';

  // Authenticate (first time only!)
  xhrPost(url, 'POST', params,
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log('URL auth response: ' + responseText);
      
      g_device_code = json.device_code;
    }      
  );
  
  // Construct URL
  url = 'https://accounts.google.com/o/oauth2/device/code'; 
  params = 'client_id=675931514419-q1ii3bckisthjkremho7v9bns70loek1.apps.googleusercontent.com&client_secret=5yXaOJxsuhSx_4RCXRXR5fAo&code=' + g_device_code + '&grant_type=http://oauth.net/grant_type/device/1.0';

  // Wait for key
  xhrPost(url, 'POST', params,
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      
      g_access_token = json.access_token;
      g_refresh_token = json.refresh_token;
      console.log('URL key response: ' + responseText);
    }      
  );  
      /*
      // Temperature in Kelvin requires adjustment
      var temperature = '' + Math.round((json.main.temp - 273.15)*9/5+32) + '\u00B0F';
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log('Conditions are ' + conditions);
      
      var id = json.weather[0].id;
      console.log('Id is ' + id + ' ' + iconFromWeatherId(id));

      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_WEATHER_ICON': iconFromWeatherId(id),
        'KEY_WEATHER_TEMPERATURE': temperature,
      };
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );
*/
      

}

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
    //getCalendarItem();
  }                     
);

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Get the initial weather
    getWeather();
    //getCalendarItem();
  }
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('https://www.google.com/');
});

////////
/*
//Open pebble dev account //login to cloudpebble.net //create new SimpleJS project //Fill in whatever you want the app called on your pebble and whatnot. I've been using "Transit Direct," but who cares. //Paste this code into the app.js file. //Add destinations into the variable below. //Turn on Directions API and get an API key from google // ** https://code.google.com/apis/console ** // Paste the API key into the API_KEY // Either publish to the pebble via direct connection or just "build" and email yourself the pbw. // When the app installs, a config screen will pop up. Click "save". This is a bug in the CloudPebble SimpleJS implementation. You should only have to do this once.

simply.text({title:"Transit", subtitle:"Direct", body:"Loading..." });

var DESTINATIONS = [{ "short":"Work", "full":"Your Address Here" } ]; // More destinations here 
var API_KEY = "YOUR API KEY HERE";

var directionsData = null; var origin = null; var depth = 0; var nodes = [0,0,0]; //Destination, Route, Step 
var networkSemiphore = false; var buttons = { back:false, up:true, select:true, down:true };

function apiUrlWithParams(){
  return "https://maps.googleapis.com/maps/api/directions/json?origin=" + origin + "&destination=" + DESTINATIONS[nodes[0]].full.replace(/ /g,"+") + "&sensor=false&key=" + API_KEY + "&mode=transit&alternatives=true&departure_time=" + Math.floor(Date.now()/1000); 
}

function fetchAndDraw(){ 
  if(depth !== 0 && directionsData === null)
  { 
    networkSemiphore = true; 
    simply.text({title:"Getting Directions...",subtitle:"",body:""}); 
    console.log(apiUrlWithParams()); 
    ajax(
      { url: apiUrlWithParams(), type: 'json' }, 
      function(data) { directionsData = data; networkSemiphore = false; draw(); },function(error){ simply.text({title:"Could not get directions.",subtitle:"",body:"Press Back."}); }); } else{ console.log("Skipping Directions fetch."); networkSemiphore = false; draw(); } }
*/