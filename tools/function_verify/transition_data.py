"""Compiler-backed layouts and original COFF storage for transition globals."""
import json
import subprocess
from compiled import ROOT, coff_symbols, constant
from compiled_extended import sha
from coff_data import extract


PROBES = {
    'Actor': ('src/b-witch/Actor.cpp', [
        ('base_config', 'CActor::gStateCfg_ACT', '?gStateCfg_ACT@CActor@@')]),
    'ActorAutonomous': ('src/b-witch/ActorAutonomous.cpp', [
        ('autonomous_config', 'CActorAutonomous::gStateCfg_AUT', '?gStateCfg_AUT@CActorAutonomous@@')]),
    'ActorFighter': ('src/b-witch/ActorFighter.cpp', [
        ('fighter_config', 'CActorFighter::_gStateCfg_FIG', '?_gStateCfg_FIG@CActorFighter@@'),
        ('bone_ids', 'gBoneIds_004343a0', '?gBoneIds_004343a0@@'),
        ('fighter_vector_c0', 'edF32VECTOR4_004257c0', '?edF32VECTOR4_004257c0@@'),
        ('fighter_vector_d0', 'edF32VECTOR4_004257d0', '?edF32VECTOR4_004257d0@@')]),
    'MathOps': ('src/b-witch/MathOps.cpp', [
        ('unit_matrix', 'gF32Matrix4Unit', '?gF32Matrix4Unit@@'),
        ('zero_vector', 'gF32Vector4Zero', '?gF32Vector4Zero@@')]),
    'TimeController': ('src/b-witch/TimeController.cpp', [('timer', 'gTimer', '?gTimer@@')]),
    'PointerConv': ('src/port/pointer_conv.cpp', [
        ('persistent', 'PointerConv::gPersistentRegistry', '?gPersistentRegistry@'),
        ('transient', 'PointerConv::gTransientRegistry', '?gTransientRegistry@')]),
    'Animation': ('src/b-witch/Animation.cpp', [
        ('stage', 'TheAnimStage', '?TheAnimStage@@'),
        ('snapshot', 'debugAnimMacroSnapshot', '?debugAnimMacroSnapshot@'),
        ('bank', 'pDebugAnimMacroBank', '?pDebugAnimMacroBank@'),
        ('history', 'debugAnimTrackHistory', '?debugAnimTrackHistory@'),
        ('lookup_count', 'debugAnimTrackLookupCount', '?debugAnimTrackLookupCount@')]),
    'AnmManager': ('src/b-witch/AnmManager.cpp', [
        ('callbacks', 'CAnimationManager::_gLayersCallbacks', '?_gLayersCallbacks@')]),
    'Scene': ('src/b-witch/scene.cpp', [('scene', 'CScene::ptable', '?ptable@CScene@@')]),
}


