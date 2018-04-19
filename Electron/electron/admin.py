from django.contrib import admin
from . import models

# Register your models here.
admin.site.register(models.Machine)
admin.site.register(models.Stamp)
admin.site.register(models.WorkCycle)