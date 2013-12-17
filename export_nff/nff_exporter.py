## NFF exporter

import bpy
from mathutils import *
from math import *

class NFFExporter:
  def __init__(self, Config, context):
    
    # Config is basically the ExportNFF class from __init__.py
    self.Config = Config
    self.context = context
    
    self.filepath = self.Config.filepath
    self.File = None
    
    self.Scene = self.context.scene
    
    self.Camera = None
    self.Lights = []
    self.MeshObjects = []
    
    Objects = list(self.Scene.objects)  
    
    # Collect objects into categories
    for Object in Objects:
      if Object.type == 'MESH' and self.Config.ExportMeshes:
        self.MeshObjects.append(MeshObject(self.Config, self, Object))
      elif Object.type == 'CAMERA':
        self.Camera = Object
      elif Object.type == 'LAMP':
        self.Lights.append(Object)
    
  def Export(self):
    self.File = open(self.filepath, 'w')
    
    # Export camera and backgraound information
    self.__WriteBackground()
    self.__WriteViewpoint()
    
    # Export lights
    for Lamp in self.Lights:
      if Lamp.data.type == 'POINT':
        WritePointLamp(self, Lamp)
      elif Lamp.data.type == 'AREA':
        WriteAreaLamp(self, Lamp)
      elif Lamp.data.type == 'SPOT':
        WriteSpotLamp(self, Lamp)
    
    # Export meshes
    for Mesh in self.MeshObjects:
      Mesh.Write()
    self.File.close()
      
  def __WriteBackground(self):
    bg = self.Scene.world.horizon_color
    self.File.write("b %f %f %f\n" % (bg[0], bg[1], bg[2]))
    
  def __WriteViewpoint(self):
    m = self.Camera.matrix_world 
    loc = [m[0][3], m[1][3], m[2][3]]
    at = [m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2]]
    up = [m[0][1], m[1][1], m[2][1]]
    angle = self.Camera.data.angle * (180.0 / pi)	# FoV
    hither = self.Camera.data.clip_start	# near plane
    res_x = self.Scene.render.resolution_x
    res_y = self.Scene.render.resolution_y
    self.File.write("v\nfrom %f %f %f\n" % (loc[0], loc[1], loc[2]))
    self.File.write("at %f %f %f\n" % (at[0], at[1], at[2]))
    self.File.write("up %f %f %f\n" % (up[0], up[1], up[2]))
    self.File.write("angle %f\n" % (angle))
    self.File.write("hither %f\n" % (hither))
    self.File.write("resolution %d %d\n" % (res_x, res_y))
    
      
def WritePointLamp(Exporter, Lamp):
  loc = Lamp.location
  colour = Lamp.data.color
  Exporter.File.write("l %f %f %f %f %f %f\n" % (loc[0], loc[1], loc[2], colour[0], colour[1], colour[2]))
  
  
def WriteAreaLamp(Exporter, Lamp):
  m = Lamp.matrix_world
  size_x = Lamp.data.size
  size_y = size_x
  if Lamp.data.size_y != None:
    size_y = Lamp.data.size_y
  loc = Lamp.location
  vec_x = [size_x * m[0][0], size_x * m[1][0], size_x * m[2][0]]	# X vecotr
  vec_y = [size_y * m[0][1], size_x * m[1][1], size_x * m[2][1]]	# Y vector
  pos = [loc[0] - (vec_x[0] + vec_y[0]) / 2.0, loc[1] - (vec_x[1] + vec_y[1]) / 2.0, loc[2] - (vec_x[2] + vec_y[2]) / 2.0]
  colour = Lamp.data.color
  Exporter.File.write("la %f %f %f %f %f %f %f %f %f %f %f %f\n" % (pos[0], pos[1], pos[2], vec_x[0], vec_x[1], vec_x[2], vec_y[0], vec_y[1], vec_y[2], colour[0], colour[1], colour[2]))
  
  
def WriteSpotLamp(Exporter, Lamp):
  m = Lamp.matrix_world
  loc = Lamp.location
  colour = Lamp.data.color
  at = [m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2]]
  angle = Lamp.data.spot_size / 2 * (180.0 / pi)
  r = 0.01	# some small radius
  Exporter.File.write("lf %f %f %f %f %f %f %f %f %f %f %f\n" % (loc[0], loc[1], loc[2], r, at[0], at[1], at[2], angle, colour[0], colour[1], colour[2]))
  

