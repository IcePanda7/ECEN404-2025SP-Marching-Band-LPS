from django.contrib.auth.models import User                             # Importing the default user model
from django.db import models                                            # Importing models

# Create your models here.
class Profile(models.Model):                                            # Creating a profile model
    Roles = [                                                           # Role options
        ('marcher', 'Marcher'),                                         # Role can be 'marcher'
        ('director', 'Director')]                                       # Role can be 'director'

    Instruments = [                                                     # Instrument options
        ('n/a', 'N/A'),                                                 # Instrument can be 'n/a'
        ('other', 'Other'),                                             # Instrument can be 'other'
        ('flute/piccolo', 'Flute/Piccolo'),                             # Instrument can be 'flue/piccolo'
        ('clarinet', 'Clarinet'),                                       # Instrument can be 'clarinet'
        ('bass clarinet', 'Bass Clarinet'),                             # Instrument can be 'bass clarinet'
        ('alto saxophone', 'Alto Saxophone'),                           # Instrument can be 'alto saxophone'
        ('tenor saxophone', 'Tenor Saxophone'),                         # Instrument can be 'tenor saxophone'
        ('baritone saxophone', 'Baritone Saxophone'),                   # Instrument can be 'baritone saxophone'
        ('trumpet', 'Trumpet'),                                         # Instrument can be 'trumpet'
        ('trombone', 'Trombone'),                                       # Instrument can be 'trombone'
        ('baritone/euphonium', 'Baritone/Euphonium'),                   # Instrument can be 'baritone/euphonium'
        ('sousaphone', 'Sousaphone'),                                   # Instrument can be 'sousaphone'
        ('drum line', 'Drum Line')]                                     # Instrument can be 'drum line'

    user = models.OneToOneField(User, on_delete=models.CASCADE)                             # User is one to one with the default user model where if user is deleted so will the profile
    role = models.CharField(choices=Roles, default='marcher', max_length=50)                # Role is an element in the database which is defaulted as 'marcher'
    instrument = models.CharField(choices=Instruments, default='n/a', max_length=50)        # Instrument is an element in the database which is defaulted as 'n/a'

class Position(models.Model):                                           # Creating a Position model
    user = models.OneToOneField(User, on_delete=models.CASCADE)         # Linking the position to a user
    x_coordinate = models.FloatField(default=0.0)                       # Having an x-coordinate that defaults to 0.0
    y_coordinate = models.FloatField(default=0.0)                       # Having a y-coordinate that defaults to 0.0
