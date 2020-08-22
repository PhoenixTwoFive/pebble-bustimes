function cutPadString(string, length) {
    let outstring = ""
    if (string.length > length) {
        outstring = string.substring(0, length - 3) + "..."
    } else {
        if (string.length < length) {
            outstring = string.padEnd(length)
        } else {
            outstring = string
        }
    }
    return outstring
}

function getTime(stringTime) {
    let d = new Date(stringTime)
    return String(d.getHours()).padStart(2, "0") + ":" + String(d.getMinutes()).padStart(2, "0")
}

function diff_minutes(date) {

    dt = new Date(date);

    var diff = (dt.getTime() - Date.now()) / 1000;
    diff /= 60;
    return Math.abs(Math.round(diff));

}

function locationSuccess(pos) {
    console.log("Location Found!")
    // Construct URL
    var xmlhttp = new XMLHttpRequest(); // new HttpRequest instance 
    var url = 'https://simple-hafas.lab.phillipkuehne.de/departures/nearest'
    // Send request to OpenWeatherMap
    xmlhttp.open("POST", url,true);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.onreadystatechange = function () {
        if (xmlhttp.readyState === 4 && xmlhttp.status === 200) {
            var json = JSON.parse(xmlhttp.responseText);
            var timesString = "";
            json.departures.forEach(departure => {
                let delay = ""
                if (departure.delay != 0) {
                    if (departure.delay>0) {
                        delay = "(+" + departure.delay + ")"
                    } else {
                        delay = "(" + departure.delay + ")"

                    }
                }


                let timePart = ""
                if (diff_minutes(departure.time)>15) {
                    timePart = getTime(departure.time)
                } else {
                    timePart = diff_minutes(departure.time) + "min"
                }
                
                timesString += (cutPadString(departure.line.toUpperCase(), 4) + " " + cutPadString(departure.direction, 14) + "\n" + cutPadString(timePart, 6)  + delay + "\n-----------------------\n")
            });
                
                
            // Assemble dictionary using our keys
            var dictionary = {
                'NAME': json.name,
                'NUM' : json.departures.length,
                'TIMES': timesString,
            };

            // Send to Pebble
            Pebble.sendAppMessage(dictionary,
                function (e) {
                    console.log('Departure times sent to Pebble successfully!');
                },
                function (e) {
                    console.log('Error sending Departure times to Pebble!');
                }
            );
        }
        // Listen for when an AppMessage is received
        Pebble.addEventListener('appmessage',
        function (e) {
                console.log('AppMessage received!');
                getTimes();
            }
        );
    };
    xmlhttp.send(JSON.stringify({
        "lat": pos.coords.latitude,
        "lon": pos.coords.longitude
    }));
    
}




function locationError(err) {
    console.log('Error requesting location!');
}

function getTimes() {
    navigator.geolocation.getCurrentPosition(
        locationSuccess,
        locationError, {
            timeout: 15000,
            maximumAge: 60000
        }
    );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
    function (e) {
        console.log('PebbleKit JS ready!');

        // Get the initial data
        getTimes();
    }
);