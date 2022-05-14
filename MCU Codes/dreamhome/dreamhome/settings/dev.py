from .base import *
from decouple import config

PRODUCTION = False

DEBUG = True

TRACEBACK_OFF = False

try:
    PRINT_EANABLED = True if (config('sl_PRINT') is not None and config('sl_PRINT')=='True') else False
except:
    PRINT_EANABLED = False


try:
    SERVICES = True if (config('sl_SERVICES') is not None and config('sl_SERVICES')=='True') else False
except:
    SERVICES = False

try:
    SECRET_KEY = config('sl_SECRET_KEY')
except:
    SECRET_KEY = '9ewkumuw^0-k+_xij(g^byzycpxo!j(xy6-hy^dh#%8*4#ik5r'

ALLOWED_HOSTS = ["localhost", ]

INSTALLED_APPS += [
   
]


STATIC_ROOT = BASE_DIR / "../static/"
MEDIA_ROOT = BASE_DIR / '../data/'
STATIC_VERSION = 1.0


try:
    SSL = True if os.environ.get('sl_SSL') is not None and os.environ.get('sl_SSL')=='True' else False
except:
    SSL = False    

if SSL == True:
    SECURE_SSL_REDIRECT             =   True
    SECURE_PROXY_SSL_HEADER         =   ('HTTP_X_FORWARDED_PROTO', 'https')
    CSRF_COOKIE_SECURE              =   True
    SECURE_HSTS_SECONDS             =   100000
    SECURE_HSTS_INCLUDE_SUBDOMAINS  =   True
    SECURE_HSTS_PRELOAD             =   True
    SECURE_REDIRECT_EXEMPT          =   ["^insecure/"]

STATIC_ROOT = os.path.join(BASE_DIR, "../static/")
MEDIA_ROOT = os.path.join(BASE_DIR, '../data/')
STATIC_VERSION = 1.0

DATABASES['default'] = {
                        'ENGINE'    :   'django.db.backends.postgresql_psycopg2',
                        'NAME'      :   config('sl_NAME'), 
                        'USER'      :   config('sl_USER'),
                        'PASSWORD'  :   config('sl_PASSWORD'),
                        'HOST'      :   config('sl_HOST'),
                        'PORT'      :   config('sl_PORT'),
                        'TIME_ZONE' :   None,
                        }


LOGGING = {
    'version': 1,
    'disable_existing_loggers': False,
    'formatters': {
        'standard': {'format': '%(asctime)s  [%(levelname)s] %(name)s: %(message)s '},
    },
    'handlers': {
        'file': {
            'level': 'DEBUG',
            'class': 'logging.handlers.RotatingFileHandler',
            'filename': 'debug.log',
            'maxBytes': 1024 * 1024 * 50,
            'backupCount': 5,
            'formatter': 'standard',
        },
        'request_handler': {
            'level': 'WARNING',
            'class': 'logging.handlers.RotatingFileHandler',
            'filename': 'django_request.log',
            'maxBytes': 1024 * 1024 * 50,
            'backupCount': 5,
            'formatter': 'standard',
        },
    },
    'loggers': {

        '': {
            'handlers': ['file'],
            'level': 'DEBUG',
            'propagate': True,
        },

        'django': {
            'handlers': ['file'],
            'level': 'DEBUG',
            'propagate': True,
        },
        'django.request': {
            'handlers': ['request_handler'],
            'level': 'DEBUG',
            'propagate': True,
        },
    },
}
