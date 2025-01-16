let dots = {};                                                                          // Initialize an empty object for dots
let movement = {};                                                                      // Initialize an empty object for movement
const field = document.getElementById('field')                        // Get the ID of the band field
let paused = false;                                                                 // Initialize the pause state as false
let playing = false;                                                                // Initialize the playing state as false
let recording1 = [];                                                                  // Initialize an empty list for recording 1 data
let recording2 = [];                                                                  // Initialize an empty list for recording 2 data
let currentIndex = {recording1: 0, recording2: 0};          // Initialize the current index for both recordings to 0
let dotIndex = {}                                                                       // Initialize an empty object for dot index
let horizontal_offset = 200;                                                        // Initialize the horizontal offset (200 pixel margin)

function PlayButton(){                                                                  // Define a function for the play button
    const pause_continue = document.getElementById('PauseContinue');    // Get the id of the pause/continue button
    if(!playing){                                               // If "playing" state is false
        playing = true;                                         // Set "playing" to true
        paused = false;                                         // Set "paused" to false

        pause_continue.disabled = false;                        // Set the button to false state
        pause_continue.textContent = "Pause";                   // Set the button text to "Pause"
    }
    else{                                                       // If "playing" state is true
        paused = false;                                         // Set "paused" to false
        pause_continue.textContent = "Pause";                   // Set the button text to "Pause"
    }

    const recording1Id = document.getElementById('attempt1').value;                 // Get the ID for the first attempt
    const recording2Id = document.getElementById('attempt2').value;                 // Get the ID for the second attempt

    if(!recording1Id || !recording2Id){                                     // If 2 recordings are not selected
        console.log('Not enough recordings have been selected.');           // Log that there are not enough recordings
    }
    else{                                                                   // If 2 recordings are selected
        Start_DisplayRecording(recording1Id, recording2Id);                 // Call the displaying function passing in both recordings IDs
    }
}

function Start_DisplayRecording(recording1Id, recording2Id){            // Define the Start Displaying function that takes in the 2 recording IDs
    ClearDot();                                                               // Clear any previous dots

    fetch(`/BandField/recording_data/?recording1=${recording1Id}&recording2=${recording2Id}`)       // Retrieve the recording-data view sending the two IDs to the server side
        .then(response => response.json())                          // Parse the data as JSON
        .then(data =>{                                                        // Go through the data
            recording1 = data.recording1;                                     // Store the recording 1 data
            recording2 = data.recording2;                                     // Store the recording 2 data

            if(!Array.isArray(recording1) || recording1.length===0){          // If recording 1 is not in array format or has zero length
                console.log("Invalid data format for recording 1: ", recording1);     // Log the error of recording 1
                return;                                                       // Return nothing
            }
            if(!Array.isArray(recording2) || recording2.length===0){          // If recording 2 is not in array format or has zero length
                console.log("Invalid data format for recording 2: ", recording2);     // Log the error of recording 2
                return;                                                       // Return nothing
            }

            recording1.forEach((point, index) =>{               // Iterate through point object that contains username and position and iterate through the index
                DotCreation({x: point.coordinates[0][0], y:point.coordinates[0][1]}, 'recording1', index, point.username);          // Creating the dot for the marcher in recording 1 with the first location
            });
            recording2.forEach((point, index) =>{                           // Iterate through point object that contains username and position and iterate through the index
                DotCreation({x: point.coordinates[0][0], y: point.coordinates[0][1]}, 'recording2', index, point.username);         // Creating the dot for the marcher in recording 2 with the first location
            });

            DotMovement(recording1, 'recording1');          // Call the function to deal with moving marchers in recording 1
            DotMovement(recording2, 'recording2');          // Call the function to deal with moving marchers in recording 2
        });
}

