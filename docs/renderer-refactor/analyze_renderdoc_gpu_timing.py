from __future__ import print_function

import csv
import os
import re
from collections import defaultdict

import renderdoc as rd


CAPTURE = r"C:\Users\icey1\Documents\renderdoc\AirPostBaseLine.rdc"
OUT_DIR = r"G:\repos\Kya\docs\renderer-refactor"
CSV_PATH = os.path.join(OUT_DIR, "AirPostBaseLine_gpu_durations.csv")
MD_PATH = os.path.join(OUT_DIR, "AirPostBaseLine_gpu_timing_summary.md")
SAMPLE_COUNT = 10
TARGET_EVENT_IDS = set([53, 40, 2308, 69, 1581, 2260, 1138, 5777])
GROUP_LIMIT = 12


def enum_name(value):
    try:
        return str(value).split(".")[-1]
    except Exception:
        return str(value)


def has_flag(flags, flag):
    try:
        return bool(flags & flag)
    except Exception:
        return False


def action_name(controller, action):
    try:
        return action.GetName(controller.GetStructuredFile())
    except Exception:
        try:
            return action.customName
        except Exception:
            return "event {}".format(action.eventId)


def counter_value_to_float(value):
    for attr in ("d", "f", "u64", "u32", "i64", "i32"):
        if hasattr(value, attr):
            try:
                return float(getattr(value, attr))
            except Exception:
                pass
    try:
        return float(value)
    except Exception:
        return 0.0


def safe_get(obj, attr, default=None):
    try:
        return getattr(obj, attr)
    except Exception:
        return default


def first_attr(obj, attrs, default=None):
    for attr in attrs:
        value = safe_get(obj, attr, None)
        if value is not None:
            return value
    return default


def resource_key(resource_id):
    if resource_id is None:
        return ""
    try:
        if resource_id == rd.ResourceId.Null():
            return ""
    except Exception:
        pass
    try:
        return str(resource_id)
    except Exception:
        return ""


def resource_label(resource_id, resource_names):
    key = resource_key(resource_id)
    if not key:
        return ""
    name = resource_names.get(key, "")
    if name:
        return "{} ({})".format(name, key)
    return key


def describe_scalar(value, resource_names=None):
    if value is None:
        return ""
    if resource_names is not None:
        key = resource_key(value)
        if key:
            label = resource_label(value, resource_names)
            if label:
                return label
    if isinstance(value, bool):
        return "on" if value else "off"
    if isinstance(value, (int, float)):
        return str(value)
    return enum_name(value)


def describe_attrs(obj, attrs, resource_names=None):
    if obj is None:
        return ""
    parts = []
    for label, names in attrs:
        value = first_attr(obj, names)
        if value is None:
            continue
        parts.append("{}={}".format(label, describe_scalar(value, resource_names)))
    return " ".join(parts)


def color_mask_name(mask):
    try:
        mask_int = int(mask)
    except Exception:
        text = enum_name(mask)
        return text if text else ""
    if mask_int == 0:
        return "none"
    channels = []
    for bit, channel in ((1, "R"), (2, "G"), (4, "B"), (8, "A")):
        if mask_int & bit:
            channels.append(channel)
    return "".join(channels) if channels else str(mask_int)


def build_resource_name_map(controller):
    names = {}
    for getter in ("GetResources", "GetTextures", "GetBuffers"):
        try:
            resources = getattr(controller, getter)()
        except Exception:
            continue
        for resource in resources:
            rid = first_attr(resource, ("resourceId", "id", "resource"))
            key = resource_key(rid)
            if not key:
                continue
            name = first_attr(resource, ("name", "customName"))
            if name:
                names[key] = str(name)
    return names


def native_asset_marker(marker_stack):
    for marker in marker_stack:
        if marker.startswith("Native ") and marker != "Native Render":
            return marker
    return "(none)"


def material_marker(marker_stack):
    for marker in marker_stack:
        match = re.match(r"Material\s+(.+)$", marker)
        if match:
            return match.group(1)
    asset = native_asset_marker(marker_stack)
    match = re.search(r"\(m:\s*([^\s\)]+)", asset)
    if match:
        return match.group(1)
    return "(none)"


