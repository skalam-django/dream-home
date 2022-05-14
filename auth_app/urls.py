from django.urls import path
from django.conf.urls import url
from .import views
app_name='auth_app'
urlpatterns = 	[
					url(r'^api/v1/login/$', views.Login.as_view(), name='api-login'),
					url(r'^api/v1/update_profile/$', views.UpdateProfile.as_view(), name='api-update-profile'),
					url(r'^login/$', views.WebLogin.as_view(), name='web-login'),
					url(r'^logout/$', views.Logout.as_view(), name='logout'),	
					url(r'^user_profile/$', views.UserProfileView.as_view(), name='user-profile'),	
				]
					
		
						
