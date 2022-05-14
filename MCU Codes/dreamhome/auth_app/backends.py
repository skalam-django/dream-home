from django.conf import settings
from dreamhome.utils import loggerf, printf, CustomServerException, CustomClientException
from dreamhome.response_handler import CustomResponse
from django.utils.translation import gettext_lazy as _
from rest_framework import exceptions


class BaseBackend:
	def authenticate(self, request, **kwargs):
		return None

class MACIMEIBackend(BaseBackend):
	def authenticate(self, request, **credentials):
		from appliances.models import Device
		try:
			if None in [credentials.get('auth_using'), credentials.get('key')]:
				return None
			device_obj = Device.objects.authenticate(**credentials)
			setattr(device_obj, 'tracking', False)
			setattr(device_obj, 'user_type', settings.USER_TYPE.index("device"))
			if self.user_can_authenticate(device_obj):
				request.device = device_obj
				setattr(request.device, 'is_authenticated', True)
				return device_obj
			request.device = None
			setattr(request.device, 'is_authenticated', False)
			return None	
		except (CustomServerException, CustomClientException, Exception) as e:	
			loggerf(f"MACIMEIBackend().authenticate() Error: {e}")
			response = 	CustomResponse(data={}, status=e)
			raise exceptions.AuthenticationFailed(_(response.data["error_details"]["description"]))

	def user_can_authenticate(self, user):
		is_active = getattr(user, 'is_active', None)
		return is_active or is_active is None