def leaf_marker(marker_stack):
    if marker_stack:
        return marker_stack[-1]
    return "(none)"


def index_bucket(num_indices):
    if num_indices <= 0:
        return "0"
    if num_indices <= 3:
        return "1-3"
    if num_indices <= 12:
        return "4-12"
    if num_indices <= 48:
        return "13-48"
    if num_indices <= 256:
        return "49-256"
    if num_indices <= 1024:
        return "257-1024"
    if num_indices <= 4096:
        return "1025-4096"
    return "4097+"


def describe_sampler(sampler):
    if sampler is None:
        return ""
    fields = []
    for label, names in (
        ("filter", ("filter",)),
        ("addr", ("addressU",)),
        ("mip", ("mipBias",)),
        ("cmp", ("compareFunction",)),
    ):
        value = first_attr(sampler, names)
        value_name = enum_name(value) if value is not None else ""
        if value_name and not value_name.startswith("<Swig Object"):
            fields.append("{}={}".format(label, value_name))
    return ",".join(fields)


def descriptor_resource_id(bound):
    descriptor = safe_get(bound, "descriptor")
    if descriptor is not None:
        rid = first_attr(descriptor, ("resource", "view", "secondary"))
        if resource_key(rid):
            return rid
    return first_attr(bound, ("resourceId", "resource", "id"))


def descriptor_type(bound):
    descriptor = safe_get(bound, "descriptor")
    if descriptor is not None:
        value = first_attr(descriptor, ("type", "textureType"))
        if value is not None:
            return enum_name(value)
    return ""


def collect_bound_resources(pipe, resource_names):
    labels = []
    stages = (rd.ShaderStage.Pixel, rd.ShaderStage.Vertex)
    methods = ("GetReadOnlyResources", "GetReadWriteResources")
    for stage in stages:
        for method in methods:
            try:
                resources = getattr(pipe, method)(stage)
            except Exception:
                resources = []
            for binding_index, binding in enumerate(resources):
                bind_array = safe_get(binding, "binds", None)
                if bind_array is None:
                    iterable = [binding]
                else:
                    try:
                        iterable = list(bind_array)
                    except Exception:
                        iterable = [binding]
                for array_index, bound in enumerate(iterable):
                    rid = descriptor_resource_id(bound)
                    label = resource_label(rid, resource_names)
                    if label:
                        sampler = describe_sampler(safe_get(bound, "sampler"))
                        desc_type = descriptor_type(bound)
                        suffix = ""
                        if desc_type or sampler:
                            suffix = " ({})".format(", ".join([x for x in (desc_type, sampler) if x]))
                        labels.append("{}:{}[{}:{}]={}{}".format(
                            enum_name(stage), method.replace("Get", ""), binding_index, array_index, label, suffix))
    return "; ".join(labels)


def first_bound_texture(bound_resources):
    if not bound_resources:
        return "(none)"
    first = bound_resources.split(";", 1)[0].strip()
    if "=" in first:
        return first.split("=", 1)[1]
    return first


def describe_blend_state(pipe):
    try:
        blends = list(pipe.GetColorBlends())
    except Exception:
        blends = []
    if not blends:
        return ""
    parts = []
    for i, blend in enumerate(blends):
        enabled = first_attr(blend, ("enabled", "blendEnable", "blendEnabled"))
        source = first_attr(blend, ("source", "sourceColor", "srcColor", "sourceRGB", "srcBlend"))
        destination = first_attr(blend, ("destination", "destinationColor", "dstColor", "destinationRGB", "dstBlend"))
        operation = first_attr(blend, ("operation", "colorOperation", "blendOp"))
        mask = first_attr(blend, ("writeMask", "writeMaskBits", "channelWriteMask"))
        fields = []
        if enabled is not None:
            fields.append("blend={}".format(describe_scalar(enabled)))
        if source is not None:
            fields.append("src={}".format(enum_name(source)))
        if destination is not None:
            fields.append("dst={}".format(enum_name(destination)))
        if operation is not None:
            fields.append("op={}".format(enum_name(operation)))
        if mask is not None:
            fields.append("mask={}".format(color_mask_name(mask)))
        parts.append("rt{}:{}".format(i, ",".join(fields) if fields else enum_name(blend)))
    return " | ".join(parts)


