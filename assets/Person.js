let dot;                                                                        // Initialize dot
let horizontal_offset = 200;                                            // Initialize the dot to 200 pixels (margin from CSS)
let x_direction = 0;                                                    // Initialize the x-coordinate to 0
let y_direction = 0;                                                    // Initialize the y-coordinate to 0

let isRecording = false;                                                // Initialize the recording check to start as not recording
let recordingData = [];                                                   // Initialize an empty list for the recording data
let recordingSessionId;                                                         // Initialize the recordingSessionId
let recordingInterval;                                                          // Initialize the recording interval

StartBlinking();                                                                // Start blinking right when the user renders the page
setInterval(Position, 700);                                             // Call the Position function every 0.7 seconds

function StartBlinking(){                                                 // Define the function StartBlinking that deals with creating the dots
    console.log('Calling');                                                     // Log that StartBlinking has started
    const field = document.getElementById('field');       // Get the id of the band field

    marcherData.forEach(marcher => {                                            // Iterate through all the presumed marchers
        if(marcher.role === 'marcher'){                                         // Check if the current user is a marcher
            const checkbox = document.getElementById("user_"+marcher.username);         // Get the checkbox from the people's catalog
            dot = document.getElementById("dot"+marcher.username);      // Getting the dots ID from the field

            if(dot === null){                                                    // If there is no dot
                console.log("Creating dot");                                     // Log that a dot is being created for the marcher
                dot = document.createElement('div');                    // Create the dot
                dot.setAttribute("id", "dot"+marcher.username);     // Give the dot attributes to be able to access to toggle
            }

            if (checkbox.checked){                                              // Check if the marcher is toggled on (activated)
                console.log("Being enabled");                                   // Log that the dot is enabled
                dot.classList.add('dot', 'blinking');                           // Add blinking to the dot
                dot.style.visibility = 'visible';                               // Set the style of the dot to be visible
                RandomMovement(marcher, dot);                                   // Call the RandomMovement function and send parameters marcher and dot to change the move the dot
                field.appendChild(dot);                                         // Add the dot to the field
            }
            else{                                                               // Marcher is toggle off (de-activated)
                console.log('Being disabled')                                   // Log that the dot is being toggled off
                let disable_dot = document.getElementById("dot"+marcher.username)           // Getting the dot's ID
                disable_dot.style.visibility = 'hidden';                        // Hiding the dot on the field
                RandomMovement(marcher, disable_dot);                           // Still moving the dot even when the dot is toggled off
            }
        }
    });
}

function Position(){                                                                // Define function that retrieves the live feed of positions
    fetch('recent-position/', {                                                // Retrieve the view "recent-position" from client side
        method: 'GET', headers:{'Content-Type': 'application/json', 'X-CSRFToken': csrf_token}         // Accessing the view and sending over no data
    })
        .then(response => response.json())                                      // Parse the JSON response from the server
        .then(data => {                                                                   // Going through the data
            console.log("Updating the position data:", data.marcher_data);               // Log the marcher that is getting updated
            marcherData = data.marcher_data                                              // Set marcherData to hold the data from the marcher
            StartBlinking();                                                             // Begin the start blinking function
        })
}

function StartRecording(){                                                          // Define function that starts the recording process
    console.log("Recording started");                                                     // Log that the recording has begun
    if (isRecording) {                                                                    // Check if there is already a recording running
        console.log("Recording is already running");                                      // Log that there is already a recording in progress
        return;                                                                           // Return nothing
    }

    isRecording = true;                                                                   // Change state to indicate a recording has begun
    recordingData = [];                                                                   // Initialize an empty list where data will populate in

    fetch('/BandField/start-recordings/', {                                     // Retrieve the view "start-recording" from client side
        method: 'POST', headers: {'Content-Type': 'application/json', 'X-CSRFToken': csrf_token}, body: JSON.stringify({})      // Being able to get access to the view and sending over no data
    })
        .then(response => response.json())                          // Parse the JSON response from the server
        .then(data => {                                                      // Going through the data
            console.log("Recording has started for session: ", data.start_session);        // Log the session that the recording is in
            recordingSessionId = data.session_id;                                          // Set the recording session id to match the session id of the response data
        })
        .catch(error => console.error("Issue with starting the recording ", error));       // Error log

        recordingInterval = setInterval(() => {                               // Start recording the data at intervals
            marcherData.forEach(marcher => {                                               // Iterate through all the members
                if(marcher.role ==='marcher'){                                             // Check if the member is a marcher
                    if (recordingSessionId){                                               // Check if the session id is valid
                        const currentPosition = {                                     // Create the object where data will be stored from the recording
                            session_id: recordingSessionId,                                // Store the session id
                            username: marcher.username,                                    // Store the marcher's username
                            x: marcher.x_coordinate,                                       // Store the marcher's x-coordinate
                            y: marcher.y_coordinate,                                       // Store the marcher's y-coordinate
                            timestamp: new Date().toISOString()                            // Store the timestamp
                        };
                        recordingData.push(currentPosition);                               // Populate the recording data list with the position of the marcher at time X
                    }
                }
            });
        }, 1000);                                                                  // Get the marcher's position every second
        setTimeout(StopRecording, 60000);                                          // After 60 seconds stop recording
}

