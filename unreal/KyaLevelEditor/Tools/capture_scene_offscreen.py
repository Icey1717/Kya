"""Capture the KyaLevelPreview through an off-screen SceneCapture2D.

Uses the same environment variables as capture_matched_view.py. The output is
written to the editor screenshot directory; final-colour exposure differs from
the editor viewport, so use it for geometry/placement comparisons.
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
sector_directory = os.environ.get('KYA_CAPTURE_SECTOR_DIRECTORY',
                                  'G:/repos/Kya/assets/CDEURO/LEVEL/LEVEL_1')

if not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(level):
    raise RuntimeError('Could not load capture map %s; refusing to capture the default scene' % level)
world = unreal.EditorLevelLibrary.get_editor_world()
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
unreal.log('KYA_CAPTURE_MAP %s actors=%s' % (world.get_path_name(),
    [(actor.get_name(), actor.get_class().get_name()) for actor in actor_subsystem.get_all_level_actors()]))
preview_class = unreal.load_class(None, '/Script/KyaLevel.KyaLevelPreview')
for old_preview in [actor for actor in actor_subsystem.get_all_level_actors()
                    if actor.get_class() == preview_class]:
    actor_subsystem.destroy_actor(old_preview)
preview = unreal.EditorLevelLibrary.spawn_actor_from_class(preview_class, unreal.Vector(), unreal.Rotator())
preview.set_editor_property('show_waypoints', False)
preview.set_editor_property('preview_base_sector', int(metadata.get('base_sector', -1)))
preview.set_editor_property('show_all_actor_meshes', os.environ.get('KYA_CAPTURE_ACTORS', '1') == '1')
preview.set_editor_property('show_background_meshes', os.environ.get('KYA_CAPTURE_BACKGROUND', '1') == '1')
preview.set_editor_property('show_cluster_meshes', os.environ.get('KYA_CAPTURE_CLUSTERS', '1') == '1')
preview.set_editor_property('show_sprites', os.environ.get('KYA_CAPTURE_SPRITES', '1') == '1')
preview.set_editor_property('apply_actor_lighting', os.environ.get('KYA_CAPTURE_LIGHTING', '1') == '1')
preview.set_editor_property('log_sector_diagnostics', os.environ.get('KYA_CAPTURE_DIAGNOSTICS', '0') == '1')

bank_asset = unreal.load_asset(bank_path)
if not bank_asset:
    task = unreal.AssetImportTask()
    task.filename = os.path.join(sector_directory, 'LEVEL.BNK')
    task.destination_path = '/Game/__KyaCapture'
    task.destination_name = 'LEVEL'
    task.automated = True
    task.save = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    objects = task.get_objects()
    if not objects:
        raise RuntimeError('Could not import %s' % task.filename)
    bank_asset = objects[0]
preview.set_editor_property('bank', bank_asset)

sector_override = os.environ.get('KYA_CAPTURE_SECTORS')
if sector_override:
    sector_ids = [int(value) for value in sector_override.split(',') if value]
else:
    sector_ids = sorted({entry['id'] for entry in metadata.get('loaded_sectors', [])})
sector_assets = []
for sector_id in sector_ids:
    asset = unreal.load_asset('/Game/SECT%d' % sector_id)
    if not asset:
        task = unreal.AssetImportTask()
        task.filename = os.path.join(sector_directory, 'SECT%d.BNK' % sector_id)
        task.destination_path = '/Game/__KyaCapture'
        task.destination_name = 'SECT%d' % sector_id
        task.automated = True
        task.save = False
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        objects = task.get_objects()
        asset = objects[0] if objects else None
    if asset:
        sector_assets.append(asset)
preview.set_editor_property('sector_banks', sector_assets)
unreal.log('KYA_OFFSCREEN_SECTORS %s' % sector_ids)


def to_unreal(vector, scale=1.0):
    return unreal.Vector(vector[0] * scale, vector[2] * scale, vector[1] * scale)


location = to_unreal(camera['position'], 100.0)
forward = to_unreal(camera['forward'])
up = to_unreal(camera['up'])
rotation = unreal.MathLibrary.make_rot_from_xz(forward, up)
preview.set_background_camera_position(location)
preview.set_editor_property('sprite_camera_forward', unreal.Vector(*camera['forward']))
preview.set_editor_property('sprite_camera_up', unreal.Vector(*camera['up']))

width = int(os.environ.get('KYA_CAPTURE_WIDTH', metadata.get('width', 1024)))
height = int(os.environ.get('KYA_CAPTURE_HEIGHT', metadata.get('height', 768)))
preview.set_preview_camera(location, forward, up, math.degrees(camera['horizontal_fov_radians']), width, height)
if os.environ.get('KYA_CAPTURE_RAYS'):
    import runpy
    runpy.run_path(os.path.join(os.path.dirname(__file__), 'inspect_preview_rays.py'))['inspect'](
        preview, camera, width, height, os.environ['KYA_CAPTURE_RAYS'])
render_target = unreal.RenderingLibrary.create_render_target2d(
    world, width, height, unreal.TextureRenderTargetFormat.RTF_RGBA8_SRGB,
    unreal.LinearColor(0.0, 0.0, 0.0, 1.0))
capture = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.SceneCapture2D, location, rotation)
component = capture.get_component_by_class(unreal.SceneCaptureComponent2D)
component.set_editor_property('texture_target', render_target)
component.set_editor_property('capture_source', unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
component.set_editor_property('capture_every_frame', False)
component.set_editor_property('capture_on_movement', False)
component.set_editor_property('always_persist_rendering_state', True)
component.set_editor_property('fov_angle', math.degrees(camera['horizontal_fov_radians']))
component.set_editor_property('max_view_distance_override', float(os.environ.get('KYA_CAPTURE_MAX_VIEW_DISTANCE', '-1')))
unreal.log('KYA_OFFSCREEN_DISTANCE max_view_distance=%s mesh_max_draw_distance=%s' % (
    component.get_editor_property('max_view_distance_override'),
    preview.get_editor_property('mesh').get_editor_property('ld_max_draw_distance')))
try:
    post_process = component.get_editor_property('post_process_settings')
    post_process.set_editor_property('override_auto_exposure_method', True)
    post_process.set_editor_property('auto_exposure_method', unreal.AutoExposureMethod.AEM_MANUAL)
    post_process.set_editor_property('override_auto_exposure_bias', True)
    post_process.set_editor_property('auto_exposure_bias', 0.0)
    post_process.set_editor_property('override_auto_exposure_apply_physical_camera_exposure', True)
    post_process.set_editor_property('auto_exposure_apply_physical_camera_exposure', False)
    component.set_editor_property('post_process_settings', post_process)
except Exception as error:
    unreal.log_error('KYA_OFFSCREEN_EXPOSURE_FAILED %s' % error)

# Native colour comparison needs the raw framebuffer, so tone mapping is off by
# default. Set KYA_CAPTURE_DISABLE_TONEMAPPER=0 to keep Unreal's tone curve.
if os.environ.get('KYA_CAPTURE_DISABLE_TONEMAPPER', '1') == '1':
    component.set_editor_property('show_flag_settings', [
        unreal.EngineShowFlagsSetting(show_flag_name=name, enabled=False)
        for name in ('Tonemapper', 'EyeAdaptation', 'ColorGrading', 'Bloom')])

project_dir = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir())
screenshot_dir = os.path.join(project_dir, 'Screenshots', 'WindowsEditor')
ticks = [0]
verify_omissions = os.environ.get('KYA_CAPTURE_VERIFY_OMISSIONS') == '1'
saved_flares = []
saved_additive = []

def export_capture(suffix=''):
    name = os.environ.get('KYA_CAPTURE_NAME', 'scene_offscreen')
    if name.lower().endswith('.png'):
        name = name[:-4]
    unreal.RenderingLibrary.export_render_target(world, render_target, screenshot_dir, name + suffix + '.png')
    unreal.log('KYA_OFFSCREEN_SCREENSHOT %s size=%dx%d' % (name + suffix, width, height))

def capture_tick(delta):
    ticks[0] += 1
    if ticks[0] == 120:
        component.capture_scene()
    if ticks[0] == 150:
        export_capture()
    if verify_omissions:
        if ticks[0] == 160:
            post_process = preview.get_components_by_class(unreal.PostProcessComponent)[0]
            settings = post_process.get_editor_property('settings')
            blendables = settings.get_editor_property('weighted_blendables').get_editor_property('array')
            for blendable in blendables:
                material = blendable.get_editor_property('object')
                saved_flares.append((material, material.get_vector_parameter_value('FlareRect')))
                material.set_vector_parameter_value('FlareRect', unreal.LinearColor(0, 0, 0, 0))
            unreal.log('KYA_VERIFY_FLARES count=%d rects=%s' % (len(saved_flares), [value for _, value in saved_flares]))
        if ticks[0] == 190:
            component.capture_scene()
        if ticks[0] == 220:
            export_capture('_no_flares')
            for material, value in saved_flares:
                material.set_vector_parameter_value('FlareRect', value)
            mesh = preview.get_editor_property('mesh')
            for index in range(mesh.get_num_sections()):
                material = mesh.get_material(index)
                if isinstance(material, unreal.MaterialInstanceDynamic):
                    parent = material.get_editor_property('parent')
                    if parent.get_editor_property('blend_mode') == unreal.BlendMode.BLEND_ADDITIVE:
                        saved_additive.append((material, material.get_scalar_parameter_value('BlendFactor')))
                        material.set_scalar_parameter_value('BlendFactor', 0.0)
            unreal.log('KYA_VERIFY_ADDITIVE count=%d factors=%s' % (len(saved_additive), [value for _, value in saved_additive]))
        if ticks[0] == 250:
            component.capture_scene()
        if ticks[0] == 280:
            export_capture('_no_additive')
            for material, value in saved_additive:
                material.set_scalar_parameter_value('BlendFactor', value)
            component.set_editor_property('max_view_distance_override', 100000000.0)
        if ticks[0] == 310:
            component.capture_scene()
        if ticks[0] == 340:
            export_capture('_far_distance')
            # An unobstructed marker is a positive control for the flare shader;
            # the normal source can be fully occluded from the matched camera.
            for material, value in saved_flares:
                marker = material.get_vector_parameter_value('FlareMarker')
                marker.b = 0.0
                material.set_vector_parameter_value('FlareMarker', marker)
        if ticks[0] == 370:
            component.capture_scene()
        if ticks[0] == 400:
            export_capture('_flare_unoccluded')
            for material, value in saved_flares:
                marker = material.get_vector_parameter_value('FlareMarker')
                marker.b = 1.0e12
                material.set_vector_parameter_value('FlareMarker', marker)
        if ticks[0] == 430:
            component.capture_scene()
        if ticks[0] == 460:
            export_capture('_flare_occluded')
    if ticks[0] == (480 if verify_omissions else 180):
        unreal.unregister_slate_post_tick_callback(handle)
        unreal.SystemLibrary.quit_editor()

handle = unreal.register_slate_post_tick_callback(capture_tick)
