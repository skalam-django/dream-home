from django.template.loader import select_template
from django.views.generic import TemplateView
from django.conf import settings
import json 
from dreamhome.utils import loggerf, printf

layout_dict = {"ErrorView" 	: 	{'title' : "Error", 'footer' : False}}
sidebar_dict = {}


def get_template(module=None, template_name=None):
	try:
		print(module, template_name, "template_name")
		if module is not None:
			return select_template([f"{module.split('.')[0]}/{template_name}", 'dreamhome/page_does_not_exist.html']).template.name
		return select_template(['dreamhome/page_does_not_exist.html']).template.name
	except Exception as e:
		printf("get_template Error : ", e)
		return select_template(['dreamhome/page_does_not_exist.html']).template.name

def reorder(arr, default_idx):
	old_arr = arr.copy()
	arr[0]  = arr[default_idx]
	old_arr.pop(default_idx)
	for idx, e in enumerate(old_arr):
		arr[idx+1] =  old_arr[idx]
	return arr	




def register_layout(view_name, title, footer=True):
	global layout_dict
	layout_dict[view_name] = {"title" : title, "footer" : footer}


def register_sidebar(user_group_arr, id, caption, get_url='/', post_url=None, icon=""):
	global sidebar_dict
	for user_group in user_group_arr:
		try:
			sidebar_dict[user_group]
		except:
			sidebar_dict[user_group] = []

		sidebar_dict[user_group] += [{
												"id" 		: 	id, 
												"caption" 	: 	caption, 
												"urls" 		: 	{"get": get_url, "post": post_url},
												"icon" 		:	icon,
										}]


class SideBar(object):
	def get(self, user_group):
		try:		
			global sidebar_dict
			return sidebar_dict.get(user_group)
		except:
			return []


class BaseTemplateView(TemplateView):
	template_name 	= 	get_template()
	default_bar_idx = 	None	
	extra_context 	= 	{
		"site_title" : "Login",
		"logout_url"  : "auth:logout",
		"next_url" : "/",
		"back_btn" : False,
	}
	cookies_dict = {}

	def __init__(self, *args, **kwargs):
		self.extra_context["this_app"] = self.__module__.split('.')[0]
		self.extra_context["base_template"] = get_template("dreamhome", "base.html")
		self.extra_context["layout_1"] = get_template("dreamhome", "layout_1.html")
		self.extra_context["edit_profile"] = {
												"caption" : "Edit Profile", 
												"icon" 	:	"account_circle",
												"urls" : {
															"get":"/auth/user_profile/",
															"post":"auth/api/v1/update_profile/"
														}
												}
		self.extra_context["logout"] = {
												"caption" : "Logout", 
												"icon" 	:	"exit_to_app",
												"urls" : {
															"get":"/",
															"post":"/auth/logout/"
														}
												}										
		# self.extra_context["footer"] = False


	def get_context_data(self, *args, **kwargs):
		printf("BaseTemplateView().get_context_data() template_name: ", self.template_name)
		user_obj = self.request.user		
		try:
			path = self.__class__.__name__
			if path	is None:
				path = "ErrorView"
		except:
			path = "ErrorView"
		if user_obj.is_authenticated == True:
			user_type = user_obj.user_type
			user_group_qs = user_obj.groups.filter(name__in=[f"{settings.USER_TYPE[user_type]}-{user_group}" for user_group in settings.USER_GROUP])
			self.user_group  = []
			if user_group_qs.exists():
				self.user_group = user_group_qs.first().name
				
			sidebar_dict = SideBar().get("_".join(self.user_group.split("-")))
			self.extra_context['show_sidebar'] = len(sidebar_dict)>0 if sidebar_dict is not None else True
			self.extra_context['sidebar_dict'] = sidebar_dict
		global layout_dict			
		self.extra_context['layout'] = layout_dict.get(path)

		context = super(BaseTemplateView, self).get_context_data(*args, **kwargs)    
		context.update(self.extra_context)
		print("context: ",context)
		return context


	def dispatch(self, request, *args, **kwargs):
		printf("BaseTemplateView().dispatch()")
		response = super(BaseTemplateView, self).dispatch(request, *args, **kwargs)
		for key, value in self.cookies_dict.items():
			response.set_cookie(key, value)
		return response
