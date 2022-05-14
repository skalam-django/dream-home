from rest_framework.views import APIView
from rest_framework.authentication import TokenAuthentication, BasicAuthentication, SessionAuthentication
from rest_framework.permissions import IsAuthenticated, AllowAny
from rest_framework.throttling import AnonRateThrottle, UserRateThrottle
from rest_framework.permissions import SAFE_METHODS, OperandHolder, OR
from rest_framework.parsers import JSONParser, MultiPartParser, FormParser
from rest_framework.exceptions import PermissionDenied
from rest_framework.exceptions import APIException
from django.core.cache import caches
from django.forms.models import model_to_dict
from django.contrib.auth import get_user_model
from dreamhome.workers import ProcessQueue
import json
import base64
from django.conf import settings
from dreamhome.utils import loggerf, printf, CustomServerException, CustomClientException, full_data
from dreamhome.response_handler import CustomResponse, CustomAPIResponse, CustomViewResponse
from dreamhome.template import register_layout, register_sidebar, get_template, BaseTemplateView
import traceback
from auth_app.models import UserTracking
from auth_app.authentication import MACIMEIAuthentication
from django.contrib.auth.views import LoginView
from django.contrib.auth import logout, login
from django.views import View
from django.shortcuts import render, redirect
import datetime
from django.template.response import TemplateResponse
from django.urls import resolve
from django.views.generic import TemplateView
from django.contrib.auth.mixins import LoginRequiredMixin
from rest_framework.renderers import BaseRenderer, JSONRenderer, TemplateHTMLRenderer
from auth_app.throttling import DeviceRateThrottle

class IsAuthenticatedAdmin(IsAuthenticated):
	def has_permission(self, request, view):
		if request.method in SAFE_METHODS and request.method=='OPTIONS':
			return True
		response=  super().has_permission(request, view)
		if response==True:
			if request.user.is_authenticated==True and request.user.user_type==settings.USER_TYPE.index("dreamhome") and request.user.groups.filter(name="dreamhome-admin").exists():
				return True
			return False
		return response


class IsAuthenticatedStaff(IsAuthenticated):
	def has_permission(self, request, view):
		if request.method in SAFE_METHODS and request.method=='OPTIONS':
			return True
		response=  super().has_permission(request, view)
		if response==True:
			if request.user.is_authenticated==True and request.user.user_type==settings.USER_TYPE.index("dreamhome") and request.user.groups.filter(name="dreamhome-staff").exists():
				return True
			return False
		return response


class IsAuthenticatedClient(IsAuthenticated):
	def has_permission(self, request, view):
		if request.method in SAFE_METHODS and request.method=='OPTIONS':
			return True
		response=  super().has_permission(request, view)
		if response==True:
			if request.user.is_authenticated==True and request.user.user_type==settings.USER_TYPE.index("dreamhome") and request.user.groups.filter(name="dreamhome-client").exists():
				return True
			return False
		return response		

class IsAuthenticatedDevice(IsAuthenticated):
	def has_permission(self, request, view):
		if request.method in SAFE_METHODS and request.method=='OPTIONS':
			return True
		response=  super().has_permission(request, view)
		if response==True:
			return request.device.is_authenticated and request.user.user_type==settings.USER_TYPE.index("device")
		return response	


class IsAuthenticatedHomeAdmin(IsAuthenticated):
	def has_permission(self, request, view):
		if request.method in SAFE_METHODS and request.method=='OPTIONS':
			return True
		response=  super().has_permission(request, view)
		if response==True:
			if request.user.is_authenticated==True and request.user.user_type==settings.USER_TYPE.index("home") and request.user.groups.filter(name="home-admin").exists():
				return True
			return False
		return response	




class PostAnonRateThrottle(AnonRateThrottle):
	scope = 'post_anon'
	cache = caches['throttling']

	def allow_request(self, request, view):
		if request.method == "GET":
			return True
		return super().allow_request(request, view)

	def get_cache_key(self, request, view):
		cache_format 	= 	super().get_cache_key(request, view)
		ident 	=	self.get_ident(request)
		if ident is not None:
			request.ident 	= 	ident
		return cache_format


class GetAnonRateThrottle(AnonRateThrottle):
	scope =	'get_anon'
	cache = caches['throttling']

	def allow_request(self, request, view):
		if request.method == "POST":
			return True
		return super().allow_request(request, view)

	def get_cache_key(self, request, view):
		cache_format 	= 	super().get_cache_key(request, view)
		ident 	=	self.get_ident(request)
		if ident is not None:
			request.ident 	= 	ident
		return cache_format


