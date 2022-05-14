from django.apps import AppConfig
from django.utils.translation import ugettext_lazy as _
from django.contrib.auth import get_user_model
from django.db.models.signals import post_save, post_migrate
from auth_app.signals import (
								create_groups_permissions, 
								populate_default_users, 
								popuplate_default_data, 
								create_auth_token
							)

class AuthAppConfig(AppConfig):
	name = 'auth_app'
	verbose_name = _('auth_app')
	def ready(self):
		post_migrate.connect(create_groups_permissions, sender = self)
		post_migrate.connect(populate_default_users, sender = self)
		post_migrate.connect(popuplate_default_data, sender = self)
		post_save.connect(create_auth_token, sender=get_user_model())
		

