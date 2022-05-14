from django.db import models
import binascii
import os
import jsonfield
from django.conf import settings
from appliances.managers import (
								HomeManager, 
								RoomManager,
								DeviceManager,
								PortInfoManager,
								AppliancesManager,
								OperatorManager
							)
from django.contrib.auth import get_user_model
from django.contrib.postgres.fields import ArrayField
from phonenumber_field.modelfields import PhoneNumberField
from dreamhome.utils import validate_mac


ROOM_CATEGORY 		= 	tuple([(idx, room_category) for idx, room_category in enumerate(settings.ROOM_CATEGORY)])
APPLIANCES_CATEGORY =	tuple([(idx, appliances_category) for idx, appliances_category in enumerate(settings.APPLIANCES_CATEGORY)])
OPERATOR_CATEGORY 	=	tuple([(idx, operator_category) for idx, operator_category in enumerate(settings.OPERATOR_CATEGORY)])

class Home(models.Model):
	user 			=	models.ForeignKey(get_user_model(), on_delete=models.CASCADE)
	name 			= 	models.CharField(max_length=255)
	is_active 		=	models.BooleanField(default=True)
	created_at      =   models.DateTimeField(auto_now_add=True)
	updated_at      =   models.DateTimeField(auto_now=True)   
	objects         =   HomeManager() 
	class Meta:
		managed = True
		db_table = 'home'
	def __str__(self):
		return F"{self.id} {self.name}" 


class Room(models.Model):
	user 			=	models.ForeignKey(get_user_model(), on_delete=models.CASCADE)
	home 			=	models.ForeignKey(Home, on_delete=models.CASCADE)
	name 			= 	models.CharField(max_length=255)
	category 		=	models.SmallIntegerField(choices=ROOM_CATEGORY, default=ROOM_CATEGORY[0][0])
	is_active 		=	models.BooleanField(default=True)
	created_at      =   models.DateTimeField(auto_now_add=True)
	updated_at      =   models.DateTimeField(auto_now=True)   
	objects         =   RoomManager() 
	class Meta:
		managed = True
		db_table = 'room'
	def __str__(self):
		return F"{self.id} {self.name}"




class Device(models.Model):
	imei 			=	models.CharField(max_length=15, unique=True)
	mac 			= 	models.CharField(max_length=17, validators =[validate_mac])
	key 			= 	models.CharField(max_length=40)
	ssid 			=	models.CharField(max_length=32, unique=True)
	password 		=	models.CharField(max_length=8)
	wifi_ssid 		= 	ArrayField(models.CharField(max_length=32, null=True), null=True)
	wifi_pass 		= 	ArrayField(models.CharField(max_length=8, null=True), null=True)
	user 			=	models.ForeignKey(get_user_model(), on_delete=models.CASCADE, null=True)
	sim_pin 		=	PhoneNumberField(region='IN', null=True)
	ident           =   models.GenericIPAddressField(null=True)
	home 			=	models.ForeignKey(Home, on_delete=models.CASCADE, null=True)
	is_active 		=	models.BooleanField(default=True)
	created_at      =   models.DateTimeField(auto_now_add=True)
	updated_at      =   models.DateTimeField(auto_now=True)
	last_online     =   models.DateTimeField(null=True)
	objects 		=	DeviceManager()
	class Meta:
		managed 	=	True
		db_table 	=	'device'

	def save(self, *args, **kwargs):
		if not self.key:
			self.key = self.generate_key()
		return super().save(*args, **kwargs)

	@classmethod
	def generate_key(cls):
		return binascii.hexlify(os.urandom(20)).decode()

	def __str__(self):
		return f"{self.key} {self.imei} {self.ssid}"


class PortInfo(models.Model):
	mac 			= 	models.CharField(max_length=17, validators =[validate_mac])
	device 			=	models.ForeignKey(Device, on_delete=models.CASCADE)
	port 			=	models.CharField(max_length=2)
	user 			=	models.ForeignKey(get_user_model(), on_delete=models.CASCADE, null=True)
	home 			=	models.ForeignKey(Home, on_delete=models.CASCADE, null=True)
	room 			=	models.ForeignKey(Room, on_delete=models.CASCADE, null=True)
	is_used 		= 	models.BooleanField(default=False)
	created_at      =   models.DateTimeField(auto_now_add=True)
	updated_at      =   models.DateTimeField(auto_now=True)   
	objects         =   PortInfoManager() 
	class Meta:
		managed = True
		db_table = 'port_info'
		unique_together = (('device', 'mac', 'port'))
	def __str__(self):
		return F"{self.id} {self.device} {self.mac} {self.port} {self.is_used}"

