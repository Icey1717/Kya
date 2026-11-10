"""Capture one level-placed actor model in isolation for comparison with the game.

Run from the Unreal Python console, or launch the editor with
``-ExecCmds="py <this file>"``. Environment overrides:

* ``KYA_CAPTURE_BANK``  asset path (default ``/Game/LEVEL``)
* ``KYA_CAPTURE_MAP``   map to load (default ``/Game/Empty``)
* ``KYA_ACTOR_NAME``    actor name to isolate (default ``GRUNTNATIV``)
* ``KYA_ACTOR_SHOTS``   number of orbit views (default ``4``)

The preview hides level geometry and every other actor, so the capture matches
the game's mesh viewer rather than a full scene. Screenshots are written to the
editor's Saved/Screenshots/WindowsEditor directory.
"""
import math
import os
import unreal

level = os.environ.get('KYA_CAPTURE_MAP', '/Game/Empty')
bank_path = os.environ.get('KYA_CAPTURE_BANK', '/Game/LEVEL')
actor_name = os.environ.get('KYA_ACTOR_NAME', 'GRUNTNATIV')
shot_count = int(os.environ.get('KYA_ACTOR_SHOTS', '4'))

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(level)
world = unreal.EditorLevelLibrary.get_editor_world()
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
preview_class = unreal.load_class(None, '/Script/KyaLevel.KyaLevelPreview')
for old_preview in [actor for actor in actor_subsystem.get_all_level_actors()
                    if actor.get_class() == preview_class]:
    actor_subsystem.destroy_actor(old_preview)

preview = unreal.EditorLevelLibrary.spawn_actor_from_class(preview_class, unreal.Vector(), unreal.Rotator())
# Set every option before the bank so the first construction already uses them.
preview.set_editor_property('show_waypoints', False)
preview.set_editor_property('show_all_actor_meshes', False)
preview.set_editor_property('show_background_meshes', False)
preview.set_editor_property('show_cluster_meshes', False)
preview.set_editor_property('show_actor_mesh', True)
preview.set_editor_property('apply_actor_lighting', os.environ.get('KYA_ACTOR_LIGHTING', '0') == '1')
preview.set_editor_property('log_sector_diagnostics', True)

bank_asset = unreal.load_asset(bank_path)
if bank_asset is None:
    raise RuntimeError('Missing bank asset %s' % bank_path)
actors = bank_asset.get_editor_property('scene_environment').get_editor_property('actors')
if not actors:
    # A bank saved before the actor fields were typed silently lacks actors.
    task = unreal.AssetImportTask()
    task.filename = os.path.join(os.environ.get(
        'KYA_CAPTURE_SECTOR_DIRECTORY', 'G:/repos/Kya/assets/CDEURO/LEVEL/LEVEL_1'), 'LEVEL.BNK')
    task.destination_path = '/Game/__KyaCapture'
    task.destination_name = 'LEVEL'
    task.automated = True
    task.save = False
    task.replace_existing = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    objects = task.get_objects()
    if not objects:
        raise RuntimeError('Could not reimport %s' % task.filename)
    bank_asset = objects[0]
    actors = bank_asset.get_editor_property('scene_environment').get_editor_property('actors')
    unreal.log('KYA_ACTOR_CAPTURE_REIMPORTED %s actors=%d' % (bank_asset.get_name(), len(actors)))
if not actors:
    raise RuntimeError('Bank asset %s has no typed actors' % bank_path)
index = next((i for i, actor in enumerate(actors)
              if actor.get_editor_property('name') == actor_name), None)
if index is None:
    raise RuntimeError('Actor %s not found in %s' % (actor_name, bank_path))
actor = actors[index]
position = actor.get_editor_property('position')
rotation = actor.get_editor_property('rotation')
scale = actor.get_editor_property('scale')
unreal.log('KYA_ACTOR_MESH actor=%s index=%d class=%d mesh=%d texture=%d position=%s rotation=%s scale=%s'
           % (actor_name, index, actor.get_editor_property('class_id'),
              actor.get_editor_property('mesh_index'), actor.get_editor_property('texture_index'),
              position, rotation, scale))
unreal.log('KYA_ACTOR_LIGHTING flags=%s value=%s' % (
    actor.get_editor_property('lighting_flags'), actor.get_editor_property('lighting_value')))
