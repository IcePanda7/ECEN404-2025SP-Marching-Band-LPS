from django.shortcuts import render                                         # To render pages
from django.contrib.auth.decorators import login_required                   # To make users login
from django.contrib.auth.models import User                                 # Retrieve the User model
from django.views.decorators.csrf import csrf_exempt

from accounts.models import Position                                        # Retrieve the Position model
import os                                                                   # To access the data file
from django.conf import settings                                            # To get the location of the data file
import json                                                                 # To handle JSON data
import subprocess                                                           # To retrieve Wi-Fi

from .models import Recordings                                              # Retrieve the Recordings model
from threading import Thread                                                # To run concurrent tasks
import time                                                                 # To incorporate delays
from django.http import JsonResponse                                        # To send JSON responses
from django.utils import timezone                                           # To get the timezone date and times

import sys                                                                  # To access functions for watchdog
watchdog_process = None                                                     # Initiate watchdog

# Create your views here.
@login_required(login_url="/accounts/login/")                               # Adding login required
def band_field(request):                                                    # Defining band_field that takes in a user request
    directors = User.objects.filter(profile__role='director')               # Get all the directors
    marchers = User.objects.filter(profile__role='marcher')                 # Get all the marchers
    marcher_data=[]                                                         # Initialize empty list
    global watchdog_process

    for director in directors:                                              # Append all director parameters in an object with default parameters
        marcher_data.append({'username': director.username, 'x_coordinate': 0,                                          # Adds the director's username, x and y coordinates, first name, last name, instrument, and role to the marcher_data list
                             'y_coordinate': 0, 'first_name': director.first_name,
                             'last_name': director.last_name, 'instrument': director.profile.instrument,
                             'role': director.profile.role})


    file_location = os.path.join(settings.BASE_DIR, 'assets', 'data.txt')       # Get the file location
    if os.path.exists(file_location):                                           # Check if the file exists
        with open(file_location, "r") as file:                                  # Open file to read
            for marcher in marchers:                                            # Iterate all the marchers in the file
                position, created = Position.objects.get_or_create(user=marcher, defaults={'x_coordinate': 0, 'y_coordinate': 0})    # Check if the marcher has a position model if not create it
                file.seek(0)                                                    # Go to the start of the file
                for line in file:                                               # Iterate line be line of the file
                    data = line.split()                                         # Store the line of data in variable
                    if data[0] == marcher.username:                             # Check if the username matched
                        position.x_coordinate = float(data[1])                  # Store the second component as the marcher's x-coordinate
                        position.y_coordinate = float(data[2])                  # Store the third component as the marcher's y-coordinate
                        position.save()                                         # Save positions to the database
                        break                                                   # Go to the next marcher
                    else:                                                       # Otherwise
                        continue                                                # Continue going through the file
                marcher_data.append({'username': marcher.username, 'x_coordinate': position.x_coordinate,       # Append all marcher parameters in an object with their parameters
                                     'y_coordinate': position.y_coordinate, 'first_name': marcher.first_name,
                                     'last_name': marcher.last_name, 'instrument': marcher.profile.instrument,
                                     'role':marcher.profile.role})
    else:                                                                       # Otherwise there is no file
        print('File not found')                                                 # Display error
    json_data = {"marcher_data": marcher_data, "marcher_data_json": json.dumps(marcher_data)}               # Object normal marcher data for html and dumps for JavaScript

    """
    wifi = subprocess.check_output([r'C:\Windows\System32\netsh', 'WLAN', 'show', 'interfaces'])            # Get Wi-Fi details and is raw string so backslashes are not seen as escapes
    data = wifi.decode('iso-8859-1')                                                                        # Decoding the Wi-Fi details where iso was the encoder
    for line in data.split('\n'):                                                                           # Iterate through each line
        if "SSID" in line and "BSSID" not in line:                                                          # Look for SSID and not BSSID in the line to locate the network name
            ssid = line.split(":")[1].strip()                                                               # Retrieve the network name
            print("Your WiFi is:", ssid)                                                                    # Display the network name
            break                                                                                           # Exit the loop because network name was found
        else:                                                                                               # Otherwise there is no Wi-Fi connection
            print("No WiFi found.")
    """
    """
    if (watchdog_process is None) or (watchdog_process.poll() is not None):                                 # Check if watchdog is running
        #watchdog_process = subprocess.Popen([r"C:\Users\vladi\PycharmProjects\MarchingBangGPSWeb\.venv\Scripts\python.exe", "C:/Users/vladi/PycharmProjects/MarchingBangGPSWeb/BandField/watchdog_daemon.py"])
        watchdog_process = subprocess.Popen([sys.executable, 'BandField/watchdog_daemon.py'])               # Run watchdog
    """


    if request.user.profile.role =='director':                                                              # Check if the user is a director to see which html render
        return render(request, 'BandField/band_field.html', {'json_data': json_data})   # Render the director band field
    else:                                                                                                   # Otherwise the role is marcher
        return render(request, 'BandField/bandfield.html',{'json_data': json_data})     # Render the marcher band field


