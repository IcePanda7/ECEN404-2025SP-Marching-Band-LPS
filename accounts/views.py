from django.shortcuts import render, redirect                                   # Import a render and redirect
from django.contrib.auth.forms import AuthenticationForm                        # Django provided user creation template and authentication form
from django.contrib.auth import login, logout                                   # To login/logout user
from django.views.decorators.cache import never_cache                           # For users to back arrow on login
from accounts.forms import SignUpForm, ProfileForm                              # To get parameters for signup


# Create your views here.
def signup_view(request):                                           # Defining signup_view that takes in a user request
    if request.method == 'POST':                                    # If signup button is clicked
        user_form = SignUpForm(request.POST)                        # user_form has contents from SignUpForm (Default User: first/last name, username, password, and password confirmation
        profile_form = ProfileForm(request.POST)                    # profile_form has the contents of ProfileForm (Extended User: role and instrument
        if user_form.is_valid() and profile_form.is_valid():        # Check if both user_form and profile_form are valid (required fields are filled)
            user = user_form.save()                                 # Create new user and add info into the database
            profile = profile_form.save(commit=False)               # Create profile form but do not save contents to the database
            profile.user = user                                     # Link user to have a profile
            profile.save()                                          # Save the profile info into the database
            return redirect('BandField:main')                       # Send user to the band field
    else:                                                           # Otherwise
        user_form = SignUpForm()                                    # user_form has parameters for signup (username, password, confirm password, first/last name)
        profile_form = ProfileForm()                                # profile_form has parameters for profile (role and instrument)
    return render(request, 'accounts/signup.html', {'user_form': user_form, 'profile_form': profile_form})      # Return signup template to render, send dictionary of data

@never_cache                                                    # Don't cache the user logging in
def login_view(request):                                        # Defining login_view that takes in a user request
    if request.user.is_authenticated:                           # Check if user is already logged in
        logout(request)                                         # Logout user

    if request.method == 'POST':                                # If clicking login button
        form = AuthenticationForm(data=request.POST)            # form is filled with the data provided by the user
        if form.is_valid():                                     # Checks if form is valid
            user = form.get_user()                              # Login the user
            login(request, user)                                # Login current user
            return redirect('BandField:main')                   # Send user to URL in BandField
    else:                                                       # Otherwise
        form = AuthenticationForm()                             # Form has parameters for login (username and password)
    return render(request, 'accounts/login.html', {'form': form})           # Return login template to render, send dictionary of data

def logout_view(request):                                       # Defining logout_view that takes in a user request
    if request.method == 'POST':                                # Click a button to logout was made
        logout(request)                                         # Logout current user
        return redirect('homepage')                             # Redirect the user to homepage
