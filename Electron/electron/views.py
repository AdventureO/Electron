from django.shortcuts import render, get_object_or_404
from django.views.decorators.csrf import csrf_exempt
from django.http import JsonResponse, HttpResponse
from .models import Machine, Stamp, WorkCycle
import json


def main_page(request):
    return render(request, 'electron/main_page.html')


def machines(request):
    all_machines = Machine.objects.all()
    return render(request, 'electron/machines.html', {'all_machines': all_machines})


def stamps(request, machine_id):
    machine = get_object_or_404(Machine, id=machine_id)
    return render(request, 'electron/stamps.html', {'machine': machine})


def work_cycles(request, machine_id, stamp_id):
    machine = get_object_or_404(Machine, id=machine_id)
    stamp = get_object_or_404(Stamp, id=stamp_id)
    return render(request, 'electron/workcycles.html', {'machine': machine, 'stamp': stamp})


@csrf_exempt
def add_data(request):
    if request.method == "POST":
        try:
            a = request.body.decode("utf-8")
            data = json.loads(a)

            if Machine.objects.filter(name=data['machine_id']).count() == 0:
                machine = Machine(name=data['machine_id'])
                machine.save()
            else:
                machine = Machine.objects.get(name=data['machine_id'])

            if Stamp.objects.filter(name=data['stamp_id'], machine=machine).count() == 0:
                stamp = Stamp(name=data['stamp_id'], machine=machine)
                stamp.save()
            else:
                stamp = Stamp.objects.get(name=data['stamp_id'], machine=machine)

            work_cycle = WorkCycle(strikes_number=int(data['strikes_number']), stamp=stamp)
            work_cycle.save()
        except:
            return JsonResponse({"success": False})

        return JsonResponse(data)