function TogglePauseContinue(){                                                             // Define function to toggle the pause/continue button
    const pause_continue = document.getElementById('PauseContinue');        // Get the ID for the button
    if (paused){                                                                                  // Check if the current state of paused is true
        paused = false;                                                                           // Change paused state to false
        pause_continue.textContent = "Pause";                                                     // Set the button text to Pause

        Object.keys(movement).forEach(key => {                                              // Iterate through each recording
            const data = key.includes('recording1') ? recording1: recording2;               // Select which recording to use starting with recording 1
            DotMovement(data, key);                                                               // Call the moving dot function to start moving the dots in recording X
        });
        Object.values(dots).forEach(dot => {                                                      // Iterate through all the dots
            dot.style.animationPlayState = 'running';                                             // Resume the CSS animations
        });
    }
    else{                                                                                         // Otherwise the current state of paused is false
        paused = true;                                                                            // Change paused state to true
        pause_continue.textContent = "Continue";                                                  // Set the button text to Continue

        Object.keys(movement).forEach(key => {                                              // Iterate through the recordings
            clearInterval(movement[key]);                                                          // Stop the movement of all the dots
        });
        Object.values(dots).forEach(dot => {                                                        // Iterate through all the dots
            dot.style.animationPlayState = 'paused';                                                // Pause the CSS animations
        });
    }
}

function DotCreation(point, recording, index, username_object){                             // Define function DotCreation to create dots that takes in parameters
    let username = username_object.username                                                       // Get the username from the username object

    const dotId = `${recording}-${username}-${index}`;                                      // Create an ID for the dot that is labeled with the recording#, username, and index
    if(!dots[dotId]){                                                                              // If the dot doesn't already exist
        const dot = document.createElement('div');                         // Create a new div element
        dot.classList.add('dot', recording);                                                       // Add a class to the dot
        dot.id = dotId;                                                                            // Set the dot ID
        RandomMovement(point.x, point.y, dot);                                                     // Displaying the dots location
        field.appendChild(dot);                                                                    // Adding the dot to the field
        dots[dotId] = dot;                                                                         // Store the dot
        dotIndex[dotId] = 0;                                                                       // Initialize the dot index
    }
}

function DotMovement(Data, recording) {                                                     // Define the function DotMovement that takes in parameters Data that has the marchers and positions and the recording
    const interval = setInterval(() => {                                    // Set an interval to move the dot after a certain amount of time
        let recording_end = true;                                                        // Flag to see if the recording has ended
        Data.forEach((point, index) => {                                                    // Iterating through each point in data and for each index #
            const dotId = `${recording}-${point.username.username}-${index}`;              // Match the ID make in DOtCreation
            const dot = dots[dotId];                                                             // Get the dot with the ID
            if (!dot) {                                                                           // If dot doesn't exist
                console.log("Dot not found: ", dotId);                                           // Log that the dot wasn't found
                return;                                                                           // Return nothing
            }

            if(!(dotId in dotIndex)){                                                           // If the dot's ID isn't in the index of dots
                dotIndex[dotId] = 0;                                                            // Set the index to zero
            }

            const dot_Index = dotIndex[dotId];                                                  // Dot's current position index
            if (dot_Index < point.coordinates.length) {                                         // Check that the index is within the coordinate range
                const [x, y] = point.coordinates[dot_Index];                                    // Get the x and y coordinates for the dot
                RandomMovement(x, y, dot);                                                      // Move the dot to its new position
                dotIndex[dotId]++;                                                              // Move to the next position
                recording_end = false;                                                          // Flag to indicate recording is still happening
            }
        });

        if (recording_end) {                                                                    // If the recording has ended
            clearInterval(interval);                                                            // Clear the interval
            console.log("Movement has ended for recording :", recording);                       // Log the ending of the recording
            delete movement[recording];                                                         // Remove the recording from movement
        }
    }, 1000);                                                                           // The moving time is a second
    movement[recording] = interval;                                                             // Get the interval index to be able to pause the recording
}

function ClearDot(){                                                                // Define function that will delete the dots from the field
    Object.values(dots).forEach(dot => {                                                  // Iterate through all the dots on the field
        field.removeChild(dot);                                                           // Remove the dots on the field
    });
    dots = {};                                                                            // Reset the dots object

    Object.values(movement).forEach(interval => {                                         // Iterate through all the movement intervals
        clearInterval(interval);                                                          // Clear all intervals to stop movement
    });
    movement = {};                                                                        // Reset the movement object
    currentIndex = {recording1: 0, recording2: 0};                                        // Reset the current recordings index
}