def describe_color_write_mask(pipe):
    try:
        blends = list(pipe.GetColorBlends())
    except Exception:
        blends = []
    masks = []
    for i, blend in enumerate(blends):
        mask = first_attr(blend, ("writeMask", "writeMaskBits", "channelWriteMask"))
        if mask is not None:
            masks.append("rt{}={}".format(i, color_mask_name(mask)))
    return " ".join(masks)


def describe_depth_state(pipe):
    try:
        depth = pipe.GetDepthTestState()
    except Exception:
        depth = None
    return describe_attrs(depth, [
        ("test", ("enabled", "depthEnable", "depthTestEnable")),
        ("write", ("writes", "depthWrites", "depthWriteEnable")),
        ("func", ("function", "compareFunction", "depthFunction")),
        ("bounds", ("bounds", "depthBoundsEnable")),
    ])


def describe_targets(pipe, resource_names):
    color_labels = []
    try:
        targets = list(pipe.GetOutputTargets())
    except Exception:
        targets = []
    for i, target in enumerate(targets):
        rid = first_attr(target, ("resourceId", "resource", "imageResourceId", "image"))
        label = resource_label(rid, resource_names)
        if not label:
            label = enum_name(target)
        if label and not label.startswith("<Swig Object"):
            color_labels.append("rt{}={}".format(i, label))

    depth_label = ""
    try:
        depth = pipe.GetDepthTarget()
    except Exception:
        depth = None
    if depth is not None:
        rid = first_attr(depth, ("resourceId", "resource", "imageResourceId", "image"))
        depth_label = resource_label(rid, resource_names) or enum_name(depth)
    return "; ".join(color_labels), depth_label


def collect_pipeline_state(controller, rows):
    resource_names = build_resource_name_map(controller)
    for row in rows:
        row["pipeline"] = ""
        row["blend_state"] = ""
        row["depth_state"] = ""
        row["color_write_mask"] = ""
        row["render_targets"] = ""
        row["depth_target"] = ""
        row["bound_resources"] = ""
        row["texture_key"] = "(none)"

    state_rows = [
        row for row in rows
        if row["is_draw"] and row["top_marker"] == "Native Render" and
        (row["duration_us"] > 0.0 or row["event_id"] in TARGET_EVENT_IDS)
    ]
    for row in state_rows:
        try:
            controller.SetFrameEvent(row["event_id"], True)
            pipe = controller.GetPipelineState()
        except Exception as exc:
            row["pipeline"] = "state unavailable: {}".format(exc)
            continue

        try:
            row["pipeline"] = resource_label(pipe.GetGraphicsPipelineObject(), resource_names)
        except Exception:
            row["pipeline"] = ""
        row["blend_state"] = describe_blend_state(pipe)
        row["depth_state"] = describe_depth_state(pipe)
        row["color_write_mask"] = describe_color_write_mask(pipe)
        row["render_targets"], row["depth_target"] = describe_targets(pipe, resource_names)
        row["bound_resources"] = collect_bound_resources(pipe, resource_names)
        row["texture_key"] = first_bound_texture(row["bound_resources"])


def flatten_actions(controller, roots):
    rows = []

    def walk(action, marker_stack, top_marker):
        name = action_name(controller, action)
        is_marker = has_flag(action.flags, rd.ActionFlags.PushMarker) or has_flag(action.flags, rd.ActionFlags.SetMarker)
        is_draw = has_flag(action.flags, rd.ActionFlags.Drawcall)

        row = {
            "action": action,
            "event_id": int(action.eventId),
            "name": name,
            "marker_stack": list(marker_stack),
            "top_marker": top_marker or "(unmarked)",
            "is_draw": is_draw,
            "is_marker": is_marker,
            "num_indices": int(getattr(action, "numIndices", 0)),
            "num_instances": int(getattr(action, "numInstances", 0)),
            "duration_us": 0.0,
        }
        row["asset_marker"] = native_asset_marker(row["marker_stack"])
        row["material"] = material_marker(row["marker_stack"])
        row["leaf_marker"] = leaf_marker(row["marker_stack"])
        row["index_bucket"] = index_bucket(row["num_indices"])
        rows.append(row)

        child_stack = marker_stack
        child_top = top_marker
        if is_marker:
            child_stack = marker_stack + [name]
            child_top = top_marker or name
        elif not marker_stack and getattr(action, "children", None):
            child_top = top_marker or name

        for child in action.children:
            walk(child, child_stack, child_top)

    for root in roots:
        walk(root, [], None)
    return rows


