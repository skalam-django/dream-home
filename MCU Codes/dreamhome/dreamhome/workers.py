from __future__ import absolute_import
from django.conf import settings
from dreamhome.celery import app
from celery import Task
from celery.decorators import task
from celery.utils.log import get_task_logger
from django.conf import settings
import datetime
import time
from dreamhome.utils import loggerf, printf, validate_phonenumber
from django.utils.module_loading import import_string
import traceback


class ProcessQueue(Task):
	def run(self, method_path, method_name, *args, **kwargs):
		time.sleep(1)
		try:

			import_string(f"{method_path}.{method_name}")(*args)

		except Exception as e:
			error = f'{datetime.datetime.now()} [ERROR] ProcessQueue().run() : {e}'
			if settings.DEBUG:
				printf(error, traceback.format_exc())
			else:	
				loggerf(error, traceback.format_exc(), info=True)	
							
ProcessQueue = app.register_task(ProcessQueue())