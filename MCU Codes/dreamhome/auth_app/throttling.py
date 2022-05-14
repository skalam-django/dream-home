from rest_framework.throttling import SimpleRateThrottle

class DeviceRateThrottle(SimpleRateThrottle):
    scope = 'device'

    def get_cache_key(self, request, view):
        if request.user.is_authenticated:
            if hasattr(request, 'device'):
                if request.device is not None and request.device.is_active:
                    ident = request.device.pk
                else:
                    ident = self.get_ident(request)
            else:  
                ident = request.user.pk
            return self.cache_format % {
                'scope': self.scope,
                'ident': ident
            }

