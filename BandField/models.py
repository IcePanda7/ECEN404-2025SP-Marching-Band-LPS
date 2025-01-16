from django.db import models                                            # Import models
from django.contrib.auth.models import User                             # Import the User model
# Create your models here.

class Recordings(models.Model):                                         # Create a custom model Recordings
    director = models.ForeignKey(User, on_delete=models.CASCADE)        # Link the user to the database
    Attempt = models.IntegerField()                                     # To get Runs 1-10
    AttemptX_Coordinates = models.JSONField(default=list)               # List all the x_coordinates for a specific user
    AttemptY_Coordinates = models.JSONField(default=list)               # List all the y_coordinates for a specific user
    timestamp = models.DateTimeField(auto_now_add=True)                 # To get the time stamp of the recording
    recording = models.BooleanField(default=False)                      # To see if the recording is still going or not
    marcher_members = models.JSONField(default=list)                    # To get all the marchers in a specified recording