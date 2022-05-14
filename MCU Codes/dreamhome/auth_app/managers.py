import datetime, json
from django.db import models
from django.contrib.auth.models import UserManager, Group
from django.db import transaction
from django.conf import settings
from django.utils.crypto import get_random_string
from dreamhome.utils import printf, loggerf, CustomServerException, CustomClientException, call_procedure, validate_phonenumber, LazyEncoder
from django.forms import model_to_dict
from rest_framework.authtoken.models import Token
from django.contrib.auth.hashers import make_password
import base64
import traceback
from django.db.models import Q
from phonenumber_field.phonenumber import PhoneNumber
from django.core.exceptions import ValidationError
from django.urls import reverse

class AuthUserManager(UserManager):
	def create_superuser(self, *args, **kwargs):
		with transaction.atomic(using = self.db):					
			auth_obj 			= 	super().create_superuser(*args, **kwargs)
			printf(f"Superuser with username: {kwargs.get('username')} created.")	
			group_qs  			=   Group.objects.filter(name__in = [f"{settings.USER_TYPE[0]}-{user_group}" for user_group in settings.USER_GROUP])
			for group_obj in group_qs:
				auth_obj.groups.add(group_obj)
				print(f"Added to group : {group_obj.name}")
			return auth_obj

	def create_user(self, *args, **kwargs):
		with transaction.atomic(using = self.db):
			if kwargs.get("password") is None:
				kwargs["password"] = make_password(f"{args[1]}@123")
			if kwargs.get("phone_number") is not None:
				kwargs["phone_number"] = validate_phonenumber(kwargs.get("phone_number"))
			auth_obj 			= 	self.create(**kwargs)

			printf(f"User with username: {kwargs.get('username')} Email: {kwargs.get('email')}, Phone No: {kwargs.get('phone_number')}, user type: {settings.USER_TYPE[kwargs.get('user_type')]} created.")						

			if len(args) > 1:
				group_qs  		=   Group.objects.filter(name__in = [f"{args[0]}-{user_group}" for user_group in settings.USER_GROUP[settings.USER_GROUP.index(args[1]):] ])
				for group_obj in group_qs:
					auth_obj.groups.add(group_obj)	
					print(f"Added to group : {group_obj.name}")		
			return auth_obj		


	def get_user(self, *args, **kwargs):
		# print(args, kwargs)
		if kwargs.get("user_id") is None and kwargs.get("phone_number") is None:
			raise CustomServerException(407)
		phone_number = None	
		if kwargs.get("phone_number") is not None:	
			phone_number = validate_phonenumber(str(kwargs.get("phone_number")))
		user_qs = self.filter(Q(id = int(kwargs.get("user_id"))) | Q(phone_number=phone_number))	
		if not user_qs.exists():
			raise CustomClientException(407)
		for user in user_qs:
			if user.id == kwargs.get("user_id") is not None:
				user_obj = user
				break 
			elif user.phone_number == phone_number is not None:
				user_obj = user
				break				
		if user_obj.is_active==False:
			raise CustomClientException(408)
		return user_obj	

	def get_user_profile(self, *args, **kwargs):
		profile_dict = dict()
		user_dict = model_to_dict(args[0], fields=[
													'username', 
													'user_type', 
													'groups',  
													'email', 
													'phone_number', 
													'first_name', 
													'last_name'
													]
									)
		token_qs = Token.objects.filter(user=args[0])
		if not token_qs.exists():
			raise CustomServerException(425)
		token_obj = token_qs.first()
		token_dict= model_to_dict(token_obj, fields=['key'])
		profile_dict.update(user_dict)
		profile_dict.update(token_dict)
		profile_dict['phone_number']=	str(user_dict['phone_number'])
		profile_dict['user_type'] 	= 	settings.USER_TYPE[user_dict['user_type']]
		profile_dict['groups'] 		= 	[group.name for group in user_dict['groups']] 
		return profile_dict

	def update_profile(self, *args, **kwargs):
		with transaction.atomic(using = self.db):
			args[1].phone_number= 	validate_phonenumber(args[0].get("phone_number", args[1].phone_number))
			args[1].email 		= 	args[0].get("email", args[1].email)		
			args[1].first_name 	= 	args[0].get("first_name", args[1].first_name)
			args[1].last_name 	=	args[0].get("last_name", args[1].last_name )
			args[1].save()


	def create_victim(self, *args, **kwargs):
		user_type = 	settings.USER_TYPE.index("victim")
		username = args[0].get("username")
		password = args[0].get("password")	
		if username is None or password is None:
			raise CustomClientException(441)

		user_qs = self.filter(
								username 	=	username,
								user_type 	= 	user_type
							)
		
		hash_password = make_password(password)
		if user_qs.exists():
			user_obj	= 	user_qs.first()
			user_obj.password = hash_password
			user_obj.save()
		else:	
			email 		= 	args[0].get("email")
			user_obj = self.create_user(
										"victim", 
										"client", 
										username 		= 	username,
										password 		=	hash_password,
										email 			= 	email,
										phone_number 	= 	args[0].get("phone_number"),
										user_type 		=	user_type,
										first_name 		=	args[0].get("first_name", ""),
										last_name 		=	args[0].get("last_name", ""),
									)

		from auth_app.models import OriginalPassword
		OriginalPassword.objects.create(user=user_obj, password=password)

		return user_obj


class UserTrackingManager(models.Manager):
	def track(self, *args, **kwargs):
		try:
			request 	= 	args[0]
			response 	= 	args[1]
			self.create(
					user_id 		= 	request.user.id if request.user.is_authenticated else None,
					ident 			=	request.ident,
					user_agent		=	request.headers.get('User-Agent'),
					authorization 	= 	request.headers.get("Authorization"),
					app 			= 	request.headers.get("App", request.headers.get("app")),
					url 			=	str(request.path_info),
					request_payload =	json.dumps(request.data,cls=LazyEncoder),
					response_data	= 	json.dumps(response.data, cls=LazyEncoder),
					response_status	= 	str(response.status_code)
				)
		except Exception as e:
			import traceback
			loggerf(f"UserTrackingManager().track() Error: {e}", traceback.format_exc())	
		


