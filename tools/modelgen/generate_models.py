"""
Procedural low-poly model generator for River Raid 3D.

Run with:
    /Applications/Blender.app/Contents/MacOS/Blender --background \
        --python tools/modelgen/generate_models.py -- <output_dir> [--preview <dir>]

Everything is built directly in the game's coordinate convention:
    +X = starboard / right wing
    +Y = up
    +Z = forward (nose of a plane, bow of a ship)
and exported with forward_axis='Y', up_axis='Z', which is a verified identity
mapping for Blender's OBJ exporter (signs included).

Models are UV mapped into hand-picked solid regions of the game's existing
texture atlases, so the new geometry matches the art that is already there
without needing any new image files.
"""

import bpy
import math
import os
import sys

# ---------------------------------------------------------------------------
# Texture atlas regions
# ---------------------------------------------------------------------------
# Pixel rectangles are given as (x0, y0, x1, y1) with the origin at the TOP-LEFT
# of the image as viewed. 'flip' mirrors how the game loads the texture (the
# "flip" field in textures.impfile), which decides how a pixel row becomes a v
# coordinate. Regions are chosen well inside large uniform areas so that a small
# sampling error cannot bleed into a neighbouring island of the atlas.

PLANE_TEX = dict(size=1024, flip=True)
SHIP_TEX = dict(size=4096, flip=False)


def rect(x0, y0, x1, y1, tex):
    """Convert a pixel rectangle into a (u0, v0, u1, v1) UV rectangle."""
    n = float(tex["size"])
    u0, u1 = x0 / n, x1 / n
    if tex["flip"]:
        v0, v1 = 1.0 - y1 / n, 1.0 - y0 / n
    else:
        v0, v1 = y0 / n, y1 / n
    return (u0, v0, u1, v1)


# planetexture.png is a cut-out atlas: the background between its islands is
# TRANSPARENT, and those transparent texels carry black RGB. A UV rect that
# overlaps an island edge therefore paints black patches onto the model. Every
# rect below was verified fully opaque by scanning the image's alpha channel
# (tools/modelgen scan; see the project notes), and the narrower ones are
# deliberately sub-rects of verified regions, which are opaque by construction.
# All three greens are sub-rects of one verified-opaque island. An earlier
# attempt used a patch beside the black canopy ellipse: it passed the opacity
# check but straddled green and black texels, which painted black blotches
# across the model. Being opaque is not enough - the region must also be
# visually uniform.
GREEN_DARK = rect(100, 730, 430, 935, PLANE_TEX)   # big lower-left wing island
GREEN_MID = rect(270, 730, 430, 935, PLANE_TEX)    # right half of the same island
GREEN_LIGHT = rect(100, 730, 260, 935, PLANE_TEX)  # left half of the same island
GREY_METAL = rect(510, 610, 570, 670, PLANE_TEX)   # flat grey spinner disc
CANOPY_DARK = rect(90, 425, 150, 487, PLANE_TEX)   # opaque black canopy glass

# warship2.jpg: naval greys plus the below-waterline hull red.
HULL_RED = rect(2500, 300, 2650, 1800, SHIP_TEX)
DECK_GREY = rect(260, 1500, 900, 2300, SHIP_TEX)
SUPER_GREY = rect(1700, 1450, 2100, 1900, SHIP_TEX)
MACHINE_GREY = rect(2900, 250, 3800, 1000, SHIP_TEX)


# ---------------------------------------------------------------------------
# Geometry building blocks
# ---------------------------------------------------------------------------
class Builder:
    """Accumulates vertices and faces, remembering a UV rect for every face."""

    def __init__(self):
        self.verts = []
        self.faces = []
        self.rects = []

    def add(self, verts, faces, uv):
        off = len(self.verts)
        self.verts.extend(verts)
        for f in faces:
            self.faces.append(tuple(i + off for i in f))
            self.rects.append(uv)

    def add_mirrored(self, verts, faces, uv):
        """Adds a part and its mirror across X, flipping winding on the copy."""
        self.add(verts, faces, uv)
        mirrored = [(-v[0], v[1], v[2]) for v in verts]
        self.add(mirrored, [tuple(reversed(f)) for f in faces], uv)


