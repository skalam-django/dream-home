from django.conf import settings
from dreamhome.utils import loggerf, printf
from django.db import connections
from django.contrib.auth.hashers import make_password

def create_groups_permissions(sender, **kwargs):
	printf('Creating groups permissions for apps models in postgres.')
	try:
		from django.contrib.auth.models import Group, Permission
		from django.contrib.contenttypes.models import ContentType
		from auth_app.permissions import MODEL_PERMS
		for user_type in settings.USER_TYPE:
			for user_group in settings.USER_GROUP:
				group, created = Group.objects.get_or_create(name=f"{user_type}-{user_group}")
		for content_type in ContentType.objects.all():
			if MODEL_PERMS.get(content_type.model):
				for perms, groups in MODEL_PERMS.get(content_type.model).items():
					if groups:
						codename= f'{perms}_{content_type.model}'
						name 	= f'Can {perms} {content_type.name}'
						perm_obj, created = Permission.objects.get_or_create(content_type=content_type, codename=codename, name=name)
						for group in groups:
							group_qs = 	Group.objects.filter(name=group)
							if group_qs.exists():
								group_obj = group_qs.first()
								group_obj.permissions.add(perm_obj)
								printf(f'{group_obj.name} Can {perms} {content_type.model}')
	except Exception as e:
		import traceback
		loggerf('In create_groups_permissions() database Error:',e, traceback.format_exc())							

def populate_default_users(sender, **kwargs):
	from django.contrib.auth import get_user_model
	username_list 	= 	[]
	email_list 		=	[]
	phone_number_list = []
	name_list 		=	[]


def popuplate_default_data(sender, **kwargs):
	pass	

def create_auth_token(sender, instance, created, **kwargs):
	if created==True:
		from rest_framework.authtoken.models import Token
		token 	= 	Token.objects.create(user=instance)
		loggerf(f'username: {instance.username}, token: {token.key}')


