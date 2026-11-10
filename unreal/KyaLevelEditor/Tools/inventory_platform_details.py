"""Inventory class 0x0d moving-platform Create data in a bank.

Run headless:
    UnrealEditor-Cmd.exe BankTest.uproject -run=pythonscript \
        -script=Tools/inventory_platform_details.py -unattended -NullRHI

Set KYA_PLATFORM_BANK to override the LEVEL_1 bank path. The script imports the
bank into a temporary package (without saving) and prints counts plus the
remaining raw class-data suffix size.
"""
import os

import unreal

path = os.environ.get(
    'KYA_PLATFORM_BANK',
    r'G:\repos\Kya\assets\CDEURO\LEVEL\LEVEL_1\LEVEL.BNK')
task = unreal.AssetImportTask()
task.filename = path
task.destination_path = '/Temp'
task.destination_name = 'KyaPlatformInventory'
task.automated = True
task.save = False
task.replace_existing = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
objects = task.get_objects()
if not objects:
    raise RuntimeError('Could not import %s' % path)
actors = objects[0].get_editor_property('scene_environment').get_editor_property('actors')

platforms = prefixes = details = cameras = sounds_a = sounds_b = 0
friction_zones = friction_actors = max_class_data = 0
camera_actors = []
for actor in actors:
    if actor.get_editor_property('class_id') != 0x0d:
        continue
    platforms += 1
    prefixes += 1 if actor.get_editor_property('has_platform_prefix') else 0
    details += 1 if actor.get_editor_property('has_platform_details') else 0
    camera_records = actor.get_editor_property('platform_cameras')
    sound_a = actor.get_editor_property('platform_sounds_a')
    sound_b = actor.get_editor_property('platform_sounds_b')
    zones = actor.get_editor_property('platform_no_friction_zones')
    cameras += len(camera_records)
    sounds_a += len(sound_a)
    sounds_b += len(sound_b)
    friction_zones += len(zones)
    for zone in zones:
        friction_actors += len(zone.get_editor_property('actors'))
    max_class_data = max(max_class_data, len(actor.get_editor_property('class_data')))
    if camera_records:
        camera_actors.append((actor.get_editor_property('index'),
                              actor.get_editor_property('name'), len(camera_records)))
unreal.log('KYA_PLATFORM_DUMP actors=%d prefixes=%d details=%d cameras=%d soundsA=%d soundsB=%d frictionZones=%d frictionActors=%d maxClassData=%d' %
           (platforms, prefixes, details, cameras, sounds_a, sounds_b,
            friction_zones, friction_actors, max_class_data))
for item in camera_actors:
    unreal.log('KYA_PLATFORM_CAMERA_ACTOR %s' % (item,))
