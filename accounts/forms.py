from django import forms                                            # To create forms
from django.contrib.auth.models import User                         # Default user
from django.contrib.auth.forms import UserCreationForm              # Default user parameters
from .models import Profile                                         # Extended default user


class SignUpForm(UserCreationForm):                                                         # Creating a SignUp form
    class Meta:                                                                             # Define what to specifically include/relate
        model = User                                                                        # model relates to default user class
        fields = ('username', 'password1', 'password2', 'first_name', 'last_name')          # Fields in the SignUpForm is username, password, confirm password, first/last name


class ProfileForm(forms.ModelForm):                                                         # Creating a profile form
    class Meta:                                                                             # Define what to specifically include/relate
        model = Profile                                                                     # model relates to Profile class
        fields = ('role', 'instrument')                                                     # Fields in the ProfileForm is role and instrument