def grouped_summary(rows, key_name, limit=GROUP_LIMIT):
    groups = {}
    for row in rows:
        key = row.get(key_name) or "(none)"
        if key not in groups:
            groups[key] = {"duration_us": 0.0, "timed_draws": 0, "total_draws": 0, "indices": 0}
        group = groups[key]
        group["total_draws"] += 1
        group["indices"] += row["num_indices"]
        if row["duration_us"] > 0.0:
            group["duration_us"] += row["duration_us"]
            group["timed_draws"] += 1
    return sorted(groups.items(), key=lambda item: item[1]["duration_us"], reverse=True)[:limit]


def group_for_event(rows, event_row, key_name):
    key = event_row.get(key_name) or "(none)"
    total = 0.0
    timed = 0
    draws = 0
    for row in rows:
        if (row.get(key_name) or "(none)") != key:
            continue
        draws += 1
        if row["duration_us"] > 0.0:
            total += row["duration_us"]
            timed += 1
    return key, total, timed, draws


def recommendation(native_timed_draws):
    if not native_timed_draws:
        return "No timed native draws were available, so no GPU fix can be selected from this capture."

    total = sum(row["duration_us"] for row in native_timed_draws)
    blend_groups = grouped_summary(native_timed_draws, "blend_state", 4)
    depth_groups = grouped_summary(native_timed_draws, "depth_state", 4)
    texture_groups = grouped_summary(native_timed_draws, "texture_key", 4)
    small_total = sum(row["duration_us"] for row in native_timed_draws if row["num_indices"] <= 48)

    best_blend = blend_groups[0] if blend_groups else ("", {"duration_us": 0.0})
    best_depth = depth_groups[0] if depth_groups else ("", {"duration_us": 0.0})
    best_texture = texture_groups[0] if texture_groups else ("", {"duration_us": 0.0})

    if best_blend[1]["duration_us"] >= total * 0.35 or best_depth[1]["duration_us"] >= total * 0.35:
        return (
            "Target the repeated blend/depth path first. The largest blend group is `{}` at {:.3f} us, "
            "and the largest depth group is `{}` at {:.3f} us out of {:.3f} us native timed draw cost. "
            "Validate alpha/depth correctness before changing ordering-sensitive behavior."
        ).format(best_blend[0], best_blend[1]["duration_us"], best_depth[0], best_depth[1]["duration_us"], total)
    if best_texture[1]["duration_us"] >= total * 0.25:
        return (
            "Target texture/sample-state behavior first. Texture/resource group `{}` accounts for {:.3f} us "
            "out of {:.3f} us native timed draw cost."
        ).format(best_texture[0], best_texture[1]["duration_us"], total)
    if small_total >= total * 0.25:
        return (
            "GPU-visible merging is worth prototyping only for adjacent native draws with identical state. "
            "`<= 48` index draws account for {:.3f} us out of {:.3f} us, so compare before/after GPU counters, "
            "not draw count alone."
        ).format(small_total, total)
    return (
        "Do not start with merging from this capture alone. The timed cost is spread across asset/material/state "
        "groups; inspect the top grouped rows and target the dominant render state or texture path first."
    )


def write_group_table(f, title, rows):
    f.write("## {}\n\n".format(title))
    f.write("| Group | Timed draws | Total draws | Indices | Duration us |\n")
    f.write("|---|---:|---:|---:|---:|\n")
    for key, group in rows:
        f.write("| {} | {} | {} | {} | {:.3f} |\n".format(
            str(key).replace("|", "\\|"),
            group["timed_draws"],
            group["total_draws"],
            group["indices"],
            group["duration_us"],
        ))
    if not rows:
        f.write("| (none) | 0 | 0 | 0 | 0.000 |\n")
    f.write("\n")


