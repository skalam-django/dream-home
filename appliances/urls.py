from django.urls import path
from django.conf.urls import url
from .import views
app_name='appliances'
urlpatterns = 	[
					url(r'^api/v1/$', views.HomeApi.as_view(), name='home-api'),	
					url(r'^api/v1/room$', views.RoomApi.as_view(), name='room-api'),	
					url(r'^api/v1/appliances/$', views.AppliancesApi.as_view(), name='appliances-api'),
					url(r'^api/v1/appliance_details/$', views.AppliancesDetailsApi.as_view(), name='appliances-details-api'),
					url(r'^api/v1/operator/$', views.OperatorApi.as_view(), name='operator-api'),
					url(r'^api/v1/device/$', views.DeviceApi.as_view(), name='device-api'),
					url(r'^$', views.HomeView.as_view(), name='home-web'),
					url(r'^room$', views.RoomView.as_view(), name='room-web'),
					url(r'^appliances/$', views.AppliancesView.as_view(), name='appliances-web'),
					url(r'^operator/$', views.OperatorView.as_view(), name='operator-web'),
					url(r'^appliance_details/$', views.AppliancesDetailsView.as_view(), name='appliances-details-web'),
					url(r'^test/$', views.Test.as_view(), name='test'),
				]