@login_required(login_url="/accounts/login/")                                   # Adding login required
def recent_position(request):                                                   # Defining recent_position that takes in the user request
    marchers = User.objects.filter(profile__role='marcher')                     # Get all the marchers
    marcher_data = []                                                           # Initialize empty list
    for marcher in marchers:                                                    # Append all director parameters in an object with default parameters
        marcher_data.append({'username': marcher.username, 'x_coordinate': marcher.position.x_coordinate,               # Adding marcher's username, x and y coordinates, and role to the empty list
                             'y_coordinate': marcher.position.y_coordinate, 'role':marcher.profile.role})
    return JsonResponse({'marcher_data': marcher_data})                         # Returning a Json response of all the marchers current position for live feed

@login_required(login_url="/accounts/login/")                                   # Adding login required
def band_recordings(request):                                                   # Defining band recordings that takes in a user request
    directors = User.objects.filter(profile__role='director')                   # Get all the directors
    marchers = User.objects.filter(profile__role='marcher')                     # Get all the marchers
    marcher_data = []                                                           # Empty list for user catalog info
    for director in directors:                                                  # Populate for all directors their model traits
        marcher_data.append({'username': director.username, 'first_name': director.first_name, 'last_name': director.last_name,         # Adding username, first name, last name, instrument, and role of all the directors to the list
                             'instrument': director.profile.instrument, 'role': director.profile.role})
    for marcher in marchers:                                                    # Populate for all marchers their model traits
        marcher_data.append({'username': marcher.username, 'first_name': marcher.first_name, 'last_name': marcher.last_name,            # Adding username, first name, last name, instrument, and role of all the marchers to the list
                             'instrument': marcher.profile.instrument, 'role': marcher.profile.role})

    recordings = Recordings.objects.all()                                       # Get all the recordings
    return render(request, 'BandField/band_recordings.html', {'recordings': recordings, 'marcher_data': marcher_data})        # Render the recordings page with the recordings and the user data

