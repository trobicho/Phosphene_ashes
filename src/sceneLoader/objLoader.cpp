#include "objLoader.hpp"
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace ObjLoader {

void  load(const std::string filename, PhosObjectMesh &mesh, const ObjLoaderConfig &config) {

  tinyobj::ObjReader reader;
  tinyobj::ObjReaderConfig reader_config;


  std::string inputFile = filename;
  if (config.useRelativePath)
    inputFile = config.scenePath + filename;

  if (!reader.ParseFromFile(inputFile, reader_config)) {
    if (!reader.Error().empty()) {
      std::cerr << "TinyObjReader: " << reader.Error();
    }
    throw PhosHelper::BasicError(("Error reading Obj file: " + filename));
  }
  if (!reader.Warning().empty()) {
    std::cout << "TinyObjReader: " << reader.Warning();
  }

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  auto& materials = reader.GetMaterials();
  float maxY = 0.0;

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      for (size_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        mesh.indices.push_back(index_offset + v);

        Vertex vertex;
        tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
        tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
        tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];
        if (vy > maxY)
          maxY = vy;
        vertex.pos = glm::vec3(
            static_cast<float>(vx) * config.scale
            , static_cast<float>(vy) * config.scale
            , static_cast<float>(vz) * config.scale
          );

        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
          tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
          tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
          vertex.normal = glm::vec3(
              static_cast<float>(nx)
              , static_cast<float>(ny)
              , static_cast<float>(nz)
            );
        }

        if (idx.texcoord_index >= 0) {
          tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
          tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
          vertex.textCoord = glm::vec2(
              static_cast<float>(tx)
              , static_cast<float>(ty)
            );
        }
        mesh.vertices.push_back(vertex);
      }

      if (shapes[s].mesh.indices[index_offset].normal_index < 0) {
        uint32_t lastVertexIdx = mesh.vertices.size() - 1;
        glm::vec3 v1 = mesh.vertices[lastVertexIdx].pos - mesh.vertices[lastVertexIdx - 1].pos;
        glm::vec3 v2 = mesh.vertices[lastVertexIdx].pos - mesh.vertices[lastVertexIdx - 2].pos;
        glm::vec3 normal = glm::cross(v2, v1);
        normal = glm::normalize(normal);
        for (uint32_t n = 0; n < fv; n++) {
          mesh.vertices[lastVertexIdx - n].normal = normal;
        }
      }
      index_offset += fv;

      //shapes[s].mesh.material_ids[f];
    }
  }
}

}