class PostUserRateThrottle(UserRateThrottle):
	scope = 'post_user'
	cache = caches['throttling']

	def allow_request(self, request, view):
		if request.method == "GET":
			return True
		return super().allow_request(request, view)

	def get_cache_key(self, request, view):
		cache_format 	= 	super().get_cache_key(request, view)
		ident 	=	self.get_ident(request)
		if ident is not None:
			request.ident 	= 	ident
		return cache_format

class GetUserRateThrottle(UserRateThrottle):
	scope = 'get_user'
	cache = caches['throttling']

	def allow_request(self, request, view):
		if request.method == "POST":
			return True
		return super().allow_request(request, view)

	def get_cache_key(self, request, view):
		cache_format 	= 	super().get_cache_key(request, view)
		ident 	=	self.get_ident(request)
		if ident is not None:
			request.ident 	= 	ident
		return cache_format



class PostDeviceRateThrottle(DeviceRateThrottle):
	scope = 'post_device'
	cache = caches['throttling']

	def allow_request(self, request, view):
		if request.method == "GET":
			return True
		return super().allow_request(request, view)

	def get_cache_key(self, request, view):
		cache_format 	= 	super().get_cache_key(request, view)
		ident 	=	self.get_ident(request)
		if ident is not None:
			request.ident 	= 	ident
		return cache_format


class GetDeviceRateThrottle(DeviceRateThrottle):
	scope = 'get_device'
	cache = caches['throttling']

	def allow_request(self, request, view):
		if request.method == "POST":
			return True
		return super().allow_request(request, view)

	def get_cache_key(self, request, view):
		cache_format 	= 	super().get_cache_key(request, view)
		ident 	=	self.get_ident(request)
		if ident is not None:
			request.ident 	= 	ident
		return cache_format	


class APIAuthenticateView(APIView):
	parser_classes 		= 	[JSONParser, MultiPartParser, FormParser]
	throttle_classes 	= 	[
								PostAnonRateThrottle, 
								GetAnonRateThrottle, 
								PostUserRateThrottle, 
								GetUserRateThrottle,
								PostDeviceRateThrottle,
								GetDeviceRateThrottle,
							]

	def finalize_response(self, request, response, *args, **kwargs):
		if not isinstance(response, TemplateResponse):
			response = self.process_response(request, response)
		if hasattr(response, 'accepted_media_type') and response.accepted_media_type=='application/json':
			response = super().finalize_response(request, response, *args, **kwargs)

		if request.user is not None and request.user.is_authenticated and request.user.tracking == True:
			UserTracking.objects.track(request, response)
		return response	

	def handle_exception(self, exc):
		log_label 	= 	'Error'	
		trace 		= 	traceback.format_exc()
		info 		= 	False
		if isinstance(exc, CustomClientException):
			log_label= 	'CustomClientException'
			info	 = 	True
			tracking = self.request.user.tracking if self.request.user.is_authenticated else False
			if settings.TRACEBACK_OFF == True and tracking==False:
				trace = ""
			response = 	CustomResponse(data={}, status=exc)
		elif isinstance(exc, APIException):
			response = super().handle_exception(exc)
		elif isinstance(exc, CustomServerException):
			response 	= 	CustomResponse(data={}, status=exc)
		else:
			response 	= 	CustomResponse(data={}, status=500)
		
		loggerf(f"************************ {log_label} ************************ : {exc}\n", trace, info=info)		
					
		# response = render(self.request, "dreamhome/error_page.html", response.data)
				
		return self.process_response(self.request, response)

	def process_response(self, request, response):
		try:
			app = request.headers.get("App")
			if hasattr(response, 'accepted_media_type') and response.accepted_media_type=='text/plain':
				return response
			
			if hasattr(response, 'data')==False or response.data is None:
				response.data = dict()
			response.data['success'] 	= response.data['success'] if 'success' in response.data else (True if response.status_code in settings.SAFE_STATUS_CODE else False)
			response.data['error'] 		= response.data['error'] if 'error' in response.data else (False if response.status_code in settings.SAFE_STATUS_CODE else True)
			response.data['message'] 	= response.data['message'] if 'message' in response.data else None
			if 'error_details' not in response.data:
				response.data['error_details'] = {
													"description" : str(response.data['detail']) if 'detail' in response.data else "Something went wrong",
													"field" : None,
													"error_type" : "BAD_REQUEST_ERROR" if 'detail' in response.data else "INTERNAL_SERVER_ERROR"
												}
				if 'detail' in response.data:								
					response.data.pop("detail")					
			status_code = response.status_code
			
			if app is None or app.lower() in ["device"]:	
				response.status_code = (200 if status_code not in settings.SAFE_STATUS_CODE else status_code) if status_code!=500 else status_code				
				response.data['status_code']= status_code
				response.data['status_text']= response.status_text
				response.accepted_renderer =  JSONRenderer()
				response.renderer_context = {}	
				response.accepted_media_type = "application/json"
			else:
				view_obj, _, _ = resolve(request.path)
				template_name = view_obj.view_class.template_name
				if request.method == "GET":
					try:
						if template_name is None:
							template_obj = BaseTemplateView()
							template_obj.request = request
							template_obj.template_name = get_template("dreamhome", 'error_page.html')
							template_obj.extra_context = {
															"site_title" : "Error",
															"logout_url"  : "auth:logout",
															"next_url" : "/",
															}
							template_obj.extra_context.update(response.data)	
							response = template_obj.dispatch(request)	

					except AttributeError as e:
						pass
			return response

		except (APIException, CustomServerException, CustomClientException) as e:
			# if app is not None and app.lower() not in ["web"]:
			raise type(e)(e)
		except Exception as e:
			import traceback
			loggerf(traceback.format_exc())
			loggerf(f"APIAuthenticateView().process_response() Error: {e}")
			raise type(e)(e)	



class AdminTokenAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(TokenAuthentication, SessionAuthentication,)
	permission_classes 			= 	[IsAuthenticatedAdmin,]

class HomeAdminTokenAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(TokenAuthentication, SessionAuthentication,)
	permission_classes 			= 	[OperandHolder(OR, IsAuthenticatedHomeAdmin, IsAuthenticatedAdmin)]

class MACIMEIAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(MACIMEIAuthentication,)
	permission_classes 			= 	[OperandHolder(OR, IsAuthenticatedDevice, IsAuthenticatedAdmin)]


class StaffTokenAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(TokenAuthentication, SessionAuthentication,)
	permission_classes 			= 	[OperandHolder(OR, IsAuthenticatedStaff, IsAuthenticatedAdmin),]


class ClientTokenAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(TokenAuthentication, SessionAuthentication,)
	permission_classes 			= 	[OperandHolder(OR, IsAuthenticatedClient, IsAuthenticatedAdmin),]



class AllUserTokenAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(TokenAuthentication, SessionAuthentication,)
	permission_classes 			= 	[IsAuthenticated,]


class AdminBasicAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(BasicAuthentication, SessionAuthentication,)
	permission_classes 			= 	[IsAuthenticatedAdmin,]


class StaffBasicAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(BasicAuthentication, SessionAuthentication,)
	permission_classes 			= 	[OperandHolder(OR, IsAuthenticatedStaff, IsAuthenticatedAdmin),]


class ClientBasicAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(BasicAuthentication, SessionAuthentication,)
	permission_classes 			= 	[OperandHolder(OR, IsAuthenticatedClient, IsAuthenticatedAdmin),]


class HomeAdminBasicAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(BasicAuthentication, SessionAuthentication,)
	permission_classes 			= 	[OperandHolder(OR, IsAuthenticatedHomeAdmin, IsAuthenticatedAdmin)]


class AllUserBasicAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(BasicAuthentication, SessionAuthentication,)
	permission_classes 			= 	[IsAuthenticated,]


class AdminAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(TokenAuthentication, BasicAuthentication, SessionAuthentication,)
	permission_classes 			= 	[IsAuthenticatedAdmin,]


class StaffAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(TokenAuthentication, BasicAuthentication, SessionAuthentication,)
	permission_classes 			= 	[OperandHolder(OR, IsAuthenticatedStaff, IsAuthenticatedAdmin),]


class ClientAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(TokenAuthentication, BasicAuthentication, SessionAuthentication,)
	permission_classes 			= 	[OperandHolder(OR, IsAuthenticatedClient, IsAuthenticatedAdmin),]


class AllUserAuthApi(APIAuthenticateView):
	authentication_classes 		= 	(TokenAuthentication, BasicAuthentication, SessionAuthentication,)
	permission_classes 			= 	[IsAuthenticated,]	


class Login(AllUserBasicAuthApi):
	def post(self, request, *args, **kwargs):
		user_profile = 	get_user_model().objects.get_user_profile(request.user)
		return CustomResponse(data={'user_profile': user_profile}, status=200)



class UpdateProfile(AllUserTokenAuthApi):
	def post(self, request, *args, **kwargs):
		if full_data(request) is None or full_data(request) == {}:
			raise CustomClientException(432)
		user_obj 	= 	get_user_model().objects.update_profile(full_data(request), request.user)
		return CustomResponse(data={}, status=200)	


