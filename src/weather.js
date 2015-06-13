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
      console.log(responseText);
      
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

function getCalendarItem() {
  // Get persistent data
  var google_access_token = localStorage.getItem(1);
  
  var now = new Date(Date.now());
  var nowplus1day = new Date(Date.now()+(24*60*60*1000));  // one day later
  
  // Construct URL
  var url = 'https://www.googleapis.com/calendar/v3/calendars/primary/events?fields=items(summary%2Cstart)&showDeleted=false&maxResults=100&singleEvents=true&timeMin=' + now.toISOString() + '&timeMax=' + nowplus1day.toISOString() + '&access_token=' + encodeURIComponent(google_access_token);
  //console.log(url);
  
  // Send request to Google API
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      //console.log(responseText);
      json.items.sort(function(a, b){return Date.parse(a.start.dateTime)-Date.parse(b.start.dateTime);});
      //console.log(JSON.stringify(json));
      
      // Get the summary, description and date time
      var start = new Date(json.items[0].start.dateTime);
      var summary = start.getHours() + ':' + start.getMinutes() + '-' + json.items[0].summary;
      console.log('Next calendar event: ' + summary);

      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_CALENDAR_SUMMARY': summary,
      };
        
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
                            function(e) {
                              console.log('Calendar info sent to Pebble successfully!');
                            },
                            function(e) {
                              console.log('Error sending calendar info to Pebble!');
                            }
                           );

    }      
  );
}

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
    getCalendarItem();
  }                     
);

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    // Get the initial weather
    getWeather();
    getCalendarItem();
  }
);

Pebble.addEventListener('showConfiguration', 
  function(e) {
    // Show config page
    Pebble.openURL('http://www.aryaservices.website/pebbletexttime.htm');
  }
);

Pebble.addEventListener('webviewclosed',
  function(e) {
    //console.log('Configuration window returned: ' + e.response);
    var json = JSON.parse(e.response);
    var google_access_token = json.access_token;
    localStorage.setItem(1, google_access_token);
    console.log('New access token: ' + google_access_token);
    getCalendarItem();
  }
);