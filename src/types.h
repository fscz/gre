#ifndef gre_types_h
#define gre_types_h

#include "list.h"
#include "hashmap.h"
#include "escontext.h"

typedef unsigned int Framerate;


typedef struct {
  void* data;  
  size_t size; 
  size_t numComponents; // 1-4
  uint useBuffer;
  GLenum dataType;
} Attribute;

typedef struct {
  void* data;
  GLenum textureFormat; 
  GLenum textureType;
  size_t width;
  size_t height;  
  uint textureHandle;
  GLenum sampleMode;
} Uniform;

typedef struct {  
  // type: always use GL_unsigned_short
  GLenum type;
  size_t count;  
  size_t start; // glDrawArrays -> use start; else ignore
  Attribute* indices; // NULL -> use glDrawArrays
} Draw;

typedef struct {
  size_t stage;
  const char* vShader;
  const char* fShader;
  HashMap attributes; // :: [char* -> Attribute*]
  HashMap uniforms; // :: [char* -> Uniform*]
  Draw* draw;
} SceneObject;

typedef struct {

} Camera;

typedef struct {
  Camera camera; // :: Camera
  List initialObjects;
  uint running;
} Scene;

typedef struct {
  Scene* scene;
  ESContext* esContext;
  void (*updateFunc) (void* userData, unsigned long long);
  void* userData;
} RenderContext;

#endif
