from .base import *

PRODUCTION = False

DEBUG = True

SERVICES = False

PRINT_EANABLED = True

TRACEBACK_OFF = False

SECRET_KEY = '9ewkumuw^0-k+_xij(g^byzycpxo!j(xy6-hy^dh#%8*4#ik5r'

ALLOWED_HOSTS = ['127.0.0.1', 'localhost', 'f10843f1c577.ngrok.io','192.168.43.219']

INSTALLED_APPS += [
   
]

STATIC_VERSION = 1.0


DATABASES['default'] = {
                        'ENGINE'    :   'django.db.backends.postgresql_psycopg2',
                        'NAME'      :   'dreamhome', 
                        'USER'      :   'postgres',
                        'PASSWORD'  :   'admin@123',
                        'HOST'      :   'localhost',
                        'PORT'      :   5432,
                        'TIME_ZONE' :   None,
                        }


GENDER_API_KEY = 'xmxLvFVQdEnXErSQBC'