def write_outputs(counter_desc, counters, event_duration, event_samples, rows, unavailable_reason):
    draw_rows = [r for r in rows if r["is_draw"]]
    for row in rows:
        samples = event_samples.get(row["event_id"], [])
        row["duration_us"] = event_duration.get(row["event_id"], 0.0)
        row["min_duration_us"] = min(samples) if samples else row["duration_us"]
        row["max_duration_us"] = max(samples) if samples else row["duration_us"]

    pass_totals = defaultdict(float)
    pass_total_draw_counts = defaultdict(int)
    pass_draw_counts = defaultdict(int)
    pass_event_counts = defaultdict(int)
    for row in rows:
        if row["is_draw"]:
            pass_total_draw_counts[row["top_marker"]] += 1
        dur = row["duration_us"]
        if dur:
            pass_totals[row["top_marker"]] += dur
            pass_event_counts[row["top_marker"]] += 1
            if row["is_draw"]:
                pass_draw_counts[row["top_marker"]] += 1

    slowest_draws = sorted(draw_rows, key=lambda r: r["duration_us"], reverse=True)[:20]
    small_draws = [r for r in draw_rows if 0 < r["num_indices"] <= 48]
    small_draw_duration = sum(r["duration_us"] for r in small_draws)
    native_draws = [r for r in draw_rows if r["top_marker"] == "Native Render"]
    native_timed_draws = [r for r in native_draws if r["duration_us"] > 0.0]

    with open(CSV_PATH, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            "event_id", "duration_us", "is_draw", "action_name", "marker_stack", "top_marker",
            "asset_marker", "material", "leaf_marker", "num_indices", "index_bucket", "num_instances",
            "pipeline", "blend_state", "depth_state", "color_write_mask", "render_targets", "depth_target",
            "texture_key", "bound_resources", "min_duration_us", "max_duration_us",
        ])
        for row in rows:
            if row["duration_us"] or row["is_draw"]:
                writer.writerow([
                    row["event_id"], "{:.6f}".format(row["duration_us"]), int(row["is_draw"]), row["name"],
                    " > ".join(row["marker_stack"]), row["top_marker"], row["asset_marker"], row["material"],
                    row["leaf_marker"], row["num_indices"], row["index_bucket"], row["num_instances"],
                    row.get("pipeline", ""), row.get("blend_state", ""), row.get("depth_state", ""),
                    row.get("color_write_mask", ""), row.get("render_targets", ""), row.get("depth_target", ""),
                    row.get("texture_key", ""), row.get("bound_resources", ""),
                    "{:.6f}".format(row["min_duration_us"]), "{:.6f}".format(row["max_duration_us"]),
                ])

    with open(MD_PATH, "w") as f:
        f.write("# AirPostBaseLine GPU Timing Summary\n\n")
        f.write("- Capture: `{}`\n".format(CAPTURE))
        f.write("- Counter requested: `rd.GPUCounter.EventGPUDuration`\n")
        f.write("- EventGPUDuration available: `{}`\n".format("yes" if not unavailable_reason else "no"))
        f.write("- Counter samples averaged: `{}`\n".format(SAMPLE_COUNT if not unavailable_reason else 0))
        if counter_desc:
            f.write("- Counter unit: `{}`\n".format(enum_name(counter_desc.unit)))
        f.write("- Enumerated counter count: `{}`\n".format(len(counters)))
        f.write("- Flattened action count: `{}`\n".format(len(rows)))
        f.write("- Draw action count: `{}`\n".format(len(draw_rows)))
        if unavailable_reason:
            f.write("- Limitation: {}\n".format(unavailable_reason))
        f.write("\n")

        f.write("## Total GPU Duration By Top-Level Pass/Marker\n\n")
        f.write("| Pass/marker | Timed events | Timed draws | Total draws | Duration us |\n")
        f.write("|---|---:|---:|---:|---:|\n")
        for name, total in sorted(pass_totals.items(), key=lambda item: item[1], reverse=True):
            f.write("| {} | {} | {} | {} | {:.3f} |\n".format(
                name.replace("|", "\\|"), pass_event_counts[name], pass_draw_counts[name],
                pass_total_draw_counts[name], total,
            ))
        if not pass_totals:
            f.write("| (none) | 0 | 0 | 0 | 0.000 |\n")
        f.write("\n")

        f.write("## Slowest 20 Draw Events\n\n")
        f.write("| Rank | Event ID | Action | Asset | Material | Texture/resource | Indices | Blend | Depth | Duration us |\n")
        f.write("|---:|---:|---|---|---|---|---:|---|---|---:|\n")
        for i, row in enumerate(slowest_draws, 1):
            f.write("| {} | {} | {} | {} | {} | {} | {} | {} | {} | {:.3f} |\n".format(
                i, row["event_id"], row["name"].replace("|", "\\|"),
                row["asset_marker"].replace("|", "\\|"), row["material"].replace("|", "\\|"),
                row.get("texture_key", "(none)").replace("|", "\\|"), row["num_indices"],
                row.get("blend_state", "").replace("|", "\\|"),
                row.get("depth_state", "").replace("|", "\\|"), row["duration_us"],
            ))
        f.write("\n")

        write_group_table(f, "Native Timed Draws By Marker Asset", grouped_summary(native_timed_draws, "asset_marker"))
        write_group_table(f, "Native Timed Draws By Material", grouped_summary(native_timed_draws, "material"))
        write_group_table(f, "Native Timed Draws By Texture/Resource", grouped_summary(native_timed_draws, "texture_key"))
        write_group_table(f, "Native Timed Draws By Index Count Bucket", grouped_summary(native_timed_draws, "index_bucket"))
        write_group_table(f, "Native Timed Draws By Blend State", grouped_summary(native_timed_draws, "blend_state"))
        write_group_table(f, "Native Timed Draws By Depth State", grouped_summary(native_timed_draws, "depth_state"))

        event_map = dict((row["event_id"], row) for row in rows)
        f.write("## Target Event Attribution\n\n")
        f.write("| Event ID | Duration us | Asset | Material | Leaf | Indices | Bucket | Texture/resource | Pipeline | Blend | Depth | Targets | Group note |\n")
        f.write("|---:|---:|---|---|---|---:|---|---|---|---|---|---|---|\n")
        for event_id in sorted(TARGET_EVENT_IDS):
            row = event_map.get(event_id)
            if not row:
                continue
            asset_key, asset_total, asset_timed, asset_draws = group_for_event(native_timed_draws, row, "asset_marker")
            material_key, material_total, material_timed, material_draws = group_for_event(native_timed_draws, row, "material")
            note = "asset `{}` {:.3f} us/{} timed/{} draws; material `{}` {:.3f} us/{} timed/{} draws".format(
                asset_key, asset_total, asset_timed, asset_draws, material_key, material_total, material_timed, material_draws,
            )
            targets = "{} / depth {}".format(row.get("render_targets", ""), row.get("depth_target", ""))
            f.write("| {} | {:.3f} | {} | {} | {} | {} | {} | {} | {} | {} | {} | {} | {} |\n".format(
                event_id, row["duration_us"], row["asset_marker"].replace("|", "\\|"),
                row["material"].replace("|", "\\|"), row["leaf_marker"].replace("|", "\\|"),
                row["num_indices"], row["index_bucket"], row.get("texture_key", "(none)").replace("|", "\\|"),
                row.get("pipeline", "").replace("|", "\\|"), row.get("blend_state", "").replace("|", "\\|"),
                row.get("depth_state", "").replace("|", "\\|"), targets.replace("|", "\\|"), note.replace("|", "\\|"),
            ))
        f.write("\n")

        f.write("## Targeted GPU Fix Candidate\n\n")
        f.write("{}\n\n".format(recommendation(native_timed_draws)))

        f.write("## Small Draws\n\n")
        f.write("- Draws with `0 < numIndices <= 48`: `{}`\n".format(len(small_draws)))
        f.write("- Total duration: `{:.3f} us`\n".format(small_draw_duration))

    return {
        "csv": CSV_PATH,
        "md": MD_PATH,
        "counter_count": len(counters),
        "rows": rows,
        "draw_rows": draw_rows,
        "native_timed_draws": native_timed_draws,
        "pass_totals": pass_totals,
        "pass_total_draw_counts": pass_total_draw_counts,
        "slowest_draws": slowest_draws,
        "small_draw_count": len(small_draws),
        "small_draw_duration": small_draw_duration,
        "unavailable_reason": unavailable_reason,
    }