@login_required(login_url="/accounts/login/")                                   # Adding login required
def recording_data(request):                                                    # Defining recording_data that takes in a user request
    recording1_id = request.GET.get('recording1')                               # Get the id for the user selected Attempt 1
    recording2_id = request.GET.get('recording2')                               # Get the id for the user selected Attempt 2

    if not recording1_id or not recording2_id:                                  # Check if both id's are valid
        return JsonResponse({'recording1': [], 'recording2': []})               # Return two empty lists for the data in the recordings

    recording1 = Recordings.objects.get(id=recording1_id)                       # The actual data for the first selected recording
    recording2 = Recordings.objects.get(id=recording2_id)                       # The actual data for the second selected recording

    recording1_cords = list(zip(recording1.AttemptX_Coordinates, recording1.AttemptY_Coordinates))      # Creating a list of x,y pairs from recording1
    recording2_cords = list(zip(recording2.AttemptX_Coordinates, recording2.AttemptY_Coordinates))      # Creating a list of x,y pairs from recording2

    members1 = len(recording1.marcher_members)                                  # Getting the member count in recording1
    members2 = len(recording2.marcher_members)                                  # Getting the member count in recording2

    def CoordinateToUser(coordinates, member_count):                            # Function to relate coordinates to marchers
        user_coordinates = [[] for pair in range(member_count)]                 # Creating a list of empty lists for each member to be populated later

        for i, coordinate in enumerate(coordinates):                            # Loop through each coordinate and provide an index for number of pairs
            user_index = i % member_count                                       # Assigning appropriate member number to their coordinate
            user_coordinates[user_index].append(coordinate)                     # Append coordinates into the member's sublist
        return user_coordinates                                                 # Return the list of now populated lists

    recording1_user_cords = CoordinateToUser(recording1_cords, members1)        # Relate all coordinates to marchers in the first recording
    recording2_user_cords = CoordinateToUser(recording2_cords, members2)        # Relate all coordinates to marchers in the second recording

    recording1_data = [{'username': marcher, 'coordinates': coordinates}                                        # Setting the usernames and coordinates in the first recording
                       for marcher, coordinates in zip(recording1.marcher_members, recording1_user_cords)]      # Linking the specific marcher to the list of all of their coordinate pairs in the first recording
    recording2_data = [{'username': marcher, 'coordinates': coordinates}                                        # Setting the usernames and coordinates in the second recording
                       for marcher, coordinates in zip(recording2.marcher_members, recording2_user_cords)]      # Linking the specific marcher to the list of all of their coordinate pairs in the second recording

    return JsonResponse({'recording1': recording1_data, 'recording2': recording2_data})             # Returning a JSONResponse for javascript of both of the recordings with their marchers and coordinates


def recording(session):                                                         # Define recording that takes in the session of the recording
    start_time = time.time()                                                    # Get the start time
    elapsed_time = 0                                                            # Initialize elapsed time
    marchers = User.objects.filter(profile__role='marcher')                     # Get all the marchers

    while session.recording and elapsed_time < 60:                              # Keep the session recording as long as the recording hasn't manually stopped and the time hasn't lapsed 60 seconds
        for marcher in marchers:                                                # Iterate through all the marchers

            x = marcher.position.x_coordinate                                   # Find the current indexed marcher's x coordinate
            y = marcher.position.y_coordinate                                   # Find the current indexed marcher's y coordinate

            session.AttemptX_Coordinates.append(x)                              # Append the x coordinate to the database for the specified recording
            session.AttemptY_Coordinates.append(y)                              # Append the y coordinate to the database for the specified recording
            session.save()                                                      # Save the recording to the database

        elapsed_time = time.time() - start_time                                 # Find the elapsed time from the current time to the time when the recording started
        time.sleep(1)                                                           # Wait one second

@login_required(login_url="/accounts/login/")                                   # Require user to be logged in otherwise send them to the login page
def start_recordings(request):                                                  # Define start_recordings that takes in a user request
    if request.method == 'POST':                                                # Check if the user wants to record
        user = request.user                                                     # Find the director who is recording

        marcher_usernames = []                                                  # Initialize usernames list
        marchers = User.objects.filter(profile__role='marcher')                 # Get all the marchers
        for marcher in marchers:                                                # Iterate through all the marchers
            marcher_usernames.append({'username': marcher.username})            # Append each marcher username to the list

        total_sessions = Recordings.objects.count()                             # Find the total number of recordings
        if total_sessions >= 10:                                                # Check to see if there are 10 or more recordings
            session_id = Recordings.objects.order_by('id').first()              # Find the recording with the smallest id number
            new_session = session_id.Attempt                                    # The recording with the smallest id number's Attempt number is the current Attempt number
            session_id.delete()                                                 # Delete the recording of the smallest id
        elif total_sessions == 0:                                               # Check to see if there are zero recordings
            new_session = 1                                                     # If so then this recording is the first attempt
        else:                                                                   # Otherwise
            latest_session = Recordings.objects.order_by('id').last()           # Find the recording with the biggest id because that would be the last recording taken place
            print(latest_session)
            new_session = latest_session.Attempt + 1                            # The attempt of the current recording will be one higher than the attempt of the latest recording

        recording_session = Recordings.objects.filter(Attempt=new_session).first()      # Find the first recording that has the new attempt number
        if recording_session:                                                   # If the recording exists
            recording_session.director = user                                   # Link the director who made the recording
            recording_session.timestamp = timezone.now()                        # Link the time when the recording was taken
            recording_session.recording = True                                  # Confirm a recording has occurred properly
            recording_session.marcher_members = marcher_usernames               # Get all the usernames of the marchers that participated in the recording
            recording_session.save()                                            # Update this recording in the database
        else:                                                                   # If the recording doesn't exist
            recording_session = Recordings.objects.create(director=user, Attempt=new_session, recording=True, marcher_members=marcher_usernames)         # Create a new recording with the appropriate parameters

        thread = Thread(target=recording, args=(recording_session, ))           # Create a new thread to run in the background concurrently to improve performance
        thread.start()                                                          # Start the tread

        return JsonResponse({'start_session': new_session, 'session_id': recording_session.id})         # Return a JSONResponse with the Attempt number and the id to the recording

