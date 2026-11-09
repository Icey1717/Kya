"""Run in Unreal's Python console with KYA_CAMERA_METADATA set to socket JSON.

Loads the test project's /Game/Empty map and spawns an unsaved preview. The
asset paths and sector list can be overridden through KYA_CAPTURE_* variables.
The current editor viewport pilots a temporary camera constrained to the game
capture's aspect ratio. The script does not save the map or imported assets.
"""
import json
import math
import os
import unreal

metadata_path = os.environ['KYA_CAMERA_METADATA']
with open(metadata_path, encoding='utf-8') as metadata_file:
    metadata = json.load(metadata_file)
camera = metadata['camera']
level = os.environ.get('KYA_CAPTURE_MAP', '/Game/Empty')
bank_path = os.environ.get('KYA_CAPTURE_BANK', '/Game/LEVEL')
sector_override = os.environ.get('KYA_CAPTURE_SECTORS')
sector_directory = os.environ.get('KYA_CAPTURE_SECTOR_DIRECTORY',
                                  'G:/repos/Kya/assets/CDEURO/LEVEL/LEVEL_1')

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(level)
world = unreal.EditorLevelLibrary.get_editor_world()
preview_class = unreal.load_class(None, '/Script/KyaLevel.KyaLevelPreview')
preview = unreal.EditorLevelLibrary.spawn_actor_from_class(preview_class, unreal.Vector(), unreal.Rotator())
preview.set_editor_property('raw_color_modulation', os.environ.get('KYA_CAPTURE_RAW_COLOR', '0') == '1')
preview.set_editor_property('bank', unreal.load_asset(bank_path))
if sector_override is not None:
    sector_ids = [int(value) for value in sector_override.split(',') if value]
else:
    sector_ids = sorted({entry['id'] for entry in metadata.get('loaded_sectors', [])})
if not sector_ids:
    raise RuntimeError('Capture has no loaded sector IDs; set KYA_CAPTURE_SECTORS explicitly')
sector_assets = []
for sector_id in sector_ids:
    asset_path = '/Game/SECT%d' % sector_id
    asset = unreal.load_asset(asset_path)
    if not asset:
        task = unreal.AssetImportTask()
        task.filename = os.path.join(sector_directory, 'SECT%d.BNK' % sector_id)
        task.destination_path = '/Game/__KyaCapture'
        task.destination_name = 'SECT%d' % sector_id
        task.automated = True
        task.save = False
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        objects = task.get_objects()
        if not objects:
            raise RuntimeError('Could not import sector %d from %s' % (sector_id, task.filename))
        asset = objects[0]
    sector_assets.append(asset)
preview.set_editor_property('sector_banks', sector_assets)
unreal.log('KYA_CAPTURE_SECTORS %s' % sector_ids)
preview.set_editor_property('show_waypoints', False)
preview.set_editor_property('show_all_actor_meshes', os.environ.get('KYA_CAPTURE_ACTORS', '1') == '1')
preview.set_editor_property('log_sector_diagnostics', os.environ.get('KYA_CAPTURE_DIAGNOSTICS', '0') == '1')

def to_unreal(vector, scale=1.0):
    return unreal.Vector(vector[0] * scale, vector[2] * scale, vector[1] * scale)

location = to_unreal(camera['position'], 100.0)
forward = to_unreal(camera['forward'])
up = to_unreal(camera['up'])
rotation = unreal.MathLibrary.make_rot_from_xz(forward, up)
capture_camera = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.CameraActor, location, rotation)
camera_component = capture_camera.get_component_by_class(unreal.CameraComponent)
camera_component.set_editor_property('field_of_view', math.degrees(camera['horizontal_fov_radians']))
camera_component.set_editor_property('aspect_ratio', metadata['width'] / metadata['height'])
camera_component.set_editor_property('constrain_aspect_ratio', True)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).pilot_level_actor(capture_camera)
preview.set_background_camera_position(location)
unreal.EditorLevelLibrary.editor_invalidate_viewports()
unreal.log('KYA_MATCHED_CAMERA location=%s rotation=%s fov=%.3f aspect=%.6f source=%s' %
           (location, rotation, math.degrees(camera['horizontal_fov_radians']),
            metadata['width'] / metadata['height'], metadata_path))

ticks = [0]
def capture_tick(delta):
    ticks[0] += 1
    if ticks[0] == 120:
        unreal.SystemLibrary.execute_console_command(world, 'HighResShot 1')
    if ticks[0] == 240:
        unreal.unregister_slate_post_tick_callback(handle)
        unreal.SystemLibrary.quit_editor()

handle = unreal.register_slate_post_tick_callback(capture_tick)
