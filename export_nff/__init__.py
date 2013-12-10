## NFF exporter

bl_info = {
  "name": "Neutral File Format Exporter",
  "author": "Maarja Lepamets",
  "version": (1, 0, 0),
  "blender": (2, 66, 1),
  "location": "File > Export > Neutral File Format (.nff)",
  "category": "Import-Export"}

import bpy
from bpy.props import StringProperty
from bpy.props import BoolProperty

# calling a file selector
class ExportNFF(bpy.types.Operator):
  
  bl_idname = "export.nff"	# operator identifier
  bl_label = "Export NFF"
  
  filepath = StringProperty(subtype = 'FILE_PATH')
  
  ExportMeshes = BoolProperty(name = "Export Meshes", description = "Export mesh objects", default = True)
        
  ExportNormals = BoolProperty(name = "Export Normals", description = "Export mesh normals", default = True)
  
  ExportMaterials = BoolProperty(name = "Export Materials", description = "Export material properties", default = True)
  
  ExportUVCoordinates = BoolProperty(name = "Export UV Coordinates", description = "Export mesh UV coordinates, if any", default = True)

  def execute(self, context):
    # adds extension ".nff" to the filepath if it is not already set
    self.filepath = bpy.path.ensure_ext(self.filepath, ".nff")
    from . import nff_exporter
    Exporter = nff_exporter.NFFExporter(self, context)
    Exporter.Export()
    return {'FINISHED'}

  # assigning properties used by execute()
  def invoke(self, context, event):
    if not self.filepath:
      self.filepath = bpy.path.ensure_ext(bpy.data.filepath, ".nff")
    # show up the file selector
    context.window_manager.fileselect_add(self)
    return {'RUNNING_MODAL'}

def menu_func(self, context):
  # places a button into the layout
  self.layout.operator(ExportNFF.bl_idname, text = "Neutral File Format (.nff)")


def register():
    # registering and adding to the file selector
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_file_export.append(menu_func)


def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
    register()