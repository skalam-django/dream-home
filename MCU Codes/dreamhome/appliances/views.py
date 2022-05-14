from django.shortcuts import render
from django.contrib.auth.mixins import LoginRequiredMixin
from auth_app.views import HomeAdminBasicAuthApi, MACIMEIAuthApi
from dreamhome.template import register_layout, register_sidebar, get_template, BaseTemplateView
from dreamhome.utils import (
							printf, loggerf, 
							CustomServerException, CustomClientException, 
							validate_phonenumber, 
							call_procedure,
							LazyEncoder,
							full_data, 
							data_handler
							)
from dreamhome.response_handler import CustomResponse, CustomAPIResponse, CustomViewResponse
from django.urls import reverse
from django.conf import settings
from appliances.models import Home, Room, Device, PortInfo, Appliances, Operator, Consumption
from dreamhome.workers import ProcessQueue


class HomeApi(HomeAdminBasicAuthApi):
	def get(self, request, *args, **kwargs):
		try:
			data = {'homes' : Home.objects.get_all(*args, **kwargs)}
			status = 200
		except (CustomClientException, CustomServerException) as e:
			data  = {}	
			status = e	
		response =  CustomResponse(data=data, status=status)
		return response

	def post(self, request, *args, **kwargs):
		try:
			Home.objects.create_home(full_data(request), request.user, *args, **kwargs)
			status = 200
		except Exception as e:
			status = e	
		response =  CustomResponse(data={}, status=status)
		return response


class HomeView(LoginRequiredMixin, BaseTemplateView):
	login_url = 'auth_app:web-login'
	LOGIN_REDIRECT_URL = '/auth/login/'
	template_name = get_template(__module__, 'home.html')
	extra_context = {
		"site_title"    : "Home",
		"logout_url"    : "auth_app:logout",
		"next_url"      : "/",
	}  
	register_layout(__qualname__, extra_context['site_title'], True)
	register_sidebar(['dreamhome_admin', 'home_client'], "home", extra_context['site_title'], get_url=extra_context['next_url'], post_url=extra_context['next_url'])

	def get_context_data(self, *args, **kwargs):
		response =  CustomAPIResponse(HomeApi, self.request, object=True, *args, **kwargs)	
		if getattr(response, 'data'):
			if response.data.get('success', False)==False and response.data.get('error', False)==True:
				self.__class__.__name__ = "ErrorView"
				self.template_name = get_template('dreamhome', 'error_page.html')
			response.data['room_url']	= '/room'
			self.extra_context.update(response.data)
		return super().get_context_data(*args, **kwargs)

	def post(self, request, *args, **kwargs):
		return CustomAPIResponse(HomeApi, request, *args, **kwargs)	




class RoomApi(HomeAdminBasicAuthApi):
	def get(self, request, *args, **kwargs):
		try:
			data = {'rooms' : Room.objects.get_room(full_data(request), request.user, *args, **kwargs)}
			status = 200
		except (CustomClientException, CustomServerException) as e:
			data  = {}	
			status = e		
		data.update({
					'room_category' : settings.ROOM_CATEGORY,
					'avail_slaves'  : PortInfo.objects.get_avail_slaves(full_data(request), request.user, *args, **kwargs)
					})	
		response =  CustomResponse(data=data, status=status)
		return response

	def post(self, request, *args, **kwargs):
		try:
			Room.objects.create_room(full_data(request), request.user, *args, **kwargs)
			status = 200
		except (CustomClientException, CustomServerException) as e:
			status = e	
		response =  CustomResponse(data={}, status=status)
		return response


class RoomView(LoginRequiredMixin, BaseTemplateView):
	login_url = 'auth_app:web-login'
	LOGIN_REDIRECT_URL = '/auth/login/'
	template_name = get_template(__module__, 'room.html')
	extra_context = {
		"site_title"    : "Room",
		"logout_url"    : "auth_app:logout",
		"next_url"      : "/room",
		"back_btn" 		: True,
	}  
	register_layout(__qualname__, extra_context['site_title'], True)
	# register_sidebar(['dreamhome_admin', 'home_client'], "room", extra_context['site_title'], get_url=extra_context['next_url'], post_url=extra_context['next_url'])

	def get_context_data(self, *args, **kwargs):
		response =  CustomAPIResponse(RoomApi, self.request, object=True, *args, **kwargs)	
		if getattr(response, 'data'):
			if response.data.get('success', False)==False and response.data.get('error', False)==True:
				self.__class__.__name__ = "ErrorView"
				self.template_name = get_template('dreamhome', 'error_page.html')
			response.data['appliances_url']	= '/appliances'
			self.extra_context.update(response.data)
		return super().get_context_data(*args, **kwargs)

	def post(self, request, *args, **kwargs):
		return CustomAPIResponse(RoomApi, request, *args, **kwargs)	


