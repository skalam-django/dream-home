from django.db import models
import datetime, json
from dreamhome.utils import (
							printf, loggerf, 
							CustomServerException, CustomClientException, 
							validate_phonenumber, 
							call_procedure,
							LazyEncoder,
							full_data, 
							data_handler
							)
from django.conf import settings
from django.db import transaction
from django.db.models import Q


class HomeManager(models.Manager):
	def get_all(self, *args, **kwargs):
		fields = [
						'id',
						'name',
						'is_active',
		]
		return data_handler(self.all(), fields=fields, exception=CustomClientException(400), **kwargs)
		
	def create_home(self, *args, **kwargs):
		with transaction.atomic(using = self.db):
			from appliances.models import Device, PortInfo
			device_obj = Device.objects.get_device(*args, **kwargs)
			obj = self.create(
							name=args[0].get('name'), 
							user=args[1],
						)
			device_obj.user = args[1]
			device_obj.home = obj
			device_obj.save()
			port_qs = PortInfo.objects.filter(device=device_obj)
			for port_obj in port_qs:
				port_obj.home = obj
				port_obj.user = args[1]
				port_obj.save()

class RoomManager(models.Manager):
	def get_room(self, *args, **kwargs):
		fields = [
						'id',
						'name',
						'is_active',
		]
		return data_handler(self.filter(home=args[0].get('hid'), user=args[1]), fields=fields, exception=CustomClientException(401), **kwargs)
	

	def create_room(self, *args, **kwargs):
		with transaction.atomic(using = self.db):
			obj = self.create(
							name 		=	args[0].get('name'), 
							category 	=	args[0].get('category'),
							home_id 	=	int(args[0].get('hid')),  
							user 		=	args[1]
						)
			from appliances.models import PortInfo
			port_qs = PortInfo.objects.validate_mac(*args, **kwargs)
			for port_obj in port_qs:
				port_obj.room = obj
				port_obj.save()


class DeviceManager(models.Manager):
	def whitelist_device(self, *args, **kwargs):
		imei = args[0]
		mac = args[1]
		if imei is not None and mac is not None:
			result = call_procedure('generate_device')
			if result is not None and len(result)>0:
				result = result[0]
				if result is not None and len(result)>0:
					result = result[0]
					if result  is not None and len(result)==2:
						ssid, password = result
						if ssid is not None and password is not None:
							obj = self.create(
											imei 	=	imei,
											mac 	= 	mac,
											ssid 	= 	ssid,
											password=	password,
										)
							return obj

	def authenticate(self, *args, **kwargs):					
		auth_using = kwargs.get('auth_using')
		key  = kwargs.get('key')
		qs = self.filter(Q(mac=auth_using) | Q(imei=auth_using)).filter(key=key)
		if not qs.exists():
			raise CustomClientException(411)
		return qs.first()

	def get_device(self, *args, **kwargs):
		qs = self.filter(imei=args[0].get('imei'), mac=args[0].get('mac'))
		if not qs.exists():
			raise CustomClientException(412)
		return qs.first()


class PortInfoManager(models.Manager):
	def all_ports(self, *args, **kwargs):
		fields = [
					'mac',
					'port',
					'is_used'
		]
		return data_handler(self.filter(device=args[1]), fields=fields, exception=CustomClientException(409), **kwargs)

	def whitelist_slaves(self, *args, **kwargs):
		dev_obj = args[0]
		slave_mac_list = args[1]
		slave_port_list = args[2]
		for idx, mac in enumerate(slave_mac_list):
			for port in slave_port_list[idx]:
				qs = self.filter(device=dev_obj, mac=mac, port=port)
				if not qs.exists():	
					obj = self.create(device=dev_obj, mac=mac, port=port) 

	def validate_mac(self, *args, **kwargs):	
		qs = self.filter(
							home_id = 	args[0].get('hid'), 
							mac 	=	args[0].get('mac'), 
							user 	=	args[1], 
							is_used =	False, 
						)
		if not qs.exists():
			raise CustomClientException(409)
		return qs

	def validate_port(self, *args, **kwargs):	
		qs = self.filter(
							home_id = 	args[0].get('hid'), 
							# room_id =	args[0].get('rid'), 
							user 	=	args[1], 
							is_used =	False, 
							port 	=	args[0].get('port'))
		if not qs.exists():
			raise CustomClientException(409)
		return qs.first()
		
	def get_avail_slaves(self, *args, **kwargs):
		avail_slaves = []
		qs = self.filter(
							home_id =	int(args[0].get('hid')), 
							# room_id =	args[0].get('rid'), 
							user 	=	args[1], 
							is_used =	False
							).distinct('mac')
		for obj in qs:
			avail_slaves.append(obj.mac)
		return avail_slaves

	def get_avail_ports(self, *args, **kwargs):
		avail_ports = []
		qs = self.filter(
							room_id =	args[0].get('rid'),
							home_id =	args[0].get('hid'), 
							# room_id =	args[0].get('rid'), 
							user 	=	args[1], 
							is_used =	False
							)
		for obj in qs:
			avail_ports.append(obj.port)
		return avail_ports


	def update_data(self, *args, **kwargs):
		qs = self.filter(device_id=args[1])
		op_data = args[0].get('op_data')
		print(op_data, "op_data")
		if op_data is not None:
			# op_data = json.loads(op_data)
			for mac, mac_val in op_data.items():
				for port, port_val in mac_val.items():
					port_qs = qs.filter(mac=mac, port=port)
					if port_qs.exists():
						port_obj  = port_qs.first()
						from appliances.models import Operator
						op_qs = Operator.objects.filter(port=port_obj, user_id=args[2], home_id=args[3])
						if op_qs.exists():
							op_obj = op_qs.first()
							if op_obj.is_active:
									op_obj.sensor_val = float(port_val[0]) if len(port_val)>0 and port_val[0] is not None else op_obj.sensor_val
									calibration_ref1 =  bool(port_val[2]) if len(port_val)>2 and port_val[2] is not None else not op_obj.calibration_ref1
									calibration_ref2 =  bool(port_val[3]) if len(port_val)>3 and port_val[3] is not None else not op_obj.calibration_ref2
									if op_obj.calibration_ref1 and calibration_ref1:
										op_obj.calibration_ref1 = False
									if op_obj.calibration_ref2 and calibration_ref2:	
										op_obj.calibration_ref2 = False
									op_obj.save()	
							ap_obj = op_obj.appliances
							if ap_obj.is_active:
								if ap_obj.manual==False:
									ap_obj.status = bool(port_val[1]) if len(port_val)>1 and port_val[1] is not None else ap_obj.status
									ap_obj.save()
									