@login_required(login_url="/accounts/login/")                                           # Require user to be logged in otherwise send them to the login page
def stop_recordings(request):                                                           # Define stop_recordings that takes in a request from the user
    if request.method == 'POST':                                                        # Check if the user wants to stop recording
        user = request.user                                                             # Find the director
        session = Recordings.objects.filter(director=user, recording=True).last()       # Find the last session from the user and if the recording is still going
        if session:                                                                     # If there is a session
            session.recording = False                                                   # Stop the recording
            session.save()                                                              # Save to the database
            return JsonResponse({'stop_session': session.Attempt})                      # Return the attempt that has stopped recording
        else:                                                                           # Otherwise
            return JsonResponse({})                                                     # Return nothing because there is no active recording session

"""
def stop_watchdog(request):                                                             # Define stop_watchdog that takes in a request
    global watchdog_process                                                             # Get the state of watchdog_process
    if watchdog_process:                                                                # If watchdog_process is running
        watchdog_process.terminate()                                                    # Stop watchdog
        watchdog_process.wait()                                                         # Wait for watchdog to end
        watchdog_process = None                                                         # Reset watchdog_process
    return JsonResponse({"status": "Watchdog ended"})                                   # Return that watchdog is no longer running
"""

@csrf_exempt                                                                            # Exempt the CSRF protection token
def update_positions(request):                                                          # Define function update_positions that takes in a request
    if request.method == 'POST':                                                        # Check if the request was a POST
        try:                                                                            # Try
            data = json.loads(request.body)                                             # Load the request data in variable data
            updated_positions = []                                                      # Initialize empty list

            for entry in data:                                                          # Iterate through each line
                tag_id = entry.get("tag_id")                                            # Extract the tag_id/username
                x_coordinate = entry.get("x_coordinate")                                # Extract the x_coordinate
                y_coordinate = entry.get("y_coordinate")                                # Extract the y_coordinate

                if not all([tag_id, x_coordinate, y_coordinate]):                       # Check is line has all required data
                    print("ERROR")                                                      # Print error is not all required data present
                    continue                                                            # Continue in the process of updating the Positions database

                user = User.objects.filter(username=tag_id).first()                     # Find a user with a matching username
                if user:                                                                # If a user exists
                    position, created = Position.objects.get_or_create(user=user)       # Retrieve the user's position or create a new database entry if they don't have one
                    position.x_coordinate = x_coordinate                                # Update the users x_coordinate in the database
                    position.y_coordinate = y_coordinate                                # Update the users y_coordinate in the database
                    position.save()                                                     # Save the new database

                    updated_positions.append({                                          # Add the username, x_coordinate, and y_coordinate to the list as an entry
                        "username": user.username,
                        "x": x_coordinate,
                        "y": y_coordinate})
                else:                                                                   # If a user doesn't exist
                    print("NO MATCH")                                                   # Display there was an error
            return JsonResponse({"message": "Successful update", "updated_positions": updated_positions})               # Return JSON response with the updated positions

        except json.JSONDecodeError:                                                    # Except handler for format
            return JsonResponse({"message": "Invalid JSON"})                            # Return JSON response that there was an invalid format

    return JsonResponse({"error": "Invalid request"})                                   # Return JSON response that the request could not be processed
