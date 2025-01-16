""" Main File (MarchingBandGPS) View """

from django.shortcuts import render                          # Import render to render html pages with data

def homepage(request):                                       # Define homepage that takes in a request from the user
    return render(request, 'homepage.html')     # Render the homepage html with the user's request

def subsystem(request):                             # Define subsystem that takes in a request from the user
    if 'counter' in request.session:                # Check if there is already a counter to be updated
        if request.method == 'POST':                # Check if the button is pressed
            if 'increment' in request.POST:         # Check if it was the increment button
                request.session['counter'] += 1     # Increment the counter by 1
            elif 'decrement' in request.POST:       # Check if it was the decrement button
                request.session['counter'] -= 1     # Decrement the counter by 1
    else:                                           # Otherwise
        request.session['counter'] = 0              # Create and set the counter to 0
    return render(request, 'subsystem.html', {'counter': request.session['counter']})  # Render the about html while passing through the counter value