class AppliancesManager(models.Manager):
	def get_appliances(self, *args, **kwargs):
		from appliances.models import Operator
		fields = [
						'id',
						'name',
						'room__category',
						'category',
						'is_active',
						'status',
						'manual',

		]

		return data_handler(
							self.filter(home=args[0].get('hid'), room=args[0].get('rid'), user=args[1]).order_by('-updated_at'), 
							fields=fields, 
							new_fields = {
										'id' : {
														'fields' : 	{
																		'operator_id' : ['id'],
																		'operator_category' : ['category'],
														},
														'instance' 	:	lambda x : Operator.objects.filter(appliances_id=x)
													},																							
							},
							func_fields={
											'category': lambda x, *args, **kwargs: settings.APPLIANCES_CATEGORY[int(x)],
											'operator_category': lambda x, *args, **kwargs: settings.OPERATOR_CATEGORY[int(x)],
										},
							exception=CustomClientException(402), 
							**kwargs
							)

	def add_appliances(self, *args, **kwargs):
		from appliances.models import PortInfo
		port_obj = PortInfo.objects.validate_port(*args, **kwargs)
		self.create(
						name 	=	args[0].get('name'), 
						category=	int(args[0].get('category')), 
						port 	=	port_obj,
						device  = 	port_obj.device,
						home_id =	args[0].get('hid'), 
						room_id =	args[0].get('rid'), 
						user 	=	args[1])
		port_obj.is_used = True
		port_obj.save()

	def update_states(self, *args, **kwargs):
		qs = self.filter(id=args[0].get('aid'), home_id=args[0].get('hid'), room_id=args[0].get('rid'), user=args[1])
		if qs.exists():
			obj = qs.first()
			obj.is_active 	= 	args[0].get('is_active', obj.is_active)
			obj.status 		= 	args[0].get('status', obj.status)
			obj.manual 		= 	args[0].get('manual', obj.manual)
			obj.save()
		else:
			raise Exception(500)

	def update_specifications(self, *args, **kwargs):
		qs = self.filter(id=args[0].get('aid'), home_id=args[0].get('hid'), room_id=args[0].get('rid'), user=args[1])
		if qs.exists():
			from appliances.models import PortInfo
			obj 				= 	qs.first()
			old_port_obj 		= 	obj.port
			new_port_obj 		= 	PortInfo.objects.validate_port(*args, **kwargs)
			obj.port 			=	new_port_obj
			obj.power_rating	=	args[0].get('power_rating', obj.power_rating)
			obj.voltage_rating	=	args[0].get('voltage_rating', obj.voltage_rating)
			obj.current_rating	=	args[0].get('current_rating', obj.current_rating)	
			new_port_obj.is_used= True
			old_port_obj.is_used= False
			new_port_obj.save()
			old_port_obj.save()
			obj.save()
		else:
			raise Exception(500)


	def delete_appliances(self, *args, **kwargs):
		qs = self.filter(id=args[0].get('aid'), home_id=args[0].get('hid'), room_id=args[0].get('rid'), user=args[1])
		if qs.exists():
			obj = qs.first()
			obj.port.is_used = False
			obj.port.save()
			obj.delete()
		else:
			raise Exception(500)

	def details(self, *args, **kwargs):
		fields = [
						'id',
						'name',
						'port__port',
						'power_rating',
						'voltage_rating',
						'active_dur',
						'total_active_dur'						
		]

		return data_handler(
							self.filter(
								id=args[0].get('aid'), 
								home_id=args[0].get('hid'), 
								room_id=args[0].get('rid'), 
								user=args[1]
							),
							exception=CustomClientException(402), 
							**kwargs
						)

	def get_appliance_operator_port_status(self, *args, **kwargs):
		qs = self.filter(device = args[0]).order_by('id')
		ap_data  = {}
		op_data  = {}
		op_ap_mac_map = {}
		from appliances.models import Operator
		op_qs = Operator.objects.get_for_device(qs, *args, **kwargs)
		for obj in qs:
			op_mac = ""
			op_port = ""
			if op_qs.exists():
				for op_obj in op_qs:
					op_value = [op_obj.category, 1 if op_obj.is_active else 0, op_obj.sensor_max_val, op_obj.sensor_min_val, 1 if op_obj.calibration_ref1 else 0, 1 if op_obj.calibration_ref2 else 0, obj.port.mac, obj.port.port]
					dimensions_data = []
					if (op_obj.category==settings.OPERATOR_CATEGORY.index("Water Tank")):
						capacity = op_obj.dimensions_data.get("capacity") or 0
						height = op_obj.dimensions_data.get("height") or 0
						circumstances = op_obj.dimensions_data.get("circumstances") or 0
						dimensions_data = [capacity, height, circumstances]
						op_value += dimensions_data
					op_mac = op_obj.port.mac
					op_port = op_obj.port.port
					try:
						op_data[op_mac].update({op_port : op_value})
					except KeyError as e:	
						op_data[op_mac] = {op_port : op_value}
			ap_value = [obj.category, 1 if obj.is_active else 0, 1 if obj.manual else 0, 1 if obj.status else 0, op_mac, op_port]
			try:
				ap_data[obj.port.mac].update({obj.port.port : ap_value})
			except KeyError as e:	
				ap_data[obj.port.mac] = {obj.port.port : ap_value}	
		return {'ap_data' : ap_data, 'op_data' : op_data}