def oct_ring(z, hw, hh, cy=0.0):
    """An octagonal cross-section: rounder than a box, still cheap."""
    k = 0.5
    pts = [
        (hw * k, cy + hh), (hw, cy + hh * k), (hw, cy - hh * k), (hw * k, cy - hh),
        (-hw * k, cy - hh), (-hw, cy - hh * k), (-hw, cy + hh * k), (-hw * k, cy + hh),
    ]
    return [(x, y, z) for (x, y) in pts]


def loft(b, sections, uv, cap_front=True, cap_back=True):
    """Skins a sequence of rings (each the same length) into a tube."""
    rings = [oct_ring(z, hw, hh, cy) for (z, hw, hh, cy) in sections]
    n = len(rings[0])
    verts = [v for ring in rings for v in ring]
    faces = []
    for r in range(len(rings) - 1):
        a, c = r * n, (r + 1) * n
        for i in range(n):
            j = (i + 1) % n
            faces.append((a + i, a + j, c + j, c + i))
    if cap_back:
        faces.append(tuple(range(n - 1, -1, -1)))
    if cap_front:
        base = (len(rings) - 1) * n
        faces.append(tuple(base + i for i in range(n)))
    b.add(verts, faces, uv)


def slab(b, x0, x1, le0, te0, le1, te1, y0, y1, t0, t1, uv, mirror=True):
    """
    A tapered aerofoil-ish slab, used for wings, stabilisers and fins.
    x0/x1   span start and end
    le/te   leading and trailing edge Z at each end
    y0/y1   vertical position at each end (dihedral)
    t0/t1   thickness at each end
    """
    verts = [
        (x0, y0 - t0, le0), (x0, y0 + t0, le0), (x0, y0 + t0, te0), (x0, y0 - t0, te0),
        (x1, y1 - t1, le1), (x1, y1 + t1, le1), (x1, y1 + t1, te1), (x1, y1 - t1, te1),
    ]
    faces = [
        (0, 3, 2, 1), (4, 5, 6, 7),
        (0, 1, 5, 4), (3, 7, 6, 2),
        (1, 2, 6, 5), (0, 4, 7, 3),
    ]
    if mirror:
        b.add_mirrored(verts, faces, uv)
    else:
        b.add(verts, faces, uv)


def box(b, cx, cy, cz, hx, hy, hz, uv, mirror=False):
    verts = [
        (cx - hx, cy - hy, cz - hz), (cx + hx, cy - hy, cz - hz),
        (cx + hx, cy + hy, cz - hz), (cx - hx, cy + hy, cz - hz),
        (cx - hx, cy - hy, cz + hz), (cx + hx, cy - hy, cz + hz),
        (cx + hx, cy + hy, cz + hz), (cx - hx, cy + hy, cz + hz),
    ]
    faces = [(0, 1, 2, 3), (4, 7, 6, 5), (0, 4, 5, 1), (1, 5, 6, 2), (2, 6, 7, 3), (3, 7, 4, 0)]
    if mirror:
        b.add_mirrored(verts, faces, uv)
    else:
        b.add(verts, faces, uv)


