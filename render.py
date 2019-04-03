import bpy
import os
import math

# ======= HEALPFUL LINKS =======
# https://wiki.blender.org/wiki/Building_Blender
# https://janakiev.com/til/blender-command-line/
# https://docs.blender.org/api/blender_python_api_current/info_tips_and_tricks.html#blender-as-a-module


def delete_all_mesh():
    for o in bpy.data.objects:
        if o.type == 'MESH':
            o.select = True
        else:
            o.select = False
    bpy.ops.object.delete()

def find_camera():
    for o in bpy.data.objects:
        if o.type == 'CAMERA':
            return o
    return None


# load one chair model

def generate(filepath, index, view):
    # file_path = os.path.join('/Users/trailingend/Documents/SFU/CMPT764/chair-modeling', 'ChairModels/chair0004.obj')
    file_path = filepath
    chair_objs = bpy.ops.import_scene.obj(filepath = file_path)

    # camera loc - front view
    camera_obj = find_camera()
    if camera_obj is None:
        buffer = 0
        buffer = bpy.ops.object.camera_add()
        while buffer is 0:
            pass
        camera_obj = bpy.context.object
    bpy.context.scene.camera = camera_obj
    print("name of cam", camera_obj.name)
    if view == 'front':
        camera_obj.location.x = 0.0
        camera_obj.location.y = -0.5
        camera_obj.location.z = -4.0
        camera_obj.rotation_euler.x = 0.0
        camera_obj.rotation_euler.y = math.pi
        camera_obj.rotation_euler.z = math.pi
    elif view == 'top':
        camera_obj.location.x = 0.0
        camera_obj.location.y = -4.5
        camera_obj.location.z = -0.05
        camera_obj.rotation_euler.x = - math.pi / 2
        camera_obj.rotation_euler.y = math.pi
        camera_obj.rotation_euler.z = math.pi
    elif view == 'side':
        camera_obj.location.x = -4.0
        camera_obj.location.y = -0.3
        camera_obj.location.z = 0
        camera_obj.rotation_euler.x = 0
        camera_obj.rotation_euler.y = math.pi / 2
        camera_obj.rotation_euler.z = math.pi

    # mist 1-15
    world = bpy.data.worlds["World"]
    world.mist_settings.use_mist = True
    world.mist_settings.start = 1.0
    world.mist_settings.depth = 8.0

    # node
    scene = bpy.data.scenes["Scene"]
    scene.use_nodes = True
    nodes = scene.node_tree.nodes
    for node in nodes:
        nodes.remove(node)

    # add nodes
    render_layer = nodes.new(type="CompositorNodeRLayers") # type = 'R_LAYERS'
    composite_layer = nodes.new(type="CompositorNodeComposite") # type = 'COMPOSITE'
    map_range = nodes.new(type="CompositorNodeMapRange") # type ='MAP_RANGE'
    map_range.inputs[1].default_value = 3.5
    map_range.inputs[2].default_value = 7.0

    # link nodes
    links = scene.node_tree.links
    link_r_m = links.new(render_layer.outputs[2], map_range.inputs[0])
    link_m_c = links.new(map_range.outputs[0], composite_layer.inputs[0])

    # render and image
    bpy.context.scene.render.image_settings.file_format='BMP'
    bpy.context.scene.render.filepath = os.path.join("/Users/trailingend/Documents/SFU/CMPT764/chair-modeling", "ChairImages/model%0.2d-" % index + view + ".jpg")
    bpy.ops.render.render(use_viewport=True, write_still=True)

    # delete chair object
    delete_all_mesh()


filenames = []
for root, dirs, files in os.walk('/Users/trailingend/Documents/SFU/CMPT764/chair-modeling/ChairModels'):
    for file in files:
        if '.obj' in file:
            filenames.append(os.path.join(root, file))

for i in range(0, len(filenames)):
    # print(filenames[i])
    generate(filenames[i], i, "front")
    generate(filenames[i], i, "top")
    generate(filenames[i], i, "side")