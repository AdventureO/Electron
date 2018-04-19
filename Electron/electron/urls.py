from django.conf.urls import url
from . import views

urlpatterns = [
    # /electron
    url(r'^$', views.main_page, name='main_page'),

    # /electron/add_data
    url(r'^add_data/$', views.add_data, name='add_data'),

    # /electron/machines
    url(r'^machines/$', views.machines, name='machines'),

    # /electron/machines/<machine_id>
    url(r'^machines/(?P<machine_id>[0-9]+)/$', views.stamps, name="stamps"),

    # /electron/machines/<machine_id>/<stamp_id>
    url(r'^machines/(?P<machine_id>[0-9]+)/(?P<stamp_id>[0-9]+)/$', views.work_cycles, name="work_cycles"),

]