for model in actor.get_editor_property('models'):
    unreal.log('KYA_ACTOR_MODEL %s -> %s' % (
        model.get_editor_property('mesh_name'), model.get_editor_property('texture_name')))

preview.set_editor_property('bank', bank_asset)
preview.set_editor_property('preview_actor_index', index)
environment = bank_asset.get_editor_property('scene_environment')
unreal.log('KYA_SCENE_TEXTURES default=%s alternate=%s' % (
    environment.get_editor_property('default_texture_index'),
    environment.get_editor_property('alternate_texture_index')))


def to_unreal(vector, scale_factor=1.0):
    return unreal.Vector(vector.x * scale_factor, vector.z * scale_factor, vector.y * scale_factor)


target = to_unreal(position, 100.0) + unreal.Vector(0.0, 0.0, 110.0)
# Use an off-screen scene capture so the result does not depend on editor focus,
# which makes HighResShot silently capture nothing when the editor is backgrounded.
distance = 480.0
height = 40.0
width, height_px = 1024, 768
render_target = unreal.RenderingLibrary.create_render_target2d(
    world, width, height_px, unreal.TextureRenderTargetFormat.RTF_RGBA8,
    unreal.LinearColor(0.0, 0.0, 0.0, 1.0))
capture = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.SceneCapture2D, unreal.Vector(), unreal.Rotator())
capture_component = capture.get_component_by_class(unreal.SceneCaptureComponent2D)
capture_component.set_editor_property('texture_target', render_target)
capture_component.set_editor_property('capture_source', unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
capture_component.set_editor_property('capture_every_frame', False)
capture_component.set_editor_property('capture_on_movement', False)
capture_component.set_editor_property('fov_angle', 40.0)
capture_component.set_editor_property('projection_type', unreal.CameraProjectionMode.PERSPECTIVE)
unreal.log('KYA_ACTOR_CAPTURE target=%s shots=%d' % (target, shot_count))

project_dir = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir())
screenshot_dir = os.path.join(project_dir, 'Screenshots', 'WindowsEditor')
ticks = [0]
shot_state = {'shot': -1, 'captured': 0}
strip_scan = os.environ.get('KYA_ACTOR_STRIP_SCAN', '0') == '1'
strip_count = int(os.environ.get('KYA_ACTOR_STRIP_COUNT', '20'))


def set_shot(shot):
    yaw = math.radians(shot * (360.0 / shot_count))
    location = unreal.Vector(target.x + math.sin(yaw) * distance,
                             target.y - math.cos(yaw) * distance,
                             target.z + height)
    rotation = unreal.MathLibrary.find_look_at_rotation(location, target)
    capture.set_actor_location(location, False, False)
    capture.set_actor_rotation(rotation, False)
    capture_component.capture_scene()


def export_shot(name):
    saved = unreal.RenderingLibrary.export_render_target(
        world, render_target, screenshot_dir, name)
    unreal.log('KYA_ACTOR_SCREENSHOT %s saved=%s' % (name, saved))


def capture_tick(delta):
    ticks[0] += 1
    if strip_scan:
        if ticks[0] == 60:
            set_shot(1)
        elif ticks[0] > 60:
            step = (ticks[0] - 60) // 30
            if step < strip_count:
                phase = (ticks[0] - 60) % 30
                if phase == 0:
                    preview.set_editor_property('preview_actor_strip_index', step)
                elif phase == 10:
                    capture_component.capture_scene()
                elif phase == 20:
                    export_shot('actor_strip_%02d' % step)
            else:
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.SystemLibrary.quit_editor()
        return
    if ticks[0] >= 90 and (ticks[0] - 90) % 30 == 0:
        shot = (ticks[0] - 90) // 30
        if shot % 2 == 0 and shot // 2 < shot_count:
            set_shot(shot // 2)
            shot_state['shot'] = shot // 2
            shot_state['captured'] = ticks[0]
        elif shot % 2 == 1 and shot_state['shot'] >= 0:
            export_shot('actor_mesh_%d' % shot_state['shot'])
            shot_state['shot'] = -1
    if ticks[0] >= 90 + shot_count * 60 + 60:
        unreal.unregister_slate_post_tick_callback(handle)
        unreal.SystemLibrary.quit_editor()


handle = unreal.register_slate_post_tick_callback(capture_tick)
