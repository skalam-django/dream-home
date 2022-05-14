from django.apps import AppConfig
from django.utils.translation import ugettext_lazy as _
from django.db.models.signals import post_migrate
from appliances.signals import device_proc, popuplate_default_data

class AppliancesConfig(AppConfig):
	name = 'appliances'
	verbose_name = _('appliances')
	def ready(self):
		post_migrate.connect(device_proc, sender=self)
		post_migrate.connect(popuplate_default_data, sender=self)
