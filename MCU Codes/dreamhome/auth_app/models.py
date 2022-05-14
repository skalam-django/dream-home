from django.db import models
import jsonfield
from django.conf import settings
from phonenumber_field.modelfields import PhoneNumberField
from django.contrib.auth.models import AbstractUser
from django.db.models.signals import m2m_changed
from django.dispatch import receiver
from django.db.utils import IntegrityError
from django.utils.translation import gettext_lazy as _
from auth_app.managers import (
                                    AuthUserManager,
                                    UserTrackingManager,    
                                )

from django.core.validators import URLValidator


USER_TYPE = tuple([(idx, user_type) for idx, user_type in enumerate(settings.USER_TYPE)])

class AuthUser(AbstractUser):
    phone_number 	=	PhoneNumberField(region='IN', null=True)
    email 			= 	models.EmailField(null=True)
    user_type 		=	models.SmallIntegerField(choices=USER_TYPE, default=USER_TYPE[0][0])
    tracking        =   models.BooleanField(default=False)
    created_at		=	models.DateTimeField(auto_now_add=True)
    updated_at		=	models.DateTimeField(auto_now=True)	
    objects 		=	AuthUserManager()

    class Meta:
        managed = True
        db_table = 'auth_user'

    def __str__(self):
        return f"{self.id}-{self.username}"

        
class UserTracking(models.Model):
    user_id         =   models.IntegerField(default=0)
    ident           =   models.GenericIPAddressField()
    user_agent      =   models.CharField(max_length=255, null=True)
    authorization   =   models.CharField(max_length=255, null=True)
    app             =   models.CharField(max_length=10, null=True)
    url             =   models.URLField(max_length = 255, null=True) #, validators=[URLValidator()])
    request_payload =   jsonfield.JSONField(null=True, blank=True)   
    response_data   =   jsonfield.JSONField(null=True, blank=True)
    response_status =   models.CharField(max_length=10, null=True)
    created_at      =   models.DateTimeField(auto_now_add=True)
    updated_at      =   models.DateTimeField(auto_now=True)   
    objects         =   UserTrackingManager() 
    class Meta:
        managed = True
        db_table = 'user_tracking'

    def __str__(self):
        return F"{self.id}-{self.user_id}"    



