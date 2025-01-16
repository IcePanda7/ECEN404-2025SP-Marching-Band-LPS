from django.urls import path        # To create urls inside the file
from . import views

app_name = 'accounts'               # name space urls

urlpatterns = [
    path('signup/', views.signup_view, name="signup"),              # path of the url, view to send the user to, use url name as 'signup' to avoid hard coding
    path('login/', views.login_view, name="login"),                 # path of the url, view to send the user to login_view
    path('logout/', views.logout_view, name="logout"),              # path of the url for users to logout
]

