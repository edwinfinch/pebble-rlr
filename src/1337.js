Pebble.addEventListener("ready",
                        function(e) {
							console.log("connect: " + e.ready);
							console.log(e.type);
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
							console.log(e.type);
							console.log(e.payload.temperature);
							console.log("message");
                        });

var values = 0;
var config_url = "http://edwinfinch.github.io/config-ruler";
var options = JSON.parse(localStorage.getItem('simplycleanops'));

Pebble.addEventListener("showConfiguration", function(e) {
	if (localStorage.getItem("simplycleanops") !== null) {
        options = JSON.parse(localStorage.simplycleanops);
    }
        Pebble.openURL(config_url);
        console.log(config_url);
});

Pebble.addEventListener("webviewclosed", function (e) {
    console.log("Configuration closed");
    console.log("Response = " + e.response.length + "   " + e.response);
    if (e.response) { // user clicked Save/Submit, not Cancel/Done
		console.log("User hit save");
		values = JSON.parse(decodeURIComponent(e.response));
		console.log("stringified options: " + JSON.stringify((values)));
		for(var key in values) {
			localStorage.setItem(key, values[key]);
		}
		
		
		Pebble.sendAppMessage(
			values
		);
	}
});