class WebLogin(BaseTemplateView):
	template_name = get_template(__module__, 'weblogin.html')
	extra_context = {
		"site_title"    : "Login",
		"logout_url"    : "auth_app:logout",
		"next_url"      : "/auth/login/",
	}  
	register_layout(__qualname__, extra_context['site_title'], True)

	def get(self, request, *args, **kwargs):
		self.LOGIN_REDIRECT_URL = request.GET.get("next")
		if request.user.is_authenticated==True:
			return redirect(self.LOGIN_REDIRECT_URL)
		return super().get(request, *args, **kwargs)

	def get_context_data(self, *args, **kwargs):
		self.extra_context.update({'login_type' : 'Admin Login'})
		self.request.session['LOGIN_REDIRECT_URL'] = self.LOGIN_REDIRECT_URL
		return super().get_context_data(*args, **kwargs)


	def post(self, request, *args, **kwargs):
		authorization_key = base64.b64encode(f"{full_data(request).get('username')}:{full_data(request).get('password')}".encode()).decode()
		request.META['HTTP_AUTHORIZATION'] = f'basic {authorization_key}'
		printf(request.POST, full_data(request).get('username'),full_data(request).get('password'),authorization_key)
		response =  CustomAPIResponse(Login, request, *args, **kwargs)	
		
		if response.data.get('success', False)==True:
			login(request, request.user, backend='django.contrib.auth.backends.ModelBackend')
			return redirect(request.session.get('LOGIN_REDIRECT_URL', request.GET.get('next')))
		logout(request)	
		error_details = response.data.get('error_details')
		self.extra_context['error'] = response.data.get('error', True)
		self.extra_context['error_details'] = {'description' : error_details.get('description') if error_details else None}
		self.LOGIN_REDIRECT_URL = request.GET.get("next")
		context_data = self.get_context_data(*args, **kwargs)
		return render(request, self.template_name, context_data)


class UserProfileView(LoginRequiredMixin, BaseTemplateView):
	login_url = 'auth_app:web-login'
	LOGIN_REDIRECT_URL = '/auth/login/'	
	template_name 	= 	get_template(__module__, 'user_profile.html')
	extra_context 	= 	{
		"site_title"    : "Profile",
		"logout_url"    : "auth_app:logout",
		"next_url"      : "/auth/user_profile/",
	}  
	register_layout(__qualname__, 'Edit Profile', True)

	def get_context_data(self, *args, **kwargs):
		text_fields = [
							{
								"label_id" : "username-label",
								"value" : self.request.user.username,
								"label" : "Username",
								"editable" : False
							},		
							{
								"label_id" : "first-name-label",
								"value" : self.request.user.first_name,
								"label" : "First name",
								"editable" : True
							},
							{
								"label_id" : "last-name-label",
								"value" : self.request.user.last_name,
								"label" : "Last name",
								"editable" : True
							},
							{
								"label_id" : "email-label",
								"value" : self.request.user.email,
								"label" : "Email",
								"editable" : True
							},
							{
								"label_id" : "phone-no-label",
								"value" : self.request.user.phone_number,
								"label" : "Phone number",
								"editable" : True
							},																					
						]				
		self.extra_context.update({"text_fields" :text_fields})
		return super().get_context_data(*args, **kwargs)
		
	def post(self, request, *args, **kwargs):
		return CustomAPIResponse(UpdateProfile, request, *args, **kwargs)	

class SendOTP(AllUserAuthApi):
	shareable = True
	def __init__(self, *args, **kwargs):
		self.shareable = kwargs.get("shareable", self.shareable)
		self.purpose = [
						" to verify your phone.\nYour can share this OTP with Recharge Teminals",
						" to verify your phone.\nDO NOT share this OTP with anyone."
		] 
	def post(self, request, *args, **kwargs):
		phone_number = None
		otp = None
		phone_number = 	request.data.get("phone_number")
		if self.shareable==False:
			if phone_number is None:
				phone_number = request.user.phone_number
		otp = OTP.objects.create_otp(str(phone_number), shareable=self.shareable, purpose=self.purpose[0 if self.shareable==True else 1])
		return CustomResponse(data={'otp':otp}, status=200)


class Logout(View):
	register_layout(__qualname__, 'Login', False)	
	def post(self, request, *args, **kwargs):
		try:
			next_url = request.GET.get("next", "/")
			logout(request)			
			return redirect(next_url)
		except Exception as e:
			loggerf(f"Logout().post() Error {e}")	


