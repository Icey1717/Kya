"""Identify procedural-mesh triangles under normalized capture pixels, without collision."""
import json
import math
import unreal


def inspect(preview, camera, width, height, output):
    def xyz(v):
        return (v.x, v.y, v.z)

    def sub(a, b):
        return tuple(x - y for x, y in zip(a, b))

    def dot(a, b):
        return sum(x * y for x, y in zip(a, b))

    def cross(a, b):
        return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])

    f = camera['forward']
    f = (f[0], f[2], f[1])
    up = camera['up']
    up = (up[0], up[2], up[1])
    right = cross(up, f)
    position = camera['position']
    position = (position[0]*100, position[2]*100, position[1]*100)
    tan_x = math.tan(camera['horizontal_fov_radians'] / 2)
    tan_y = tan_x * height / width
    mesh = preview.get_editor_property('mesh')
    triangles = []
    sections = []
    for section in range(mesh.get_num_sections()):
        vertices, indices, normals, uvs, tangents = unreal.ProceduralMeshLibrary.get_section_from_procedural_mesh(mesh, section)
        points = [xyz(v) for v in vertices]
        material = mesh.get_material(section)
        mode = str(material.get_editor_property('parent').get_editor_property('blend_mode')) if isinstance(material, unreal.MaterialInstanceDynamic) else 'untextured'
        sections.append(dict(section=section, triangles=len(indices)//3, blend=mode))
        for index in range(0, len(indices), 3):
            a, b, c = (points[indices[index + corner]] for corner in range(3))
            triangles.append((section, index//3, a, sub(b, a), sub(c, a)))
    rays = []
    for y in (0.02, 0.06, 0.10, 0.14, 0.18):
        for x in (0.66, 0.70, 0.74, 0.78):
            direction = tuple(f[i] + right[i]*(2*x-1)*tan_x + up[i]*(1-2*y)*tan_y for i in range(3))
            hits = []
            for section, index, a, edge1, edge2 in triangles:
                p = cross(direction, edge2)
                determinant = dot(edge1, p)
                if abs(determinant) < 1e-10:
                    continue
                t = sub(position, a)
                u = dot(t, p) / determinant
                if u < 0 or u > 1:
                    continue
                q = cross(t, edge1)
                v = dot(direction, q) / determinant
                if v < 0 or u + v > 1:
                    continue
                distance = dot(edge2, q) / determinant
                if distance > 0:
                    hits.append(dict(section=section, triangle=index, camera_depth_units=distance/100,
                                     determinant=determinant, barycentric=[1-u-v, u, v]))
            hits.sort(key=lambda hit: hit['camera_depth_units'])
            rays.append(dict(uv=[x, y], hits=hits[:12]))
    with open(output, 'w', encoding='utf-8') as stream:
        json.dump(dict(sections=sections, rays=rays), stream, indent=2)
    unreal.log('KYA_PREVIEW_RAYS %s triangles=%d rays=%d' % (output, len(triangles), len(rays)))