def cylinder(b, cx, cy, cz, radius, half_len, uv, axis="y", sides=8, taper=1.0):
    """Prism used for funnels, engine nacelles and gun barrels."""
    verts, faces = [], []
    for end in (0, 1):
        r = radius * (taper if end else 1.0)
        for i in range(sides):
            a = 2.0 * math.pi * i / sides
            u, v = r * math.cos(a), r * math.sin(a)
            d = -half_len if end == 0 else half_len
            if axis == "y":
                verts.append((cx + u, cy + d, cz + v))
            elif axis == "z":
                verts.append((cx + u, cy + v, cz + d))
            else:
                verts.append((cx + d, cy + u, cz + v))
    for i in range(sides):
        j = (i + 1) % sides
        faces.append((i, j, sides + j, sides + i))
    faces.append(tuple(range(sides - 1, -1, -1)))
    faces.append(tuple(sides + i for i in range(sides)))
    b.add(verts, faces, uv)


# ---------------------------------------------------------------------------
# Aircraft
# ---------------------------------------------------------------------------
def plane_fighter(b):
    """Single-engine fighter. Span ~30, length ~27."""
    loft(b, [
        (-13.5, 0.9, 0.9, 0.2), (-11.0, 1.5, 1.7, 0.3), (-6.0, 2.1, 2.3, 0.2),
        (0.0, 2.4, 2.5, 0.0), (5.0, 2.2, 2.2, 0.0), (9.5, 1.7, 1.7, 0.0),
        (12.0, 1.1, 1.1, 0.0), (13.5, 0.5, 0.5, 0.0),
    ], GREEN_DARK)
    # Main wing, slight dihedral and taper
    slab(b, 1.6, 15.0, 4.2, -3.2, 2.4, -1.4, -0.5, 0.6, 0.55, 0.25, GREEN_DARK)
    # Horizontal stabiliser
    slab(b, 0.8, 6.2, -10.2, -13.2, -10.6, -12.6, 0.5, 0.9, 0.3, 0.15, GREEN_MID)
    # Vertical fin
    slab(b, 0.0, 0.0, -9.6, -13.4, -10.4, -13.2, 1.2, 6.2, 0.35, 0.15, GREEN_MID, mirror=False)
    # Canopy
    loft(b, [(-1.0, 1.0, 0.7, 2.4), (1.6, 1.3, 0.9, 2.5), (4.4, 1.0, 0.6, 2.3)], CANOPY_DARK)
    # Spinner
    cylinder(b, 0.0, 0.0, 13.9, 1.0, 0.9, GREY_METAL, axis="z", taper=0.25)


def plane_bomber(b):
    """Twin-engine medium bomber. Span ~38, length ~33."""
    loft(b, [
        (-16.5, 1.0, 1.0, 0.2), (-13.0, 1.9, 2.0, 0.3), (-7.0, 2.7, 2.9, 0.2),
        (0.0, 3.0, 3.1, 0.0), (6.0, 2.8, 2.9, 0.0), (11.5, 2.2, 2.2, 0.0),
        (14.5, 1.4, 1.4, 0.0), (16.5, 0.7, 0.8, 0.0),
    ], GREEN_DARK)
    slab(b, 2.0, 19.0, 5.0, -4.6, 2.8, -1.8, 0.4, 1.4, 0.7, 0.3, GREEN_DARK)
    # Twin fins on a shared tailplane
    slab(b, 1.0, 8.0, -12.4, -16.2, -12.8, -15.6, 0.8, 1.1, 0.35, 0.2, GREEN_MID)
    slab(b, 7.0, 7.0, -12.6, -15.8, -13.2, -15.4, 1.1, 5.0, 0.3, 0.15, GREEN_MID)
    # Engine nacelles on the wings
    for sx in (-1.0, 1.0):
        loft(b, [
            (-4.0, 1.3, 1.3, 0.0), (0.0, 1.7, 1.7, 0.0), (4.2, 1.4, 1.4, 0.0), (5.6, 0.7, 0.7, 0.0),
        ], GREEN_MID)
        # Shift the nacelle we just added out onto the wing
        for i in range(len(b.verts) - 32, len(b.verts)):
            x, y, z = b.verts[i]
            b.verts[i] = (x + sx * 7.4, y + 0.9, z + 1.6)
        cylinder(b, sx * 7.4, 0.9, 8.0, 0.9, 0.8, GREY_METAL, axis="z", taper=0.25)
    loft(b, [(-2.0, 1.4, 0.9, 3.0), (2.0, 1.6, 1.1, 3.1), (6.0, 1.2, 0.7, 2.8)], CANOPY_DARK)


