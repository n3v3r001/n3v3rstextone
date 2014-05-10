var initialized = false;

Pebble.addEventListener("ready", function() {
	console.log("ready called!");
	initialized = true;
});

Pebble.addEventListener("showConfiguration", function() {
	console.log("showing configuration");

	var url = 'http://n3v3r001.at/pebble/configs/n3v3rs_text_one.html?';
	var firstParam = true;

	for(var i = 0, x = localStorage.length; i < x; i++) {
			var key = localStorage.key(i);
			var val = localStorage.getItem(key);

			if(val !== null) {
				if (!firstParam) {
					url += "&";
				} else {
					firstParam = false;
				}
				url += encodeURIComponent(key) + "=" + encodeURIComponent(val);
			}
		}

	Pebble.openURL(url);
});

Pebble.addEventListener("webviewclosed", function(e) {
	console.log("Trace: Configuration Closed");

		var options = JSON.parse(decodeURIComponent(e.response));
		for(var key in options) {
			localStorage.setItem(key, options[key]);
		}
		console.log("Trace: Options Recorded - " + JSON.stringify(options));

  var dict = { 0: options.KEY_INVERTED, 1: options.KEY_BLUETOOTH, 2: options.KEY_VIBE, 3: options.KEY_BATT_IMG, 4: options.KEY_TEXT_NRW, 5: options.KEY_TEXT_WIEN };
		console.log("Trace: Dict Sending - " + JSON.stringify(dict));

		Pebble.sendAppMessage(dict,
								function(e) {
									console.log("Trace: Options Sent Successfully");
								},
								function(e) {
									console.log("Trace: Failed to Send Options. \nError: " + e.error.message);
								});
});
