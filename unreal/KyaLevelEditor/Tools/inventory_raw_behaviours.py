"""Inventory actor behaviour payloads that are still untyped in a bank.

Run headless:
    UnrealEditor-Cmd.exe KyaEditorTest.uproject -run=pythonscript \
        -script=Tools/inventory_raw_behaviours.py -unattended -NullRHI

Set KYA_ACTOR_BANK to override the LEVEL_1 bank path. A payload counts as typed
when its `Data` array is empty (fully decoded) or any behaviour typed flag is set.
The script exits with an error if any payload is still raw, so it can gate a
milestone.
"""
import os

import unreal

path = os.environ.get(
    'KYA_ACTOR_BANK',
    r'G:\repos\Kya\assets\CDEURO\LEVEL\LEVEL_1\LEVEL.BNK')
task = unreal.AssetImportTask()
task.filename = path
task.destination_path = '/Temp'
task.destination_name = 'KyaRawBehaviourInventory'
task.automated = True
task.save = False
task.replace_existing = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
objects = task.get_objects()
if not objects:
    raise RuntimeError('Could not import %s' % path)
actors = objects[0].get_editor_property('scene_environment').get_editor_property('actors')

FLAGS = [
    'has_typed_bonus_behaviour',
    'has_typed_bridge_basic_behaviour',
    'has_typed_bridge_wind_behaviour',
    'has_typed_catch_by_trap_behaviour',
    'has_typed_cinematic_behaviour',
    'has_typed_cluster_zones',
    'has_typed_commander_behaviour',
    'has_typed_companion_behaviour',
    'has_typed_dca_controlled_behaviour',
    'has_typed_dca_trajectory_behaviour',
    'has_typed_fog_behaviour',
    'has_typed_fruit_behaviour',
    'has_typed_fx_behaviour',
    'has_typed_gravity_bell_behaviour',
    'has_typed_gravity_fall_behaviour',
    'has_typed_hero_unknown_behaviour',
    'has_typed_inventory_behaviour',
    'has_typed_micken_behaviour',
    'has_typed_nativ_behaviour',
    'has_typed_platform_behaviour',
    'has_typed_projectile_behaviour',
    'has_typed_shoot_fire_behaviour',
    'has_typed_shoot_wave_behaviour',
    'has_typed_switch_behaviour',
    'has_typed_trap_behaviour',
    'has_typed_weapon_behaviour',
    'has_typed_wolfen_behaviour',
]

raw = {}
typed_count = 0
for actor in actors:
    if len(actor.get_editor_property('class_data')):
        raw[('classdata', actor.get_editor_property('class_id'),
             len(actor.get_editor_property('class_data')))] = \
            raw.get(('classdata', actor.get_editor_property('class_id'),
                     len(actor.get_editor_property('class_data'))), 0) + 1
    for behaviour in actor.get_editor_property('behaviours'):
        size = len(behaviour.get_editor_property('data'))
        if size == 0:
            typed_count += 1
            continue
        typed = False
        for flag in FLAGS:
            try:
                if behaviour.get_editor_property(flag):
                    typed = True
                    break
            except Exception:
                pass
        if typed:
            typed_count += 1
            continue
        key = (actor.get_editor_property('class_id'), behaviour.get_editor_property('id'), size)
        raw[key] = raw.get(key, 0) + 1

for key in sorted(raw):
    unreal.log('KYA_RAW_BEHAVIOUR class=0x%02x id=%d size=%d count=%d' %
               (key[0], key[1], key[2], raw[key]))
unreal.log('KYA_RAW_BEHAVIOUR_TOTAL raw=%d typed=%d' % (sum(raw.values()), typed_count))
if raw:
    raise RuntimeError('%d raw Create payloads remain' % sum(raw.values()))