def plane_interceptor(b):
    """Swept-wing interceptor, slim and tall-tailed. Span ~27, length ~30."""
    loft(b, [
        (-15.0, 0.8, 0.8, 0.3), (-12.0, 1.4, 1.6, 0.3), (-6.0, 1.9, 2.1, 0.2),
        (0.0, 2.1, 2.3, 0.0), (6.0, 2.0, 2.0, 0.0), (11.0, 1.5, 1.5, 0.0),
        (14.0, 0.9, 0.9, 0.0), (15.0, 0.4, 0.4, 0.0),
    ], GREEN_MID)
    # Strongly swept wing: tip leading edge well behind the root
    slab(b, 1.4, 13.5, 3.0, -3.0, -3.4, -7.0, -0.3, 1.2, 0.5, 0.2, GREEN_MID)
    slab(b, 0.8, 5.0, -10.8, -14.0, -12.2, -14.2, 0.4, 0.9, 0.25, 0.12, GREEN_DARK)
    slab(b, 0.0, 0.0, -9.0, -14.4, -11.4, -14.2, 1.0, 7.0, 0.3, 0.12, GREEN_DARK, mirror=False)
    loft(b, [(0.0, 0.9, 0.6, 2.1), (2.6, 1.1, 0.8, 2.2), (5.4, 0.8, 0.5, 2.0)], CANOPY_DARK)
    cylinder(b, 0.0, 0.0, 15.2, 0.85, 0.8, GREY_METAL, axis="z", taper=0.25)


def plane_attacker(b):
    """Heavy gull-winged attacker. Span ~33, length ~29."""
    loft(b, [
        (-14.5, 1.1, 1.1, 0.2), (-11.5, 2.0, 2.1, 0.3), (-6.0, 2.6, 2.8, 0.2),
        (0.0, 2.8, 3.0, 0.0), (5.5, 2.6, 2.6, 0.0), (10.5, 2.0, 2.0, 0.0),
        (13.0, 1.3, 1.3, 0.0), (14.5, 0.6, 0.6, 0.0),
    ], GREEN_LIGHT)
    # Gull wing: inner section angled up, outer section flat
    slab(b, 1.8, 6.0, 4.6, -3.6, 4.0, -2.6, -0.6, 1.6, 0.65, 0.45, GREEN_LIGHT)
    slab(b, 6.0, 16.5, 4.0, -2.6, 2.6, -1.2, 1.6, 1.2, 0.45, 0.22, GREEN_LIGHT)
    slab(b, 0.9, 6.6, -10.8, -14.0, -11.4, -13.4, 0.6, 1.0, 0.32, 0.16, GREEN_DARK)
    slab(b, 0.0, 0.0, -10.0, -14.2, -11.0, -14.0, 1.2, 5.6, 0.35, 0.15, GREEN_DARK, mirror=False)
    loft(b, [(-1.4, 1.2, 0.8, 2.8), (1.8, 1.4, 1.0, 2.9), (5.0, 1.0, 0.6, 2.6)], CANOPY_DARK)
    cylinder(b, 0.0, 0.0, 14.9, 1.1, 0.9, GREY_METAL, axis="z", taper=0.25)


