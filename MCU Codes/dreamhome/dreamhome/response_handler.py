from dreamhome.utils import loggerf, printf
from rest_framework.response import Response
from django.utils.translation import gettext_lazy as _
from rest_framework.renderers import BaseRenderer, JSONRenderer, TemplateHTMLRenderer

RESPONSE_DICT = dict()



RESPONSE_DICT[200] = {"success" : True, "error" : False, "status" : 200, "message" : _("Everything went as planned"), "error_description" : None, "error_field" : None, "error_type" : None}


RESPONSE_DICT[400] = {"success" : True, "error" : True, "status" : 422, "message" : None, "error_description" : _("No Home Found. Please Add Home."), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[401] = {"success" : True, "error" : True, "status" : 422, "message" : None, "error_description" : _("No Room Found. Please Add Room."), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[402] = {"success" : True, "error" : True, "status" : 422, "message" : None, "error_description" : _("No Appliances Found. Please Add Appliance."), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[403] = {"success" : True, "error" : True, "status" : 422, "message" : None, "error_description" : _("No Operator Found. Please Add Operator."), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[407] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Invalid user"), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[408] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Inactive user"), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[409] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Port unavailable."), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[410] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Invalid dimension data."), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[411] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Invalid MAC/IMEI or key"), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[412] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Invalid IMEI"), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[416] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("No OTP exists for this phone number"), "error_field" : _("phone_number"), "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[417] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Invalid OTP"), "error_field" : _("otp"), "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[418] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("This OTP has already been used"), "error_field" : _("otp"), "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[419] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("This OTP has been expired"), "error_field" : _("otp"), "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[422] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("You are not allowed to perform this action"), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[423] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Inactive user"), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[425] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Token unavailable for this user"), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[430] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Phone number is required"), "error_field" : _("phone_number"), "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[431] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("The phone number entered is not valid."), "error_field" : _("phone_number"), "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[432] = {"success" : False, "error" : False, "status" : 422, "message" : None, "error_description" : _("Nothing to update"), "error_field" : None, "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[433] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Update of Email is not permitted."), "error_field" : _("email"), "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[439] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Blank email"), "error_field" : 'email', "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[440] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Invalid email"), "error_field" : 'email', "error_type" : _("BAD_REQUEST_ERROR")}
RESPONSE_DICT[441] = {"success" : False, "error" : True, "status" : 422, "message" : None, "error_description" : _("Invalid username/password."), "error_field" : 'username/password', "error_type" : _("BAD_REQUEST_ERROR")}

RESPONSE_DICT[500] = {"success" : False, "error" : True, "status" : 500, "message" : None, "error_description" : _("Something went wrong"), "error_field" : None, "error_type" : _("INTERNAL_SERVER_ERROR")}


class PlainTextRenderer(BaseRenderer):
    media_type = 'text/plain'
    format = 'text'
    def render(self, data, media_type=None, renderer_context=None):
        response = str(JSONRenderer().render(data, media_type, renderer_context))
        print(data, media_type, renderer_context, response)
        return response

class CustomResponse(Response):
	def __init__(self, data=None, status=None, content_type='application/json', *args, **kwargs):
		try:
			status = int(str(status))
		except Exception as e:
			loggerf(f"CustomResponse().__init__() Error: {e}, status: {status}")
			self.__init__({}, 500)

		if content_type=='application/json':
			if status in RESPONSE_DICT:
				self.data 	= 	{
								"success" 		: 	RESPONSE_DICT[status]["success"],
								"error" 		:	RESPONSE_DICT[status]["error"],
								"message" 		:	RESPONSE_DICT[status]["message"],
								"error_details"	:	{
													"description" 	: 	RESPONSE_DICT[status]["error_description"],
													"field" 		:	RESPONSE_DICT[status]["error_field"],
													"error_type" 	: 	RESPONSE_DICT[status]["error_type"],
								},
				}

				self.data.update(data)			
				self.status = 	RESPONSE_DICT[status]["status"]	
			else:
				self.__init__({}, 500)	

			self.accepted_renderer =  JSONRenderer()
		elif content_type=='text/html':
			self.data = data
			self.status = status
			self.accepted_renderer =  TemplateHTMLRenderer()
		elif content_type=='text/plain':
			self.data = data
			self.status = status			
			self.accepted_renderer =  PlainTextRenderer()

		self.renderer_context = {}	
		self.accepted_media_type = content_type
					
		super().__init__(data=self.data, status=self.status, content_type=content_type)



class CustomAPIResponse(Response):
	def __init__(self, view, request, *args, **kwargs):
		response = view.as_view()(request, *args, **kwargs)
		self.accepted_renderer = JSONRenderer()
		self.accepted_media_type = "application/json"
		self.renderer_context = {}
		data = response.data if hasattr(response , 'data') else (response.context_data if hasattr(response , 'context_data') else {})
		super().__init__(data=data, status=response.status_code)

class CustomViewResponse(object):
	__response = Response()
	def __init__(self, view, request, *args, **kwargs):
		if request.method == 'GET':
			view.template_name = kwargs.get('template_name')
			self.__response = view.as_view()(request, *args, **kwargs).render()
		elif request.method == 'POST':
			self.__response = view.as_view()(request, *args, **kwargs)

	def response(self, *args, **kwargs):
		return self.__response

