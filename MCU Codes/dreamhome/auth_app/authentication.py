import inspect
import re
from django.conf import settings
from django.core.exceptions import ImproperlyConfigured, PermissionDenied
from django.utils.module_loading import import_string
from django.contrib.auth.signals import user_login_failed
import base64
import binascii
from django.contrib.auth import get_user_model
from django.utils.translation import gettext_lazy as _
from rest_framework import HTTP_HEADER_ENCODING, exceptions
from django.utils.module_loading import import_string
from dreamhome.utils import loggerf, printf, CustomServerException, CustomClientException


def get_authorization_header(request):
    auth = request.META.get('HTTP_AUTHORIZATION', b'')
    if isinstance(auth, str):
        auth = auth.encode(HTTP_HEADER_ENCODING)
    return auth

def load_backend(path):
    return import_string(path)()

def _get_backends(auth_backend, return_tuples=False):
    backends = []
    backend_arr = import_string(f"dreamhome.settings.{auth_backend}")
    for backend_path in backend_arr:
        backend = load_backend(backend_path)
        backends.append((backend, backend_path) if return_tuples else backend)
    if not backends:
        raise ImproperlyConfigured(
            'No authentication backends have been defined. Does '
            'AUTHENTICATION_BACKENDS contain anything?'
        )
    return backends


def get_backends():
    return _get_backends(return_tuples=False)

def _clean_credentials(credentials):
    SENSITIVE_CREDENTIALS = re.compile('api|token|key|secret|password|signature|otp|dh', re.I)
    CLEANSED_SUBSTITUTE = '********************'
    for key in credentials:
        if SENSITIVE_CREDENTIALS.search(key):
            credentials[key] = CLEANSED_SUBSTITUTE
    return credentials


def authenticate(auth_backend, request=None, **credentials):
    for backend, backend_path in _get_backends(auth_backend, return_tuples=True):
        try:
            inspect.getcallargs(backend.authenticate, request, **credentials)
        except TypeError:
            continue
        try:
            user = backend.authenticate(request, **credentials)
        except PermissionDenied:
             break
        if user is None:
            continue
        user.backend = backend_path
        return user
    user_login_failed.send(sender=__name__, credentials=_clean_credentials(credentials), request=request)


class BaseAuthentication:
    def authenticate(self, request):
        raise NotImplementedError(".authenticate() must be overridden.")
    def authenticate_header(self, request):
        pass

class MACIMEIAuthentication(BaseAuthentication):
    www_authenticate_realm = 'api'
    auth_backend = 'MAC_IMEI_AUTHENTICATION_BACKENDS'
    def authenticate(self, request):
        try:
            auth = get_authorization_header(request).split()
            if auth is None  or len(auth)==0 or auth[0].lower() != b'dh':
                return None
            printf("********   MACIMEIAuthentication   ********")
            if len(auth) == 1:
                msg = _('Invalid MAC/IMEI header. No credentials provided.')
                raise exceptions.AuthenticationFailed(msg)
            elif len(auth) > 2:
                msg = _('Invalid MAC/IMEI header. Credentials string should not contain spaces.')
                raise exceptions.AuthenticationFailed(msg)

            try:
                auth_parts = base64.b64decode(auth[1]).decode(HTTP_HEADER_ENCODING).split('|')
            except (TypeError, UnicodeDecodeError, binascii.Error):
                msg = _('Invalid MAC/IMEI header. Credentials not correctly base64 encoded.')
                raise exceptions.AuthenticationFailed(msg)
            if len(auth_parts)!=2:
                msg = _('Invalid MAC/IMEI Credentials.')
                raise exceptions.AuthenticationFailed(msg)            
            auth_using, key = auth_parts[0], auth_parts[1]
            printf('auth_key, key: ',auth_using, key)
            return self.authenticate_credentials(auth_using, key, request)

        except (CustomServerException, CustomClientException, Exception) as e:    
            loggerf(f"MACIMEIAuthentication().authenticate() Error: {e}")
            raise type(e)(e)         

    def authenticate_credentials(self, auth_using, key, request=None):
        credentials = {
            'auth_using'  :   auth_using,
            'key'   :   key
        }
        device = authenticate(self.auth_backend, request=request, **credentials)
        if device is None:
            raise exceptions.AuthenticationFailed(_('Invalid MAC/IMEI or Key.'))
        if not device.is_active:
            raise exceptions.AuthenticationFailed(_('Device inactive or deleted.'))
        return (device, None)

    def authenticate_header(self, request):
        return 'DH realm="%s"' % self.www_authenticate_realm