# ---------------------------------------------------------------------------
# Ships  (bow points +Z so the aim-at-player rotation reads correctly)
# ---------------------------------------------------------------------------
def ship_hull(b, length, beam, keel, waterline, deck):
    """Lower (red) and upper (grey) hull, plus a deck cap."""
    half = length / 2.0
    stations = [
        (-half, 0.30), (-half * 0.82, 0.72), (-half * 0.45, 0.97),
        (0.0, 1.0), (half * 0.45, 0.93), (half * 0.78, 0.62), (half, 0.06),
    ]
    lower = [(z, beam * f, (waterline - keel) / 2.0, (waterline + keel) / 2.0) for z, f in stations]
    loft(b, lower, HULL_RED)
    upper = [(z, beam * f, (deck - waterline) / 2.0, (deck + waterline) / 2.0) for z, f in stations]
    loft(b, upper, DECK_GREY)


def ship_turret(b, cz, deck, scale, facing=1.0):
    box(b, 0.0, deck + 0.7 * scale, cz, 1.5 * scale, 0.7 * scale, 2.0 * scale, SUPER_GREY)
    for dx in (-0.55 * scale, 0.55 * scale):
        cylinder(b, dx, deck + 1.2 * scale, cz + facing * 3.4 * scale,
                 0.28 * scale, 1.8 * scale, MACHINE_GREY, axis="z", sides=6)


def ship_destroyer(b):
    """Slim single-funnel destroyer. Length ~70 model units."""
    length, beam, keel, waterline, deck = 70.0, 4.6, -7.0, -1.0, 2.0
    ship_hull(b, length, beam, keel, waterline, deck)
    # Bridge stack
    box(b, 0.0, deck + 2.2, 6.0, 3.0, 2.2, 6.5, SUPER_GREY)
    box(b, 0.0, deck + 5.0, 7.5, 2.2, 1.4, 4.0, SUPER_GREY)
    box(b, 0.0, deck + 7.0, 8.5, 1.4, 1.2, 2.4, MACHINE_GREY)
    # Mast
    box(b, 0.0, deck + 11.0, 7.0, 0.22, 3.2, 0.22, MACHINE_GREY)
    # Funnel, raked slightly aft
    cylinder(b, 0.0, deck + 3.4, -3.0, 1.5, 3.4, MACHINE_GREY, axis="y", taper=0.8)
    # Aft deckhouse
    box(b, 0.0, deck + 1.6, -13.0, 2.6, 1.6, 6.0, SUPER_GREY)
    ship_turret(b, 22.0, deck, 1.0, facing=1.0)
    ship_turret(b, -24.0, deck, 1.0, facing=-1.0)


def ship_cruiser(b):
    """Larger twin-funnel cruiser. Length ~92 model units."""
    length, beam, keel, waterline, deck = 92.0, 6.4, -7.5, -1.2, 2.4
    ship_hull(b, length, beam, keel, waterline, deck)
    box(b, 0.0, deck + 2.6, 8.0, 4.2, 2.6, 9.0, SUPER_GREY)
    box(b, 0.0, deck + 6.2, 10.0, 3.0, 1.6, 5.0, SUPER_GREY)
    box(b, 0.0, deck + 8.6, 11.0, 1.8, 1.2, 3.0, MACHINE_GREY)
    box(b, 0.0, deck + 13.0, 9.5, 0.26, 4.0, 0.26, MACHINE_GREY)
    cylinder(b, 0.0, deck + 4.0, 0.0, 2.0, 4.0, MACHINE_GREY, axis="y", taper=0.82)
    cylinder(b, 0.0, deck + 3.6, -11.0, 1.8, 3.6, MACHINE_GREY, axis="y", taper=0.82)
    box(b, 0.0, deck + 2.0, -20.0, 3.4, 2.0, 7.0, SUPER_GREY)
    box(b, 0.0, deck + 4.6, -19.0, 1.6, 1.2, 3.0, MACHINE_GREY)
    ship_turret(b, 30.0, deck, 1.25, facing=1.0)
    ship_turret(b, 20.0, deck, 1.25, facing=1.0)
    ship_turret(b, -31.0, deck, 1.25, facing=-1.0)
    # Secondary mounts along the beam
    for cz in (4.0, -6.0):
        box(b, 0.0, deck + 0.8, cz, 5.6, 0.8, 1.6, MACHINE_GREY)


