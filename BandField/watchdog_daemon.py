import sys                                              # Import the system
import time                                             # Import time to sleep
import logging                                          # To log messages
from watchdog.observers import Observer                 # Import observer to watch for file system changes
from watchdog.events import FileSystemEventHandler      # Import FileSystemEventHandler to handle file system events

#KB
import os                                               # To interact with the operating system
import django                                           # Django framework for backend

#sys.path.append("C:\\Users\\vladi\\PycharmProjects\\MarchingBangGPSWeb")        # Add the project path to the system path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
os.environ['DJANGO_SETTINGS_MODULE'] = 'MarchingBangGPSWeb.settings'            # Setting the virtual environment
django.setup()                                                                  # Setting up framework

from django.contrib.auth.models import User                                     # To get all marchers
from accounts.models import Position                                            # To get x and y coordinates


class CustomEventHandler(FileSystemEventHandler):                           # Create a custom event handler for file system events
    def on_modified(self, event):                                           # Override the event handler

        """
        if event.src_path.endswith("data.txt"):                             # Find data.txt file
            with open(event.src_path, 'r') as file:                         # Open the data.txt file to read
                marchers = User.objects.filter(profile__role='marcher')     # Get all the marchers
                for marcher in marchers:                                    # Iterate all the marchers in the file
                    position = Position.objects.get(user=marcher)           # Check if the marcher has a position model if not create it
                    file.seek(0)                                            # Go to the start of the file
                    for line in file:                                       # Iterate line be line of the file
                        data = line.split()                                 # Store the line of data in variable
                        if data[0] == marcher.username:                     # Check if the username matched
                            position.x_coordinate = float(data[1])          # Store the second component as the marcher's x-coordinate
                            position.y_coordinate = float(data[2])          # Store the third component as the marcher's y-coordinate
                            position.save()                                 # Save positions to the database
                            break                                           # Exit the for loop
                        else:                                               # If the username doesn't match
                            continue                                        # Continue through the file
        """


if __name__ == "__main__":                                                  # Starting the code
    logging.basicConfig(level=logging.INFO, format='%(asctime)s  - %(message)s',        # Set format for logging info
                        datefmt='%Y-%m-%d %H:%M:%S')

    #path = 'C:/Users/vladi/PycharmProjects/MarchingBangGPSWeb/assets'       # Directory path to data.txt file
    path = os.path.join(os.path.dirname(__file__), 'assets')
    logging.info("Watchdog in progress")                                    # Log the start of watchdog

    event_handler = CustomEventHandler()                                    # Initialize custom event handler
    observer = Observer()                                                   # Initialize observer to monitor file system events
    observer.schedule(event_handler, path, recursive=True)                  # Set the observer to monitor the path along with the event handler
    observer.start()                                                        # Start the observer
    try:                                                                    # Try block
        while True:                                                         # Constantly running to check for file changes
            time.sleep(1)                                                   # Sleep 1 second for each iteration
    except KeyboardInterrupt:                                               # Except for loop termination
        observer.stop()                                                     # Stop the observer
    observer.join()                                                         # Wait for the observer to complete running
