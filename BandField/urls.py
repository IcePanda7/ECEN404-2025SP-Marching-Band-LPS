from django.urls import path
from . import views

app_name = 'BandField'

urlpatterns = [
    path('', views.band_field, name="main"),                                        # Link the band field page
    path('recent-position/', views.recent_position, name="current_position"),       # Link current_position function
    path('band_recordings/', views.band_recordings, name="band_recordings"),        # Link the recordings page
    path('recording_data/', views.recording_data, name="recording_data"),           # Line recording_data function
    path('start-recordings/', views.start_recordings, name="start-recordings"),     # Link start_recordings function
    path('stop-recordings/', views.stop_recordings, name="stop-recordings")         # Link stop_recordings function
]