class OperatorManager(models.Manager):
	def get_all(self, *args, **kwargs):
		fields = [
						'id',
						'category',
		]
		return data_handler(self.filter(home_id=args[0].get('hid'), room_id=args[0].get('rid'), user=args[1]), fields=fields, exception=CustomClientException(402), **kwargs)
	
	def get_for_device(self, *args, **kwargs):
		op_qs = self.filter(
								device 	=	args[1],
								appliances__in = [obj for obj in args[0]]
							).order_by('appliances_id')
		return op_qs


	def get_single(self, *args, **kwargs):
		fields = [
						'id',
						'name',
						'category',
						'sensor_val',
						'sensor_max_val',
						'sensor_min_val',
						'dimensions_data',
						'appliances__name',
						'appliances__category'		
				]
		return data_handler(
							self.filter(id=args[0].get('oid'), home_id=args[0].get('hid'), room_id=args[0].get('rid'), appliances_id=args[0].get('aid'), user=args[1]), 
							fields=fields, 
							func_fields = {
											'category' : lambda x, *args, **kwargs : settings.OPERATOR_CATEGORY[x],
											'appliances__category' : lambda x, *args, **kwargs : settings.APPLIANCES_CATEGORY[x],
											},

							exception=CustomClientException(403), 
							**kwargs
							)	

	def update_sensor_ref_value(self, *args, **kwargs):
		qs = self.filter(id=args[0].get('oid'), home_id=args[0].get('hid'), room_id=args[0].get('rid'), user=args[1])
		if qs.exists():
			obj = qs.first()
			obj.sensor_max_val  = 	 args[0].get('sensor_max_val')
			obj.sensor_min_val 	= 	 args[0].get('sensor_min_val')
			obj.save()
		else:
			CustomClientException(403)	

	def create_operator(self, *args, **kwargs):
		category = args[0].get('category')
		dimensions_data = args[0].get('dimensions_data')
		if dimensions_data is None or len(dimensions_data)==0:
			raise CustomClientException(410)
		if category==settings.OPERATOR_CATEGORY.index('Water Tank'):
			capacity = dimensions_data.get('capacity')
			height = dimensions_data.get('height')
			circumstances = dimensions_data.get('circumstances')
			if capacity is None or height is None or circumstances is None:
				raise CustomClientException(410)
		from appliances.models import PortInfo
		self.create(
					appliances_id 	=	args[0].get('aid'),
					home_id 		= 	args[0].get('hid'),
					room_id 		= 	args[0].get('rid'),	
					port 			= 	PortInfo.objects.validate_port(*args, **kwargs),
					user 			= 	args[1],	
					name 			= 	args[0].get('name'), 
					category 		=	category,
					dimensions_data =	dimensions_data,
			)
	