# 
class AppliancesApi(HomeAdminBasicAuthApi):
	def get(self, request, *args, **kwargs):
		try:
			data = {
				'appliances' : Appliances.objects.get_appliances(full_data(request), request.user, *args, **kwargs)
			}
			status = 200
		except (CustomClientException, CustomServerException) as e:
			data  = {}	
			status = e
		room_category = None
		room_id = request.GET.get('rid')
		r_qs = Room.objects.filter(id=room_id)
		if r_qs.exists():
			r_obj = r_qs.first()
			room_category = settings.ROOM_CATEGORY[r_obj.category]
		# ratings = Device.objects.get_ratings(request.full_data, request.user, *args, **kwargs) 	
		data.update({
						'room_category' : room_category.lower(),
						'appliances_all_categories' : settings.APPLIANCES_CATEGORY,  
						'avail_ports' : PortInfo.objects.get_avail_ports(full_data(request), request.user, *args, **kwargs),
						'relay_power_rating' : settings.RELAY_POWER_RATING,
						'relay_voltage_rating' : settings.RELAY_VOLTAGE_RATING					
					})
		response =  CustomResponse(data=data, status=status)
		return response

	def post(self, request, *args, **kwargs):
		try:
			if full_data(request).get("delete"):
				Appliances.objects.delete_appliances(request.full_data, request.user, *args, **kwargs)
			elif request.full_data.get("add"): 
				Appliances.objects.add_appliances(request.full_data, request.user, *args, **kwargs)
			else:	
				Appliances.objects.update_states(request.full_data, request.user, *args, **kwargs)
			status = 200
		except (CustomClientException, CustomServerException) as e:
			status = e
		response =  CustomResponse(data={}, status=status)
		return response


class AppliancesView(LoginRequiredMixin, BaseTemplateView):
	login_url = 'auth_app:web-login'
	LOGIN_REDIRECT_URL = '/auth/login/'
	template_name = get_template(__module__, 'appliances.html')
	extra_context = {
		"site_title"    : "Appliances",
		"logout_url"    : "auth_app:logout",
		"next_url"      : "/appliances/",
		"back_btn" 		: True,
	}  
	register_layout(__qualname__, extra_context['site_title'], True)

	def get_context_data(self, *args, **kwargs):
		response =  CustomAPIResponse(AppliancesApi, self.request, object=True, *args, **kwargs)	
		if getattr(response, 'data'):
			if response.data.get('success', False)==False and response.data.get('error', False)==True:
				self.__class__.__name__ = "ErrorView"
				self.template_name = get_template('dreamhome', 'error_page.html')
			response.data['appliance_details_url']	= '/appliance_details/'
			response.data['operator_url']  = '/operator/'
			self.extra_context.update(response.data)
		return super().get_context_data(*args, **kwargs)

	def post(self, request, *args, **kwargs):
		return CustomAPIResponse(AppliancesApi, request, *args, **kwargs)	



class OperatorApi(HomeAdminBasicAuthApi):
	def get(self, request, *args, **kwargs):
		try:
			data = {
				'operator' : Operator.objects.get_single(full_data(request), request.user, *args, **kwargs)[0]
			}
			status = 200
		except (CustomClientException, CustomServerException) as e:
			data  = {}	
			status = e
		data['get_operator_live_url'] = '/api/v1/operator/'
		print(data)
		response =  CustomResponse(data=data, status=200)
		return response

	def post(self, request, *args, **kwargs):
		try:
			if full_data(request).get('add'):
				Operator.objects.create_operator(request.full_data, request.user, *args, **kwargs)
			else:	
				Operator.objects.update_sensor_ref_value(request.full_data, request.user, *args, **kwargs)
			status = 200
		except (CustomClientException, CustomServerException) as e:
			status = e		
		response =  CustomResponse(data={}, status=200)
		return response


