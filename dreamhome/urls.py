"""dreamhome URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/3.1/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path, include
from django.conf.urls import url
from django.conf import settings
from django.views.generic.base import RedirectView
from django.conf.urls.static import static
from django.views.generic import TemplateView


urlpatterns = [
    url(r'^favicon\.ico$', RedirectView.as_view(url='/static/favicon.ico', permanent=True)),
    url(r'^manifest.json', (TemplateView.as_view(template_name="dreamhome/manifest.json", content_type='application/json', )), name='manifest.json'),
    url(r'^sw.js', (TemplateView.as_view(template_name="dreamhome/sw.js", content_type='text/javascript', )), name='sw.js'),
    path('admin/', admin.site.urls),
    path('auth/', include('auth_app.urls', namespace="dh-auth")),
    path('', include('appliances.urls', namespace="appliances")),
]+ static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)