class Appliances(models.Model):
	device 			= 	models.ForeignKey(Device, on_delete=models.CASCADE)
	port 			=	models.OneToOneField(PortInfo, on_delete=models.CASCADE)

	user 			=	models.ForeignKey(get_user_model(), on_delete=models.CASCADE)
	home 			=	models.ForeignKey(Home, on_delete=models.CASCADE)
	room 			=	models.ForeignKey(Room, on_delete=models.CASCADE)
	
	name 			= 	models.CharField(max_length=255)
	category 		=	models.SmallIntegerField(choices=APPLIANCES_CATEGORY, default=APPLIANCES_CATEGORY[0][0])

	is_active 		=	models.BooleanField(default=True)
	status 			=	models.BooleanField(default=False)
	manual 			=	models.BooleanField(default=False)

	power_rating 	=	models.DecimalField(null=True, max_digits=8, decimal_places=2)
	voltage_rating 	=	models.DecimalField(null=True, max_digits=8, decimal_places=2)
	
	active_dur 		=	models.IntegerField(default=0.0)
	total_active_dur=	models.IntegerField(default=0.0)

	created_at      =   models.DateTimeField(auto_now_add=True)
	updated_at      =   models.DateTimeField(auto_now=True)   
	objects         =   AppliancesManager() 
	class Meta:
		managed = True
		db_table = 'appliances'
	def __str__(self):
		return F"{self.id} {self.name}"


class Operator(models.Model):
	device 			= 	models.ForeignKey(Device, on_delete=models.CASCADE)
	port 			=	models.OneToOneField(PortInfo, on_delete=models.CASCADE)

	user 			=	models.ForeignKey(get_user_model(), on_delete=models.CASCADE)
	home 			=	models.ForeignKey(Home, on_delete=models.CASCADE)
	room 			=	models.ForeignKey(Room, on_delete=models.CASCADE)
	appliances 		=	models.ForeignKey(Appliances, on_delete=models.CASCADE)

	name 			= 	models.CharField(max_length=255)
	category 		=	models.SmallIntegerField(choices=OPERATOR_CATEGORY, default=OPERATOR_CATEGORY[0][0])
	
	sensor_val 		=	models.FloatField(default=0.0) #0-1023
	sensor_max_val 	=	models.FloatField(default=1023)
	sensor_min_val 	=	models.FloatField(default=0.0)
	
	calibration_ref1 = models.BooleanField(default=False)
	calibration_ref2 = models.BooleanField(default=False)

	dimensions_data =	jsonfield.JSONField()
	
	is_active 		=	models.BooleanField(default=True)

	created_at      =   models.DateTimeField(auto_now_add=True)
	updated_at      =   models.DateTimeField(auto_now=True)   
	objects         =   OperatorManager() 
	class Meta:
		managed = True
		db_table = 'operator'
	def __str__(self):
		return F"{self.id} {self.name}"



class Consumption(models.Model):
	user						=	models.ForeignKey(get_user_model(), on_delete=models.CASCADE)
	appliances 					=	models.ForeignKey(Appliances, on_delete=models.CASCADE)

	per_unit_cost				= 	models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2)

	cur_voltage_level			=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	cur_current_level			=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	active_duration_min			=	ArrayField(models.BooleanField(blank=True, null=True))	

	daily_consumption			=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	weekly_consumption			=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	monthly_consumption			=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	yearly_consumption			=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	
	daily_expanse				=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	weekly_expanse				=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	monthly_expanse				=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	yearly_expanse				=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))	
	
	total_daily_expanse			=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	total_weekly_expanse		=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	total_monthly_expanse		=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))
	total_yearly_expanse		=	ArrayField(models.DecimalField(blank=True, null=True, max_digits=8, decimal_places=2))

	class Meta:
		managed 	=	True
		db_table 	=	'consumption'


	def __str__(self):
		return self.appliances



# R = V^2/P, E = t*P*(v/V)^2 
# E = p*t = v^2/R * t = t*P*(v/V)^2 
# i = v/R 