def main():
    if not os.path.exists(CAPTURE):
        raise RuntimeError("Capture not found: {}".format(CAPTURE))

    rd.InitialiseReplay(rd.GlobalEnvironment(), [])
    cap = None
    controller = None
    try:
        cap = rd.OpenCaptureFile()
        result = cap.OpenFile(CAPTURE, "", None)
        if result != rd.ResultCode.Succeeded:
            raise RuntimeError("OpenFile failed: {}".format(result))
        if not cap.LocalReplaySupport():
            raise RuntimeError("Capture cannot be replayed locally")

        result, controller = cap.OpenCapture(rd.ReplayOptions(), None)
        if result != rd.ResultCode.Succeeded:
            raise RuntimeError("OpenCapture failed: {}".format(result))

        roots = controller.GetRootActions()
        rows = flatten_actions(controller, roots)

        counters = list(controller.EnumerateCounters())
        target = rd.GPUCounter.EventGPUDuration
        available = target in counters
        counter_desc = None
        event_duration = defaultdict(float)
        event_samples = defaultdict(list)
        unavailable_reason = ""

        if available:
            counter_desc = controller.DescribeCounter(target)
            unit_name = enum_name(counter_desc.unit)
            multiplier = 1000000.0 if unit_name.lower().endswith("seconds") or unit_name.lower() == "seconds" else 1.0
            for _ in range(SAMPLE_COUNT):
                sample_duration = defaultdict(float)
                results = controller.FetchCounters([target])
                for result in results:
                    if result.counter == target:
                        sample_duration[int(result.eventId)] += counter_value_to_float(result.value) * multiplier
                for event_id, duration in sample_duration.items():
                    event_samples[event_id].append(duration)
            for event_id, samples in event_samples.items():
                event_duration[event_id] = sum(samples) / float(len(samples))
        else:
            unavailable_reason = "rd.GPUCounter.EventGPUDuration was not returned by controller.EnumerateCounters()."

        for row in rows:
            samples = event_samples.get(row["event_id"], [])
            row["duration_us"] = event_duration.get(row["event_id"], 0.0)
            row["min_duration_us"] = min(samples) if samples else row["duration_us"]
            row["max_duration_us"] = max(samples) if samples else row["duration_us"]
        collect_pipeline_state(controller, rows)
        summary = write_outputs(counter_desc, counters, event_duration, event_samples, rows, unavailable_reason)

        print("capture={}".format(CAPTURE))
        print("root_actions={}".format(len(roots)))
        print("flattened_actions={}".format(len(rows)))
        print("draw_actions={}".format(len(summary["draw_rows"])))
        print("native_timed_draws={}".format(len(summary["native_timed_draws"])))
        print("counter_count={}".format(summary["counter_count"]))
        print("EventGPUDuration_available={}".format(available))
        print("counter_samples={}".format(SAMPLE_COUNT if available else 0))
        if counter_desc:
            print("EventGPUDuration_name={}".format(counter_desc.name))
            print("EventGPUDuration_unit={}".format(enum_name(counter_desc.unit)))
        print("small_draw_count={}".format(summary["small_draw_count"]))
        print("small_draw_duration_us={:.3f}".format(summary["small_draw_duration"]))
        print("markdown={}".format(summary["md"]))
        print("csv={}".format(summary["csv"]))
        print("top_passes:")
        for name, total in sorted(summary["pass_totals"].items(), key=lambda item: item[1], reverse=True)[:10]:
            print("  {:.3f} us  {}".format(total, name))
        print("slowest_draws:")
        for row in summary["slowest_draws"][:20]:
            print("  event={} duration_us={:.3f} indices={} texture={} name={} stack={}".format(
                row["event_id"], row["duration_us"], row["num_indices"], row.get("texture_key", "(none)"),
                row["name"], " > ".join(row["marker_stack"]) or "(none)",
            ))
    finally:
        if controller is not None:
            controller.Shutdown()
        if cap is not None:
            cap.Shutdown()
        rd.ShutdownReplay()


if __name__ == "__main__":
    main()





