from django.contrib import admin                  # Importing the admin
from .models import Profile, Position             # Import the Profile and Position models

# Register your models here.
admin.site.register(Profile)             # Register the profile model to the admin site
admin.site.register(Position)            # Register the position model to the admin site

