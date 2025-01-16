from django.contrib import admin                # Import the admin
from BandField.models import Recordings         # Import Recordings model

# Register your models here.
admin.site.register(Recordings)                 # Adding Recordings model to the admin site