def prepare(build, output, ninja, objects):
    """Compile disposable same-TU sizeof/offsetof probes; never edit game code."""
    output.mkdir(parents=True, exist_ok=True)
    specifications, evidence, layouts = {}, {}, {}
    for source, (relative, probes) in PROBES.items():
        target = 'CMakeFiles/Kya.dir/' + relative + '.obj'
        subprocess.run([ninja, '-C', str(build), target], check=True)
        original = (build / target).read_bytes()
        objects[source] = original
        command = subprocess.check_output([ninja, '-C', str(build), '-t', 'commands', target], text=True).splitlines()[-1]
        probe_source, probe_object = output / (source + '-layout.cpp'), output / (source + '-layout.obj')
        expressions = {'fv_' + key + '_size': 'sizeof(' + expression + ')' for key, expression, _ in probes}
        if source == 'Actor':
            expressions.update({
                'fv_actor_size': 'sizeof(CActor)',
                **{'fv_actor_' + key: '__builtin_offsetof(CActor, ' + member + ')'
                   for key, member in [('components', 'aComponents'), ('behaviour', 'curBehaviourId'),
                                       ('state', 'actorState'), ('animation', 'currentAnimType'),
                                       ('previous_state', 'prevActorState'), ('previous_animation', 'prevAnimType'),
                                       ('air_time', 'timeInAir'), ('idle_timer', 'idleTimer'),
                                       ('idle_loops', 'numIdleLoops'), ('animation_controller', 'pAnimationController'),
                                       ('macro_table', 'pMacroAnimTable')]},
                'fv_list_entries': '__builtin_offsetof(BehaviourList<1>, aComponents)',
                'fv_entry_size': 'sizeof(BehaviourEntry)',
                'fv_entry_id': '__builtin_offsetof(BehaviourEntry, id)',
                'fv_entry_handle': '__builtin_offsetof(BehaviourEntry, pBehaviour)',
                'fv_macro_table_size': 'sizeof(CActor::MacroAnimTable)',
                'fv_macro_count': '__builtin_offsetof(CActor::MacroAnimTable, nbEntries)',
                'fv_macro_entries': '__builtin_offsetof(CActor::MacroAnimTable, aEntries)',
                'fv_macro_entry_size': 'sizeof(CActor::MacroAnimEntry)',
                'fv_macro_entry_animation': '__builtin_offsetof(CActor::MacroAnimEntry, animType)',
                'fv_macro_entry_id': '__builtin_offsetof(CActor::MacroAnimEntry, macroAnimId)',
            })
        if source == 'ActorFighter':
            expressions.update({
                'fv_config_size': 'sizeof(StateConfig)',
                'fv_config_animation': '__builtin_offsetof(StateConfig, animId)',
                'fv_config_flags': '__builtin_offsetof(StateConfig, flags_0x4)',
                'fv_fighter_size': 'sizeof(CActorFighter)',
                'fv_behaviour_size': 'sizeof(CBehaviourFighter)',
                'fv_behaviour_owner': '__builtin_offsetof(CBehaviourFighter, pOwner)',
                **{'fv_fighter_' + key: '__builtin_offsetof(CActorFighter, ' + member + ')'
                   for key, member in [('fight_flags', 'fightFlags'), ('flags_36c', 'field_0x36c'),
                                       ('flags_44c', 'field_0x44cuint'), ('turn_speed', 'field_0x47c'),
                                       ('vector_4e0', 'field_0x4e0'), ('speed', 'dynamic.speed'),
                                       ('excluded_count', 'actorsExcludeTable.nbEntries'),
                                       ('timer_value', 'field_0x6bc'), ('adversary', 'pAdversary')]},
                'fv_fighter_stand_animation': '__builtin_offsetof(CActorFighter, standAnim)',
                'fv_fighter_weight': '__builtin_offsetof(CActorFighter, dynamic.weightA)',
                'fv_fighter_animation_blend': '__builtin_offsetof(CActorFighter, field_0x4fc)',
            })
        if source == 'PointerConv':
            expressions.update({
                'fv_slot_size': 'sizeof(PointerConv::PointerSlot)',
                'fv_slot_pointer': '__builtin_offsetof(PointerConv::PointerSlot, pValue)',
                'fv_slot_refcount': '__builtin_offsetof(PointerConv::PointerSlot, refCount)',
                'fv_slot_generation': '__builtin_offsetof(PointerConv::PointerSlot, generation)',
                'fv_registry_slots': '__builtin_offsetof(PointerConv::PointerRegistry, slots)',
                'fv_vector_size': 'sizeof(PointerConv::gPersistentRegistry.slots)',
            })
        if source == 'Animation':
            # Flatten nested playback fields so the proof uses actual x64
            # compiler offsets, including every inactive descriptor member.
            layer_members = ['blendOp', 'blendWeight', 'animPlayState', 'field_0xc',
                             'field_0xb8', 'field_0xbc', 'pFunction_0xc0', 'pActor',
                             'pAnimManagerKeyData', 'field_0xcc', 'playRate', 'field_0xd4']
            for desc in ('currentAnimDesc', 'nextAnimDesc'):
                layer_members += [desc + '.' + member for member in
                                  ('animType', 'pHdrA', 'flags', 'maxKey_0xc', 'pHdrB',
                                   'origAnimType', 'morphDuration', 'animMode')]
                layer_members += [desc + '.state.' + member for member in
                                  ('flags', 'currentAnimDataFlags', 'field_0x8', 'time_0xc',
                                   'time_0x10', 'keyStartTime_0x14', 'keyDuration_0x18',
                                   'pAnimKeyTableEntry', 'pKeyDataArray', 'pFunction',
                                   'pActor', 'animationType', 'playingDuration')]
            expressions.update({'fv_playback_' + member.replace('.', '_'):
                                '__builtin_offsetof(edAnmLayer, ' + member + ')'
                                for member in layer_members})
            expressions.update({
                'fv_state_desc_size': 'sizeof(edAnmStateDesc)',
                **{'fv_state_desc_' + key: '__builtin_offsetof(edAnmStateDesc, ' + member + ')'
                   for key, member in [('field', 'field_0x0'), ('animation', 'animType'),
                                       ('header_a', 'pHdrA'), ('flags', 'flags'),
                                       ('max_key', 'maxKey_0x10'), ('header_b', 'pHdrB'),
                                       ('original_animation', 'origAnimType')]},
                'fv_state_parser_size': 'sizeof(edAnmStateParser)',
                'fv_state_parser_data': '__builtin_offsetof(edAnmStateParser, pData)',
                'fv_controller_size': 'sizeof(CAnimation)',
                'fv_controller_count': '__builtin_offsetof(CAnimation, count_0x2c)',
                'fv_controller_current_animation': '__builtin_offsetof(CAnimation, currentAnimType)',
                'fv_controller_track_data': '__builtin_offsetof(CAnimation, aTrackData)',
                **{'fv_layer_' + key: '__builtin_offsetof(edAnmLayer, ' + member + ')'
                   for key, member in [('blend_op', 'blendOp'), ('play_state', 'animPlayState'),
                                       ('callback', 'pFunction_0xc0'), ('next_animation', 'nextAnimDesc.animType'),
                                       ('current_animation', 'currentAnimDesc.animType'),
                                       ('play_rate', 'playRate'), ('field_d4', 'field_0xd4')]},
                'fv_animator_size': 'sizeof(edAnmBinMetaAnimator)',
                'fv_animator_layers': '__builtin_offsetof(edAnmBinMetaAnimator, aAnimData)',
                'fv_animator_layer_count': '__builtin_offsetof(edAnmBinMetaAnimator, layerCount)',
                'fv_animator_bank': '__builtin_offsetof(edAnmBinMetaAnimator, pAnimKeyEntryData)',
                'fv_layer_size': 'sizeof(edAnmLayer)',
                'fv_layer_actor': '__builtin_offsetof(edAnmLayer, pActor)',
                'fv_track_manager_size': 'sizeof(CTrackManager)',
                'fv_track_count': '__builtin_offsetof(CTrackManager, trackCount)',
                'fv_scene_track_manager': '__builtin_offsetof(ManagerContainer, g_TrackManager_004516b4)',
                'fv_history_entry_size': 'sizeof(DebugAnimTrackLookup)',
                **{'fv_history_' + key: '__builtin_offsetof(DebugAnimTrackLookup, ' + field + ')'
                   for key, field in [('animator', 'pAnimator'), ('actor', 'pActor'), ('layer', 'layerIndex'),
                                      ('animation', 'animType'), ('offset', 'offset'), ('track', 'trackId')]},
            })
        text = '#include "' + (ROOT / relative).as_posix() + '"\nextern "C" {\n'
        text += ''.join('extern const unsigned int ' + key + ' = ' + value + ';\n' for key, value in expressions.items())
        probe_source.write_text(text + '}\n')
        probe_command = command.replace((ROOT / relative).as_posix(), probe_source.as_posix())
        if probe_command == command:
            raise ValueError('Cannot locate source in layout compiler command')
        probe_command = probe_command.replace(target.replace('/', '\\') + '.d', (output / (source + '.d')).as_posix())
        probe_command = probe_command.replace(target, probe_object.as_posix())
        subprocess.run(probe_command, cwd=build, check=True, stdout=subprocess.DEVNULL)
        layout_bytes = probe_object.read_bytes()
        layout = {key.removeprefix('fv_'): constant(layout_bytes, key) for key in expressions}
        layouts[source] = layout
        symbols, _ = coff_symbols(original)
        for key, _, prefix in probes:
            names = [s[0] for s in symbols if s[0].startswith(prefix) and s[2] > 0 and s[3] == 0]
            if len(names) != 1:
                raise ValueError('Missing/ambiguous transition global: ' + prefix)
            symbol = names[0]
            size = layout[key + '_size']
            item = extract(original, symbol, size)
            specifications[symbol] = (source, size)
            evidence[key] = {k: v for k, v in item.items() if k != 'bytes'}
            evidence[key].update(source=source, initial_bytes_hex=item['bytes'].hex(),
                                 runtime_obligation='Establish initialized runtime memory; COFF zeros are not constructor postconditions')
        layouts[source]['provenance'] = dict(command=probe_command, source_sha256=sha((ROOT / relative).read_bytes()),
                                             object_sha256=sha(original), probe_source_sha256=sha(probe_source.read_bytes()),
                                             probe_object_sha256=sha(layout_bytes))
    document = dict(scope='Layout/storage evidence only; no runtime invariant or equivalence claimed',
                    layouts=layouts, globals=evidence)
    (output / 'memory-layout.json').write_text(json.dumps(document, indent=2) + '\n')
    return specifications, document