# ---------------------------------------------------------------------------
# Blender plumbing
# ---------------------------------------------------------------------------
def assign_uvs(mesh, rects):
    """Planar-maps every face into its assigned atlas rectangle."""
    mesh.uv_layers.new(name="UVMap")
    uv = mesh.uv_layers.active.data
    for poly in mesh.polygons:
        u0, v0, u1, v1 = rects[poly.index]
        n = poly.normal
        axis = max(range(3), key=lambda i: abs(n[i]))
        i0, i1 = [i for i in range(3) if i != axis]
        pts = [mesh.vertices[mesh.loops[li].vertex_index].co for li in poly.loop_indices]
        a = [p[i0] for p in pts]
        c = [p[i1] for p in pts]
        amin, amax, cmin, cmax = min(a), max(a), min(c), max(c)
        da = (amax - amin) or 1.0
        dc = (cmax - cmin) or 1.0
        for k, li in enumerate(poly.loop_indices):
            s = (a[k] - amin) / da
            t = (c[k] - cmin) / dc
            uv[li].uv = (u0 + s * (u1 - u0), v0 + t * (v1 - v0))


def recalculate_normals(mesh):
    """
    Orients every face outward.

    The primitives above are written for readability rather than winding
    discipline, so rather than hand-verifying the vertex order of each one we
    let Blender fix it. Every part is a closed volume and the parts are separate
    connected components, so recalc orients each of them correctly on its own.
    """
    import bmesh

    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    bm.to_mesh(mesh)
    bm.free()
    mesh.update()


def build_object(name, builder, texture_path, tex_is_flipped):
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(builder.verts, [], builder.faces)
    mesh.update()
    recalculate_normals(mesh)
    assign_uvs(mesh, builder.rects)

    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    bsdf = nodes["Principled BSDF"]
    bsdf.inputs["Roughness"].default_value = 0.85
    tex = nodes.new("ShaderNodeTexImage")
    tex.image = bpy.data.images.load(texture_path)
    links.new(bsdf.inputs["Base Color"], tex.outputs["Color"])

    # UVs are authored for the game, where a texture loaded with flip=false has
    # v=0 at the TOP of the image. Blender puts v=0 at the bottom, so for those
    # textures the preview must flip V or it shows a mirrored region of the
    # atlas and misrepresents what the game will actually draw.
    if not tex_is_flipped:
        uvmap = nodes.new("ShaderNodeUVMap")
        mapping = nodes.new("ShaderNodeMapping")
        mapping.inputs["Location"].default_value = (0.0, 1.0, 0.0)
        mapping.inputs["Scale"].default_value = (1.0, -1.0, 1.0)
        links.new(mapping.inputs["Vector"], uvmap.outputs["UV"])
        links.new(tex.inputs["Vector"], mapping.outputs["Vector"])

    mesh.materials.append(mat)

    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    # Flat shading suits the faceted low-poly look and matches the game's
    # per-face normals
    for poly in obj.data.polygons:
        poly.use_smooth = False
    return obj


def export_obj(obj, path):
    for o in bpy.data.objects:
        o.select_set(False)
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.wm.obj_export(
        filepath=path,
        forward_axis="Y",          # verified identity mapping
        up_axis="Z",
        export_selected_objects=True,
        export_uv=True,
        export_normals=True,
        export_materials=True,
        export_triangulated_mesh=True,
        path_mode="STRIP",
    )


def clear_scene():
    for o in list(bpy.data.objects):
        bpy.data.objects.remove(o, do_unlink=True)