class MeshObject:
  def __init__(self, Config, Exporter, BlenderObject):
    self.Config = Config
    self.Exporter = Exporter
    self.BlenderObject = BlenderObject
    
  def Write(self):      
    self.Mesh = self.BlenderObject.to_mesh(self.Exporter.context.scene, False, 'PREVIEW')
    self.Mesh.transform(self.BlenderObject.matrix_world)
    self.Mesh.calc_normals_split()
    if self.Config.ExportUVCoordinates:
      self.__WriteMeshWithTexture()
    elif self.Config.ExportMaterials:
      self.__WriteMeshWithMaterial()
    else:
      self.__WriteMeshOnly()
    
    
  def __WriteMeshWithMaterial(self):
    if len(self.Mesh.materials) == 0:
      self.__WriteMeshOnly()
      return
    
    id = 0
    for Material in self.Mesh.materials:
      self.__WriteMaterial(Material) 
      # collects all polygons with given material
      for Polygon in self.Mesh.polygons:
        if Polygon.material_index == id:
          self.__WritePolygon(Polygon) 
      id += 1
      
      
  def __WriteMeshWithTexture(self):
    # only if object has material and we also export that material
    if len(self.Mesh.materials) == 0 or not self.Config.ExportMaterials:
      self.__WriteMeshOnly() 
      return
    if not self.Mesh.uv_textures:
      self.Config.ExportUVCoordinates = False
      self.__WriteMeshWithMaterial()
      return

    id = 0
    for Material in self.Mesh.materials:
      tex = Material.active_texture
      if tex.type != 'IMAGE':
        self.__WriteMeshWithMaterial()
        return
      self.__WriteMaterial(Material)
      filepath = tex.image.filepath.split('/')
      name = filepath[len(filepath) - 1].split('.')
      path = str(name[0]) + ".tga"
      
      self.Exporter.File.write("t %s\n" % (path))
      
      for Polygon in self.Mesh.polygons:
        if Polygon.material_index == id:
          self.__WritePolygon(Polygon) 
      id += 1


        
  def __WriteMeshOnly(self):
    Polygons = self.Mesh.polygons
    for Polygon in Polygons:
      self.__WritePolygon(Polygon)
  
  
  def __WriteMaterial(self, Material):
    colour = Material.diffuse_color
    diffuse_comp = Material.diffuse_intensity
    specular_comp = Material.specular_intensity
    shine = Material.specular_hardness
    t = 1 - Material.alpha
    ior = 1.0
    if Material.transparency_method == 'RAYTRACE':
      ior = Material.raytrace_transparency.ior
    self.Exporter.File.write("f %f %f %f %f %f %d %f %f\n" % (colour[0], colour[1], colour[2], diffuse_comp, specular_comp, shine, t, ior))
    
    
  def __WritePolygon(self, Polygon):
    l = len(Polygon.vertices)
    
    if self.Config.ExportMaterials and self.Config.ExportUVCoordinates and Polygon.use_smooth == True:
      self.Exporter.File.write("ppt %d\n" % (l))
      for i in Polygon.loop_indices:
        loop = self.Mesh.loops[i]
        Vertex = self.Mesh.vertices[loop.vertex_index]
        coord = Vertex.co
        normal = Vertex.normal
        for j, ul in enumerate(self.Mesh.uv_layers):
            uv_coord = ul.data[loop.index].uv
        self.Exporter.File.write("%f %f %f %f %f %f %f %f\n" % (coord[0], coord[1], coord[2], normal[0], normal[1], normal[2], uv_coord[0], uv_coord[1]))
    
    elif self.Config.ExportMaterials and self.Config.ExportUVCoordinates:
      self.Exporter.File.write("ppt %d\n" % (l))
      normal = Polygon.normal
      for i in Polygon.loop_indices:
        loop = self.Mesh.loops[i]
        Vertex = self.Mesh.vertices[loop.vertex_index]
        coord = Vertex.co
        for j, ul in enumerate(self.Mesh.uv_layers):
            uv_coord = ul.data[loop.index].uv
        self.Exporter.File.write("%f %f %f %f %f %f %f %f\n" % (coord[0], coord[1], coord[2], normal[0], normal[1], normal[2], uv_coord[0], uv_coord[1]))
        
    elif Polygon.use_smooth == True:
      self.Exporter.File.write("pp %d\n" % (l))
      for i in Polygon.vertices:
        Vertex = self.Mesh.vertices[i]
        coord = Vertex.co
        normal = Vertex.normal
        self.Exporter.File.write("%f %f %f %f %f %f\n" % (coord[0], coord[1], coord[2], normal[0], normal[1], normal[2]))
          
    else:
      normal = Polygon.normal
      self.Exporter.File.write("pp %d\n" % (l))
      for i in Polygon.vertices:
        Vertex = self.Mesh.vertices[i]
        coord = Vertex.co
        self.Exporter.File.write("%f %f %f %f %f %f\n" % (coord[0], coord[1], coord[2], normal[0], normal[1], normal[2]))