class OperatorView(LoginRequiredMixin, BaseTemplateView):
	login_url = 'auth_app:web-login'
	LOGIN_REDIRECT_URL = '/auth/login/'
	template_name = get_template(__module__, 'operator.html')
	extra_context = {
		"site_title"    : 	"Operator",
		"logout_url"    : 	"auth_app:logout",
		"next_url"      : 	"/operator/",
		"back_btn"		:	True,
	}  
	register_layout(__qualname__, extra_context['site_title'], True)

	def get_context_data(self, *args, **kwargs):
		response =  CustomAPIResponse(OperatorApi, self.request, object=True, *args, **kwargs)	
		if getattr(response, 'data'):
			if response.data.get('error', False)==True:
				self.__class__.__name__ = "ErrorView"
				self.template_name = get_template('dreamhome', 'error_page.html')
			self.extra_context.update(response.data)
		return super().get_context_data(*args, **kwargs)

	def post(self, request, *args, **kwargs):
		return CustomAPIResponse(OperatorApi, request, *args, **kwargs)	



class AppliancesDetailsApi(HomeAdminBasicAuthApi):
	def get(self, request, *args, **kwargs):
		try:			
			data = {
				'details_obj' : Appliances.objects.details(full_data(request), request.user, *args, **kwargs)[0]
			}
			status = 200
		except (CustomClientException, CustomServerException) as e:
			data  = {}	
			status = e
		ratings = Device.objects.get_port_info(request.full_data, request.user, *args, **kwargs) 
		data['relay_power_rating'] = ratings[0],
		data['relay_voltage_rating'] = ratings[1]		
		response =  CustomResponse(data=data, status=status)
		return response

	def post(self, request, *args, **kwargs):
		try:
			Appliances.objects.update_specifications(full_data(request), request.user, *args, **kwargs)
			status = 200
		except (CustomClientException, CustomServerException) as e:
			status = e
		response =  CustomResponse(data={}, status=status)
		return response


class AppliancesDetailsView(LoginRequiredMixin, BaseTemplateView):
	login_url = 'auth_app:web-login'
	LOGIN_REDIRECT_URL = '/auth/login/'
	template_name = get_template(__module__, 'appliances_details.html')
	extra_context = {
		"site_title"    : "Appliances Details",
		"logout_url"    : "auth_app:logout",
		"next_url"      : "/appliances_details/",
		"back_btn" 		: True,
	}  
	register_layout(__qualname__, extra_context['site_title'], True)

	def get_context_data(self, *args, **kwargs):
		response =  CustomAPIResponse(AppliancesDetailsApi, self.request, object=True, *args, **kwargs)	
		if getattr(response, 'data'):
			if response.data.get('success', False)==False and response.data.get('error', False)==True:
				self.__class__.__name__ = "ErrorView"
				self.template_name = get_template('dreamhome', 'error_page.html')
			self.extra_context.update(response.data)
		return super().get_context_data(*args, **kwargs)

	def post(self, request, *args, **kwargs):
		return CustomAPIResponse(AppliancesDetailsApi, request, *args, **kwargs)	

from auth_app.views import APIAuthenticateView, AllowAny
class Test(APIAuthenticateView):
	authentication_classes 		= 	()
	permission_classes 			= 	[AllowAny,]
	def post(self, request, *args, **kwargs):
		print(request.body)
		return CustomResponse(data={"context" : "Hello SIM800L"}, status=200)


class DeviceApi(MACIMEIAuthApi):

	def get(self, request, *args, **kwargs):
		try:	
			pass
		except (CustomClientException, CustomServerException) as e:
			data  = {}	
			status = e	
		response =  CustomResponse(data=data, status=status)
		return response

	def post(self, request, *args, **kwargs):
		try:	
			ProcessQueue.delay("appliances.utils", "update_sensor_status", full_data(request), request.device.id, request.device.user_id, request.device.home_id, *args, **kwargs)
			data = Appliances.objects.get_appliance_operator_port_status(request.device, *args, **kwargs)
			# data = {}
			status = 200
		except (CustomClientException, CustomServerException) as e:
			data  = {}	
			status = e	
		print("data: ", data)	
		response =  CustomResponse(data=data, status=status)
		return response