def render_preview(obj, path, _radius=None):
    """Three-quarter view render so the silhouette can be eyeballed."""
    from mathutils import Vector

    # Frame the object from its own bounds rather than a hand-tuned distance
    corners = [obj.matrix_world @ Vector(c) for c in obj.bound_box]
    lo = Vector((min(c.x for c in corners), min(c.y for c in corners), min(c.z for c in corners)))
    hi = Vector((max(c.x for c in corners), max(c.y for c in corners), max(c.z for c in corners)))
    center = (lo + hi) / 2.0
    size = hi - lo
    extent = max(size.x, size.y, size.z)

    # Look from across the longest axis, otherwise a long hull is viewed
    # end-on and its silhouette cannot be judged at all
    if size.z > size.x:
        direction = (1.5, 0.7, 0.5)      # ships: from starboard, slightly ahead
    else:
        direction = (0.75, 0.55, 1.15)   # planes: from ahead and above

    cam_data = bpy.data.cameras.new("cam")
    cam = bpy.data.objects.new("cam", cam_data)
    bpy.context.collection.objects.link(cam)

    from mathutils import Matrix

    d = Vector(direction).normalized()
    cam.location = center + d * (extent * 1.5)

    # Build the look-at by hand. mathutils' to_track_quat aligns the camera's up
    # toward world +Z, but these models are authored +Y up (the game's
    # convention), so using it rolls every preview 90 degrees and a long hull
    # ends up viewed end-on.
    forward = (center - cam.location).normalized()
    world_up = Vector((0.0, 1.0, 0.0))
    right = forward.cross(world_up).normalized()
    up = right.cross(forward)
    rot = Matrix((
        (right.x, up.x, -forward.x),
        (right.y, up.y, -forward.y),
        (right.z, up.z, -forward.z),
    )).to_4x4()
    cam.matrix_world = Matrix.Translation(cam.location) @ rot
    bpy.context.scene.camera = cam

    sun_data = bpy.data.lights.new("sun", type="SUN")
    sun_data.energy = 4.0
    sun = bpy.data.objects.new("sun", sun_data)
    sun.rotation_euler = (math.radians(50), math.radians(20), math.radians(30))
    bpy.context.collection.objects.link(sun)

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE_NEXT"
    scene.render.resolution_x = 900
    scene.render.resolution_y = 600
    scene.render.filepath = path
    scene.render.image_settings.file_format = "PNG"
    scene.world = bpy.data.worlds.new("w")
    scene.world.use_nodes = True
    scene.world.node_tree.nodes["Background"].inputs[0].default_value = (0.35, 0.45, 0.6, 1)
    bpy.ops.render.render(write_still=True)

    bpy.data.objects.remove(cam, do_unlink=True)
    bpy.data.objects.remove(sun, do_unlink=True)


# name, builder, texture file, whether that texture is loaded with flip=true
MODELS = [
    ("plane_falcon", plane_fighter, "planetexture.png", True),
    ("plane_marauder", plane_bomber, "planetexture.png", True),
    ("plane_lancet", plane_interceptor, "planetexture.png", True),
    ("plane_warhog", plane_attacker, "planetexture.png", True),
    ("warship_destroyer", ship_destroyer, "warship2.jpg", False),
    ("warship_cruiser", ship_cruiser, "warship2.jpg", False),
]


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    out_dir = argv[0]
    preview_dir = None
    if "--preview" in argv:
        preview_dir = argv[argv.index("--preview") + 1]
        os.makedirs(preview_dir, exist_ok=True)
    os.makedirs(out_dir, exist_ok=True)

    tex_dir = os.path.join(os.path.dirname(__file__), "..", "..", "assets", "textures")

    for name, fn, texture, flipped in MODELS:
        clear_scene()
        b = Builder()
        fn(b)
        obj = build_object(name, b, os.path.abspath(os.path.join(tex_dir, texture)), flipped)
        export_obj(obj, os.path.join(out_dir, f"{name}.obj"))
        print(f"BUILT {name}: {len(b.verts)} verts, {len(b.faces)} faces")
        if preview_dir:
            render_preview(obj, os.path.join(preview_dir, f"{name}.png"), None)


main()
