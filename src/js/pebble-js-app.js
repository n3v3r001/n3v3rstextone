var initialized = false;

Pebble.addEventListener("ready", function(){
  console.log("JS: ready called!");
  initialized = true;
});

//Create and open the Config URL
Pebble.addEventListener("showConfiguration", function(){
  console.log("URL: creating configuration URL");

  //default URL
  var url = 'http://n3v3r001.at/pebble/configs/n3v3rs_text_one_dev.html?';

  //starting with first key
  var firstKey = true; 

  //loop through all keys
  for(var i = 0; i < localStorage.length; i++){
    var key = localStorage.key(i);
    var val = localStorage.getItem(key);

    //add "&" between every key
    if(val !== null) {
      if (!firstKey) {
        url += "&";
      } else {
        firstKey = false;
      }
      //create new url (add keys to url to transfer current configuration)
      url += encodeURIComponent(key) + "=" + encodeURIComponent(val);
    }
  }

  //open the newly created url
  console.log("URL: open the URL - "+url);
  Pebble.openURL(url);
});

//apply the new config to pebble
Pebble.addEventListener("webviewclosed", function(e) {
  console.log("Trace: webviewclosed");

  var options = JSON.parse(decodeURIComponent(e.response));
  for(var key in options) {
    localStorage.setItem(key, options[key]);
  }
  console.log("Trace: Options Recorded - " + JSON.stringify(options));

  var dict = {
    0: options.KEY_INVERTED,
    1: options.KEY_BLUETOOTH,
    2: options.KEY_VIBE,
    3: options.KEY_BATT_IMG,
    4: options.KEY_TEXT_NRW,
    5: options.KEY_TEXT_WIEN,
    6: options.KEY_DATE
  };

  console.log("Trace: Dict Sending - " + JSON.stringify(dict));

  Pebble.sendAppMessage(dict,
                        function(e) {
                          console.log("Trace: Options Sent Successfully");
                        },
                        function(e) {
                          console.log("Trace: Failed to Send Options. \nError: " + e.error.message);
                        });
});