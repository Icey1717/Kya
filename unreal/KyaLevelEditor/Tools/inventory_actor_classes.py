"""Inventory actor classes and remaining raw Create data in a bank.

Run headless:
    UnrealEditor-Cmd.exe BankTest.uproject -run=pythonscript \
        -script=Tools/inventory_actor_classes.py -unattended -NullRHI

Set KYA_ACTOR_BANK to override the LEVEL_1 bank path. The script imports the
bank into a temporary package (without saving) and prints one line per class:
actor count, actors with nonempty class data, maximum class-data size, and a
few example names.
"""
import os

import unreal

path = os.environ.get(
    'KYA_ACTOR_BANK',
    r'G:\repos\Kya\assets\CDEURO\LEVEL\LEVEL_1\LEVEL.BNK')
task = unreal.AssetImportTask()
task.filename = path
task.destination_path = '/Temp'
task.destination_name = 'KyaActorInventory'
task.automated = True
task.save = False
task.replace_existing = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
objects = task.get_objects()
if not objects:
    raise RuntimeError('Could not import %s' % path)
actors = objects[0].get_editor_property('scene_environment').get_editor_property('actors')

classes = {}
for actor in actors:
    class_id = actor.get_editor_property('class_id')
    entry = classes.setdefault(class_id, {'count': 0, 'raw': 0, 'max': 0, 'names': []})
    entry['count'] += 1
    raw = len(actor.get_editor_property('class_data'))
    if raw:
        entry['raw'] += 1
        entry['max'] = max(entry['max'], raw)
    if len(entry['names']) < 4:
        entry['names'].append(actor.get_editor_property('name'))

for class_id in sorted(classes):
    entry = classes[class_id]
    unreal.log('KYA_ACTOR_CLASS class=0x%02x count=%d withRaw=%d maxRaw=%d names=%s' %
               (class_id, entry['count'], entry['raw'], entry['max'], ','.join(entry['names'])))
unreal.log('KYA_ACTOR_CLASS_TOTAL actors=%d classes=%d withRaw=%d' %
           (len(actors), len(classes), sum(e['raw'] for e in classes.values())))