function StopRecording(){                                                 // Define function that stops recordings
    console.log("Recording has stopped");                                       // Log the end of the recording
    if(!isRecording){                                                           // Check if there is a recording to stop
        console.log("No recording available to stop");                          // Log that there is no running recording
        return;                                                                 // Return nothing if there isn't a recording to stop
    }

    isRecording = false;                                                        // Recording has stopped
    clearInterval(recordingInterval);                                           // Clear the interval that was running when starting the recording

    fetch('/BandField/stop-recordings/', {                            // Fetch the view stop-recordings from client side
        method: 'POST', headers: {'Content-Type': 'application/json', 'X-CSRFToken': csrf_token},       // Being able to get access to the view
        body: JSON.stringify({session_id: recordingSessionId, data: recordingData})               // Convert to JSON strong to send as the request sending the session id and the recording data
    })
        .then(response => response.json())                                                     // Parse the JSON response from the server
        .then(data => {console.log("Recording has stopped and the data has been saved: ", data);        // Log the data that was saving as a recording
        });
}

function RandomMovement(marcher, dot){                                                 // Define function RandomMovement that takes in parameters marcher and dot
    const field_pixels = document.querySelector('.field-photo-space')      // Get the size of the field
    const field_width = field_pixels.offsetWidth;                                   // Get the field width
    const field_height = field_pixels.offsetHeight;                                 // Get the field height

    const menu_pixels = document.querySelector('.button-section')          // Get the size of the menu
    const menu_height = menu_pixels.offsetHeight;                                   // Get the menu height

    const title_pixels = document.querySelector('.title')                  // Get the size of the title
    const title_height = title_pixels.offsetHeight;                                 // Get the height of the title


    let dot_dimension = 10 * field_width / 1203;                                    // Change the dimension of the dot based on page size
    dot.style.width = `${dot_dimension}px`;                                                 // Change the width of the dot
    dot.style.height = `${dot_dimension}px`;                                                // Change the height of the dot


    x_direction = marcher.x_coordinate * (field_width / 1203) + horizontal_offset;                      // Equation for the marcher's x direction for different field sizes
    y_direction = marcher.y_coordinate * (field_height/622.61) + menu_height + title_height;            // Equation for the marcher's y direction for different field sizes


    // Case 1: x out of bounds left
    if(x_direction < horizontal_offset){
        x_direction = horizontal_offset;                                            // If Case 1: the x direction becomes the horizontal offset (200)
    }
    // Case 2: x out of bounds right
    if(x_direction > field_width + horizontal_offset - dot_dimension){
        x_direction = field_width + horizontal_offset - dot_dimension;              // If Case 2: the x direction becomes the field width and the horizontal offset minus the dot's dimensions
    }
    // Case 3: y out of bounds up
    if(y_direction < menu_height + title_height){
        y_direction = menu_height + title_height;                                   // If Case 3: the y direction becomes the menu height plus the title height
    }
    // Case 4: y out of bounds down
    if(y_direction > menu_height + title_height + field_height - dot_dimension){
        y_direction = menu_height + title_height + field_height - dot_dimension;    // If Case 4: the y direction becomes the menu height plus the title height plus the field height minus dot's dimensions
    }

    // Setting the new position
    dot.style.left = `${x_direction}px`;                                        // Move the current marcher's dot from the left accordingly
    dot.style.top = `${y_direction}px`;                                         // Move the current marcher's dot from the top accordingly
}

function ShowMembers(){                                                     // function to show members catalog
    let x = document.getElementById("members_list");        // Get the id of the div class members_list
    if (x.style.display === "none"){                                              // Check if the members_list is being displayed
        x.style.display = "block";                                                // Display the members_list
    }
    else{                                                                         // Otherwise
        x.style.display = "none";                                                 // Hide the display of members_list
    }
}

/*
window.onbeforeunload = function (){                                    // Before loading the next html
    fetch('/BandField/stop_watchdog/')                                  // Retrieve the stop_watchdog function
        .then(response => response.json())                          // Stop watchdog from running
}
*/