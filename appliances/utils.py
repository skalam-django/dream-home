from appliances.models import PortInfo
def update_sensor_status(*args, **kwargs):
	PortInfo.objects.update_data(*args, **kwargs)