function ShowMembers(){                                                     // Define a function that toggles revealing a people's catalog
    let x = document.getElementById("members_list");        // Get the id of the div class members_list
    if (x.style.display === "none"){                                              // Check if the members_list is being displayed
        x.style.display = "block";                                                // Display the members_list
    }
    else{                                                                         // Otherwise
        x.style.display = "none";                                                 // Hide the display of members_list
    }
}

function ToggleMarchers(username, checked){                                 // Define the function ToggleMarchers that takes in a username and the checkbox status
    ['recording1', 'recording2'].forEach(recording => {                    // Iterate through recording1 and recording2
        const marcher_dot = Object.keys(dots).filter(dotId => {    // Gets all the dot IDs from dots
            const username_split = dotId.split('-');             // Split the ID into components (recording, username, and index)
            const dotUsername = username_split[1];                         // Get the username component
            return dotId.startsWith(`${recording}-`) && dotUsername === username;       // Return true if the dot is in the recording and matches the username
        });

        marcher_dot.forEach(dotId => {                                  // Iterate through each dot ID
            const dot = dots[dotId];                                           // Get the dot that matches the ID
            if(dot){                                                           // If the dot exists
                dot.style.visibility = checked ? 'visible' : 'hidden';                // If checked is true then set the display to block but if false then set the display to none
            }
        });
    });
}

function RandomMovement(x, y, dot){                                                    // Define function RandomMovement that takes in parameters marcher and dot
    const field_pixels = document.querySelector('.field-photo-space')      // Get the size of the field
    const field_width = field_pixels.offsetWidth;                                   // Get the field width
    const field_height = field_pixels.offsetHeight;                                 // Get the field height

    const menu_pixels = document.querySelector('.button-section')          // Get the size of the menu
    const menu_height = menu_pixels.offsetHeight;                                   // Get the menu height

    const title_pixels = document.querySelector('.title')                  // Get the size of the title
    const title_height = title_pixels.offsetHeight;                                 // Get the height of the title

    let dot_dimension = 10 * field_width / 1203;                                     // Change the dimension of the dot based on page size
    dot.style.width = `${dot_dimension}px`;                                                 // Change the width of the dot
    dot.style.height = `${dot_dimension}px`;                                                // Change the height of the dot

    x_direction = x * (field_width / 1220) + horizontal_offset;                                            // Equation for the marcher's x direction for different field sizes
    y_direction = y * (field_height/631.41) + menu_height + title_height + 50;                             // Get the y coordinate of the current marcher


    // Case 1: x out of bounds left
    if(x_direction < horizontal_offset){
        x_direction = horizontal_offset;                                            // If Case 1: the x direction becomes the horizontal offset (200)
    }
    // Case 2: x out of bounds right
    if(x_direction > field_width + horizontal_offset - dot_dimension){
        x_direction = field_width + horizontal_offset - dot_dimension;              // If Case 2: the x direction becomes the field width and the horizontal offset minus the dot's dimensions
    }
    // Case 3: y out of bounds up
    if(y_direction < menu_height + title_height + 50){
        y_direction = menu_height + title_height + 50;                                   // If Case 3: the y direction becomes the menu height plus the title height plus the margin
    }
    // Case 4: y out of bounds down
    if(y_direction > menu_height + title_height + field_height - dot_dimension + 50){
        y_direction = menu_height + title_height + field_height - dot_dimension + 50;    // If Case 4: the y direction becomes the menu height plus the title height plus the field height minus dot's dimensions plus the margin
    }

    // Setting the new position
    dot.style.left = `${x_direction}px`;                                        // Move the current marcher's dot from the left accordingly
    dot.style.top = `${y_direction}px`;                                         // Move the current marcher's dot from the top accordingly
}