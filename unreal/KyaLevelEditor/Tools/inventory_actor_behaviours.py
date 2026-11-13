"""Inventory the serialized behaviour payloads of one actor class.

Run headless from `unreal/`:
    UnrealEditor-Cmd.exe BankTest.uproject -run=pythonscript \
        -script=KyaLevelEditor/Tools/inventory_actor_behaviours.py -unattended -NullRHI

Environment:
    KYA_ACTOR_BANK   bank path (defaults to LEVEL_1)
    KYA_ACTOR_CLASS  class id in decimal or 0x form (defaults to 0x1c Projectile)

Output is one `KYA_ACTOR_BEHAVIOUR` line per distinct (id, payload size, typed
flags) combination, with an example actor name, so a family audit can reconcile
source behaviour ids against real bank data without reading raw offsets by hand.
"""
import os

import unreal

path = os.environ.get(
    'KYA_ACTOR_BANK',
    r'G:\repos\Kya\assets\CDEURO\LEVEL\LEVEL_1\LEVEL.BNK')
class_text = os.environ.get('KYA_ACTOR_CLASS', '0x1c')
class_id = int(class_text, 0)

task = unreal.AssetImportTask()
task.filename = path
task.destination_path = '/Temp'
task.destination_name = 'KyaActorBehaviourInventory'
task.automated = True
task.save = False
task.replace_existing = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
objects = task.get_objects()
if not objects:
    raise RuntimeError('Could not import %s' % path)
actors = objects[0].get_editor_property('scene_environment').get_editor_property('actors')

# Editor property names for typed behaviour payloads. Unreal Python does not
# expose UPROPERTY names through dir(), so keep this list in sync with
# FKyaActorBehaviour. Unknown flags are reported as untyped.
TYPED_FLAGS = {
    'has_typed_bonus_behaviour': 'bonus',
    'has_typed_bridge_basic_behaviour': 'bridge-basic',
    'has_typed_bridge_wind_behaviour': 'bridge-wind',
    'has_typed_catch_by_trap_behaviour': 'catch-by-trap',
    'has_typed_cinematic_behaviour': 'cinematic',
    'has_typed_commander_behaviour': 'commander',
    'has_typed_companion_behaviour': 'companion',
    'has_typed_dca_controlled_behaviour': 'dca-controlled',
    'has_typed_dca_trajectory_behaviour': 'dca-trajectory',
    'has_typed_fog_behaviour': 'fog',
    'has_typed_fruit_behaviour': 'fruit',
    'has_typed_fx_behaviour': 'fx',
    'has_typed_gravity_bell_behaviour': 'gravity-bell',
    'has_typed_gravity_fall_behaviour': 'gravity-fall',
    'has_typed_hedgehog_behaviour': 'hedgehog',
    'has_typed_hero_unknown_behaviour': 'hero-unknown',
    'has_typed_inventory_behaviour': 'inventory',
    'has_typed_micken_behaviour': 'micken',
    'has_typed_nativ_behaviour': 'nativ',
    'has_typed_platform_behaviour': 'platform',
    'has_typed_projectile_behaviour': 'projectile',
    'has_typed_shoot_fire_behaviour': 'shoot-fire',
    'has_typed_shoot_wave_behaviour': 'shoot-wave',
    'has_typed_switch_behaviour': 'switch',
    'has_typed_trap_behaviour': 'trap',
    'has_typed_weapon_behaviour': 'weapon',
    'has_typed_wolfen_behaviour': 'wolfen',
}

seen = {}
examples = {}
matching_actors = 0
for actor in actors:
    if actor.get_editor_property('class_id') != class_id:
        continue
    matching_actors += 1
    name = actor.get_editor_property('name')
    for behaviour in actor.get_editor_property('behaviours'):
        bid = behaviour.get_editor_property('id')
        size = len(behaviour.get_editor_property('data'))
        typed_flags = []
        for flag, label in TYPED_FLAGS.items():
            if behaviour.get_editor_property(flag):
                typed_flags.append(label)
        key = (bid, size, tuple(sorted(typed_flags)))
        seen[key] = seen.get(key, 0) + 1
        examples.setdefault(key, name)

for key in sorted(seen):
    bid, size, typed = key
    unreal.log('KYA_ACTOR_BEHAVIOUR class=0x%02x id=%d size=%d typed=%s count=%d example=%s' %
               (class_id, bid, size, ','.join(typed) or 'none', seen[key], examples[key]))
unreal.log('KYA_ACTOR_BEHAVIOUR_TOTAL class=0x%02x actors=%d shapes=%d' %
           (class_id, matching_actors, len(seen)))
