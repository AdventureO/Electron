from django.db import models
from django.utils import timezone


def now_date():
    return timezone.now()


class Machine(models.Model):
    name = models.CharField(max_length=200)
    objects = models.Manager()

    def __unicode__(self):
        return self.name

    @staticmethod
    def delete_everything():
        Machine.objects.all().delete()


class Stamp(models.Model):
    name = models.CharField(max_length=200)
    machine = models.ForeignKey(Machine, on_delete=models.CASCADE)
    objects = models.Manager()

    def __unicode__(self):
        return self.name

    def __str__(self):
        return self.name

    @staticmethod
    def delete_everything():
        Stamp.objects.all().delete()


class WorkCycle(models.Model):
    strikes_number = models.IntegerField(max_length=1000)
    date = models.DateTimeField(auto_now_add=True)
    stamp = models.ForeignKey(Stamp, on_delete=models.CASCADE)
    objects = models.Manager()

    @staticmethod
    def delete_everything():
        WorkCycle.objects.all().delete()


