#include <Python.h>
#include <structmember.h>
#include <gre.h>

//////////////////////////////////////////////////////////
///////////////////////////// Attribute
//////////////////////////////////////////////////////////

typedef struct {
  PyObject_HEAD
  GREHandle handle;
} PyGRE_Attribute;

static void
PyGRE_Attribute_dealloc(PyGRE_Attribute* self) {
  Attribute* attribute = (Attribute*)self->handle;
  if ( attribute ) {
    free ( attribute->data );
    gre_attribute_free( (GREHandle)attribute );
  }
  self->ob_type->tp_free((PyObject*)self);
}

static int
PyGRE_Attribute_init(PyGRE_Attribute *self, PyObject *args, PyObject *kwds) {
  if ( self->handle ) {
    PyErr_SetString(PyExc_AttributeError, "cannot reinitialize \"Attribute\"");
    return -1;
  }

  static char *kwlist[] = {"data", "dataType", "numComponents", "useBuffer", NULL};

  void* bytes;
  int numBytes;
  int dataType;
  size_t numComponents;
  uint useBuffer;
  void* dataArray;

  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "s#iII", kwlist, &bytes, &numBytes, &dataType, &numComponents, &useBuffer) ) {
    PyErr_SetString(PyExc_AttributeError, "\"Attribute\" initializer takes following parameters (str, int, int, bool)");
    return -1;
  }  

  if ( numComponents > 4 || numComponents <=0 ) {
    PyErr_SetString(PyExc_AttributeError, "\"Attribute\" must satisfy 0 < numComponents <= 4");
    return -1;
  }
  
  dataArray = malloc(numBytes);
  memcpy ( dataArray, bytes, numBytes );

  self->handle = gre_create_attribute(dataArray, dataType, numBytes, numComponents, useBuffer);
  return 0;
}

static PyMemberDef PyGRE_Attribute_members[] = {
  {NULL}  /* Sentinel */
};

static PyMethodDef PyGRE_Attribute_methods[] = {
  {NULL}  /* Sentinel */
};

static PyTypeObject PyGRE_AttributeType = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "pygre.Attribute",             /*tp_name*/
  sizeof(PyGRE_Attribute),             /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor)PyGRE_Attribute_dealloc, /*tp_dealloc*/
  0,                         /*tp_print*/
  0,                         /*tp_getattr*/
  0,                         /*tp_setattr*/
  0,                         /*tp_compare*/
  0,                         /*tp_repr*/
  0,                         /*tp_as_number*/
  0,                         /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  0,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "GRE Attribute",           /* tp_doc */
  0,                     /* tp_traverse */
  0,                     /* tp_clear */
  0,                     /* tp_richcompare */
  0,                     /* tp_weaklistoffset */
  0,                     /* tp_iter */
  0,                     /* tp_iternext */
  PyGRE_Attribute_methods,             /* tp_methods */
  PyGRE_Attribute_members,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)PyGRE_Attribute_init,      /* tp_init */
  0,                         /* tp_alloc */
  PyType_GenericNew,                 /* tp_new */
};


//////////////////////////////////////////////////////////
///////////////////////////// Uniform
//////////////////////////////////////////////////////////

typedef struct {
  PyObject_HEAD
  GREHandle handle;
} PyGRE_Uniform;

static void
PyGRE_Uniform_dealloc(PyGRE_Uniform* self) {
  
  Uniform* uniform = (Uniform*)self->handle;

  if ( uniform ) {
    if ( uniform->data ) {
      free ( uniform->data );
    }
    gre_uniform_free( (GREHandle)uniform );
  }
  self->ob_type->tp_free((PyObject*)self);
}

static int
PyGRE_Uniform_init(PyGRE_Uniform *self, PyObject *args, PyObject *kwds) {  
  if ( self->handle ) {
    PyErr_SetString(PyExc_AttributeError, "cannot reinitialize \"Uniform\"");
    return -1;
  }

  int numKwds = 0;
  if ( kwds ) numKwds = PyDict_Size( kwds );
  int numArgs = 0;
  if ( args ) numArgs = PyTuple_Size( args );
  size_t numArgsKwds = ( numKwds >= 0 ? numKwds : 0 ) + ( numArgs >= 0 ? numArgs : 0 );  

    
  if ( 1 == numArgsKwds ) { // just uniform data, ie. glUniform[1234][if]v?

    static char *kwlist[] = {"data", NULL};
    void* bytes = NULL;
    size_t numBytes = 0;
    if ( !PyArg_ParseTupleAndKeywords(args, kwds, "s#", kwlist, &bytes, &numBytes) ) {
      PyErr_SetString(PyExc_AttributeError, "invalid parameters, \"data\" must be byte string");
      return -1;
    }    

    void* dataArray = malloc(numBytes);
    memcpy( dataArray, bytes, numBytes);

    self->handle = gre_create_uniform( dataArray );

  } else if ( 2 == numArgsKwds ) {
    static char *kwlist[] = {"textureHandle", "sampleMode", NULL};
    uint textureHandle, sampleMode;
    if ( !PyArg_ParseTupleAndKeywords(args, kwds, "II", kwlist, &textureHandle, &sampleMode) ) {
      PyErr_SetString(PyExc_AttributeError, "invalid parameters, \"textureHandle\" and \"sampleMode\" must be int values");
      return -1;
    }
    
    self->handle = gre_create_uniform_texture_handle( textureHandle, sampleMode );

  } else if ( 6 == numArgsKwds ) {
    static char *kwlist[] = {"data", "width", "height", "textureFormat", "textureType", "sampleMode", NULL};

    void* bytes;
    int numBytes;
    size_t width;
    size_t height;
    int textureFormat;
    int textureType;
    int sampleMode; 

    if ( !PyArg_ParseTupleAndKeywords(args, kwds, "s#IIiii", kwlist, &bytes, &numBytes, &width, &height, &textureFormat, &textureType, &sampleMode) ) {
      PyErr_SetString(PyExc_AttributeError, "invalid parameters, \"data\" must be str type, other parameters must be int values");
      return -1;
    }

    void* dataArray = malloc(numBytes);
    memcpy ( dataArray, bytes, numBytes );

    self->handle = gre_create_uniform_texture( dataArray, textureFormat, textureType, width, height, sampleMode );

  } else {
    PyErr_SetString(PyExc_AttributeError, "\"Uniform\" initializer takes 1, 2 or 6 parameters");
    return -1;
  }


  return 0;
}

static PyObject*
PyGRE_Uniform_update(PyGRE_Uniform* self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"data", NULL};

  void* bytes = NULL;
  size_t numBytes = 0;
  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "s#", kwlist, &bytes, &numBytes) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, \"data\" must be byte string");
    return NULL;
  }
  
  free ( gre_uniform_data_get(self->handle) );

  void* dataArray = malloc(numBytes);
  memcpy( dataArray, bytes, numBytes );

  gre_uniform_data_set (self->handle, dataArray );

  Py_RETURN_NONE;
}

static PyMemberDef PyGRE_Uniform_members[] = {
  {NULL}  /* Sentinel */
};

static PyMethodDef PyGRE_Uniform_methods[] = {
  {"update", (PyCFunction)PyGRE_Uniform_update, METH_VARARGS, "update a uniform with new values" },
  {NULL}  /* Sentinel */
};

static PyTypeObject PyGRE_UniformType = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "pygre.Uniform",             /*tp_name*/
  sizeof(PyGRE_Uniform),             /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor)PyGRE_Uniform_dealloc, /*tp_dealloc*/
  0,                         /*tp_print*/
  0,                         /*tp_getattr*/
  0,                         /*tp_setattr*/
  0,                         /*tp_compare*/
  0,                         /*tp_repr*/
  0,                         /*tp_as_number*/
  0,                         /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  0,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "GRE Uniform",           /* tp_doc */
  0,                     /* tp_traverse */
  0,                     /* tp_clear */
  0,                     /* tp_richcompare */
  0,                     /* tp_weaklistoffset */
  0,                     /* tp_iter */
  0,                     /* tp_iternext */
  PyGRE_Uniform_methods,             /* tp_methods */
  PyGRE_Uniform_members,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)PyGRE_Uniform_init,      /* tp_init */
  0,                         /* tp_alloc */
  PyType_GenericNew,                 /* tp_new */
};


//////////////////////////////////////////////////////////
///////////////////////////// Draw
//////////////////////////////////////////////////////////

typedef struct {
  PyObject_HEAD
  PyObject* indices;
  GREHandle handle;
} PyGRE_Draw;

static void
PyGRE_Draw_dealloc(PyGRE_Draw* self) {  
  
  if ( self->handle ) {
    gre_draw_free( self->handle );
  }  
  
  Py_XDECREF(self->indices);
  self->ob_type->tp_free((PyObject*)self);
}

static int
PyGRE_Draw_init(PyGRE_Draw *self, PyObject *args, PyObject *kwds) {  

  if ( self->handle ) {
    PyErr_SetString(PyExc_AttributeError, "cannot reinitialize \"Draw\"");
    return -1;
  }

  static char *kwlist[] = {"type", "start", "count", "indices", NULL};

  int type;
  size_t count;
  size_t start;
  PyObject* indices = NULL;
  

  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "iIIO", kwlist, &type, &start, &count, &indices) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, signature is: (int, int, int, Attribute)");
    return -1;
  }

  if ( Py_None != indices ) {
    if (!PyObject_TypeCheck(indices, &PyGRE_AttributeType)) {
      PyErr_SetString(PyExc_AttributeError, "invalid parameters, signature is: (int, int, int, Attribute");
      return -1;
    }

    Py_INCREF ( indices );
    self->indices = indices;

    GREHandle attribHandle = ((PyGRE_Attribute*)indices)->handle;

    self->handle = gre_create_draw_elements(type, start, count, attribHandle);

  } else {

    self->handle = gre_create_draw_arrays(type, start, count);
  }


  return 0;
}

static PyMemberDef PyGRE_Draw_members[] = {
  {NULL}  /* Sentinel */
};

static PyMethodDef PyGRE_Draw_methods[] = {
  {NULL}  /* Sentinel */
};

static PyTypeObject PyGRE_DrawType = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "pygre.Draw",             /*tp_name*/
  sizeof(PyGRE_Draw),             /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor)PyGRE_Draw_dealloc, /*tp_dealloc*/
  0,                         /*tp_print*/
  0,                         /*tp_getattr*/
  0,                         /*tp_setattr*/
  0,                         /*tp_compare*/
  0,                         /*tp_repr*/
  0,                         /*tp_as_number*/
  0,                         /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  0,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "GRE Draw",           /* tp_doc */
  0,                     /* tp_traverse */
  0,                     /* tp_clear */
  0,                     /* tp_richcompare */
  0,                     /* tp_weaklistoffset */
  0,                     /* tp_iter */
  0,                     /* tp_iternext */
  PyGRE_Draw_methods,             /* tp_methods */
  PyGRE_Draw_members,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)PyGRE_Draw_init,      /* tp_init */
  0,                         /* tp_alloc */
  PyType_GenericNew,                 /* tp_new */
};

//////////////////////////////////////////////////////////
///////////////////////////// SceneObject
//////////////////////////////////////////////////////////

typedef struct {
  PyObject_HEAD
  PyObject* vShader;
  PyObject* fShader;
  PyObject* draw;
  GREHandle handle;
  PyObject* attributes;
  PyObject* uniforms;
} PyGRE_SceneObject;

static void
PyGRE_SceneObject_dealloc(PyGRE_SceneObject* self) {  

  if ( self->handle ) {
    gre_so_free( self->handle );
  }  

  Py_XDECREF(self->vShader);
  Py_XDECREF(self->fShader);
  Py_XDECREF(self->draw);
  Py_XDECREF(self->attributes);
  Py_XDECREF(self->uniforms);
  self->ob_type->tp_free((PyObject*)self);
}

static int
PyGRE_SceneObject_init(PyGRE_SceneObject *self, PyObject *args, PyObject *kwds)
{  

  if ( self->handle ) {
    PyErr_SetString(PyExc_AttributeError, "cannot reinitialize \"SceneObject\"");
    return -1;
  }

  static char *kwlist[] = {"stagebit", "vShader", "fShader", "draw", NULL};
  
  PyObject* vShader = NULL;
  PyObject* fShader = NULL;
  PyObject* draw = NULL;
  uint stagebit = 0;

  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "ISSO", kwlist, &stagebit, &vShader, &fShader, &draw) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, signature is: (str, str, Draw)");
    return -1;
  }  

  if ( Py_None == draw || Py_None == vShader || Py_None == fShader ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, None type not allowed");
    return -1;
  } 

  self->attributes = PyDict_New();
  self->uniforms = PyDict_New();

  
  Py_INCREF ( vShader );
  Py_INCREF ( fShader );
  Py_INCREF ( draw );

  self->vShader = vShader;
  self->fShader = fShader;
  self->draw = draw;

  GREHandle drawHandle = ((PyGRE_Draw*)draw)->handle;
  self->handle = gre_create_so(stagebit, PyString_AsString(vShader), PyString_AsString(fShader), drawHandle);

  return 0;
}

static PyObject*
PyGRE_SceneObject_setAttribute(PyGRE_SceneObject* self, PyObject *args, PyObject *kwds) {

  static char *kwlist[] = {"name", "attribute", NULL};
  PyObject* name = NULL;
  PyObject* attribute = NULL;
  
  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "SO", kwlist, &name, &attribute) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, add accepts (name:str, attribute:\"Attribute\") as input");
    return NULL;
  }

  if (!PyObject_TypeCheck(attribute, &PyGRE_AttributeType)) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, add accepts (name:str, attribute:\"Attribute\") as input");
    return NULL;
  }

  if ( PyDict_Contains(self->attributes, name) ) {
    PyDict_DelItem(self->attributes, name);
  }

  if ( -1 == PyDict_SetItem(self->attributes, name, attribute) ) {

    PyErr_SetString(PyExc_AttributeError, "could not set attribute");
    return NULL;
  }

  GREHandle attributeHandle = ((PyGRE_Attribute*)attribute)->handle;

  gre_so_attribute_set ( self->handle, PyString_AsString(name), attributeHandle );

  Py_RETURN_NONE;
}

static PyObject*
PyGRE_SceneObject_setUniform(PyGRE_SceneObject* self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"name", "uniform", NULL};
  PyObject* name = NULL;
  PyObject* uniform = NULL;
  
  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "SO", kwlist, &name, &uniform) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, add accepts (name:str, uniform:\"Uniform\") as input");
    return NULL;
  }

  if (!PyObject_TypeCheck(uniform, &PyGRE_UniformType)) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, add accepts (name:str, attribute:\"Uniform\") as input");
    return NULL;
  }

  if ( PyDict_Contains(self->uniforms, name) ) {
    PyDict_DelItem(self->uniforms, name);
  } 

  if ( -1 == PyDict_SetItem(self->uniforms, name, uniform) ) {

    PyErr_SetString(PyExc_AttributeError, "could not set uniform");
    return NULL;
  }

  GREHandle uniformHandle = ((PyGRE_Uniform*)uniform)->handle;

  gre_so_uniform_set ( self->handle, PyString_AsString(name), uniformHandle );

  Py_RETURN_NONE;
}

static PyMemberDef PyGRE_SceneObject_members[] = {
  {NULL}  /* Sentinel */
};
  
static PyMethodDef PyGRE_SceneObject_methods[] = {
  {"setAttribute", (PyCFunction)PyGRE_SceneObject_setAttribute, METH_VARARGS, "set attribute on sceneobject" },
  {"setUniform", (PyCFunction)PyGRE_SceneObject_setUniform, METH_VARARGS, "set uniform on sceneobject" },
  {NULL}  /* Sentinel */
};

static PyTypeObject PyGRE_SceneObjectType = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "pygre.SceneObject",             /*tp_name*/
  sizeof(PyGRE_SceneObject),             /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor)PyGRE_SceneObject_dealloc, /*tp_dealloc*/
  0,                         /*tp_print*/
  0,                         /*tp_getattr*/
  0,                         /*tp_setattr*/
  0,                         /*tp_compare*/
  0,                         /*tp_repr*/
  0,                         /*tp_as_number*/
  0,                         /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  0,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "GRE SceneObject",           /* tp_doc */
  0,                     /* tp_traverse */
  0,                     /* tp_clear */
  0,                     /* tp_richcompare */
  0,                     /* tp_weaklistoffset */
  0,                     /* tp_iter */
  0,                     /* tp_iternext */
  PyGRE_SceneObject_methods,             /* tp_methods */
  PyGRE_SceneObject_members,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)PyGRE_SceneObject_init,      /* tp_init */
  0,                         /* tp_alloc */
  PyType_GenericNew,                 /* tp_new */
};

//////////////////////////////////////////////////////////
///////////////////////////// Scene
//////////////////////////////////////////////////////////

typedef struct {
  PyObject_HEAD
  GREHandle handle;
} PyGRE_Scene;

static void
PyGRE_Scene_dealloc(PyGRE_Scene* self) {  
  if ( self->handle ) {
    gre_scene_free( self->handle );
  }  
  
  self->ob_type->tp_free((PyObject*)self);
}

static void PyGRE_Scene_updateFunction(void* userData, unsigned long long missed) {
  PyObject* result = PyObject_CallMethod((PyObject*)userData, "__update__", "(K)", missed);
  Py_XDECREF(result);
}

static int
PyGRE_Scene_init(PyGRE_Scene *self, PyObject *args, PyObject *kwds) {  

  if ( self->handle ) {
    PyErr_SetString(PyExc_AttributeError, "cannot reinitialize \"Scene\"");
    return -1;
  }

  static char *kwlist[] = {NULL};

  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, Scene accepts no arguments");
    return -1;
  }  

  self->handle = gre_scene_alloc();

  return 0;
}

static PyObject*
PyGRE_Scene_add(PyGRE_Scene* self, PyObject *args, PyObject *kwds) {

  static char *kwlist[] = {"so", NULL};
  PyObject* so = NULL;
  
  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &so) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, add accepts a single \"SceneObject\" as input");
    return NULL;
  }  

  if (!PyObject_TypeCheck(so, &PyGRE_SceneObjectType)) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, add accepts a SceneObject as input");
    return NULL;
  }

  GREHandle soHandle = ((PyGRE_SceneObject*)so)->handle;

  gre_scene_add_so( self->handle, soHandle );
  Py_INCREF(so);

  Py_RETURN_NONE;
}

static PyObject*
PyGRE_Scene_remove(PyGRE_Scene* self, PyObject *args, PyObject *kwds) {
  
  static char *kwlist[] = {"so", NULL};
  PyObject* so = NULL;
  
  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &so) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, remove accepts a single \"SceneObject\" as input");
    return NULL;
  }

  if (!PyObject_TypeCheck(so, &PyGRE_SceneObjectType)) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, remove accepts a SceneObject as input");
    return NULL;
  }

  GREHandle soHandle = ((PyGRE_SceneObject*)so)->handle;

  if ( 0 == gre_scene_remove_so ( self->handle, soHandle ) ) {
    Py_DECREF(so);
  }
  
  Py_RETURN_NONE;
}

static PyObject*
PyGRE_Scene_start(PyGRE_Scene* self, PyObject *args, PyObject *kwds) {

  static char *kwlist[] = {NULL};
  
  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist ) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, start accepts no parameters");
    return NULL;
  }  
  
  GREHandle sceneHandle = self->handle;

  gre_scene_start( sceneHandle, 30, &PyGRE_Scene_updateFunction, self );   

  Py_RETURN_NONE;
}

static PyObject*
PyGRE_Scene_stop(PyGRE_Scene* self, PyObject *args, PyObject *kwds) {

  static char *kwlist[] = {NULL};
  
  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist ) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, stop accepts no parameters");
    return NULL;
  }  
  
  GREHandle sceneHandle = self->handle;

  gre_scene_stop( sceneHandle );

  Py_RETURN_NONE;
}

static PyObject* 
PyGRE_Scene_screenshot(PyGRE_Scene* self, PyObject *args, PyObject * kwds) {

  static char *kwlist[] = {"x", "y", "width", "height", NULL};

  size_t x, y, width, height;

  if ( !PyArg_ParseTupleAndKeywords(args, kwds, "IIII", kwlist, &x, &y, &width, &height ) ) {
    PyErr_SetString(PyExc_AttributeError, "invalid parameters, screenshot accepts 4 integers: x, y, width, height");
    return NULL;
  }

  size_t bufsize = width * height * 3; // rgb
  char buffer[bufsize];

  gre_scene_snapshot(self->handle, x, y, width, height, buffer);
  return Py_BuildValue("s#", buffer, bufsize);
}

static PyObject* 
PyGRE_Scene_update(PyGRE_Scene* self, PyObject *args, PyObject *kwds) {
  Py_RETURN_NONE;
}

static PyMemberDef PyGRE_Scene_members[] = {
  {NULL}  /* Sentinel */
};

static PyMethodDef PyGRE_Scene_methods[] = {
  {"add", (PyCFunction)PyGRE_Scene_add, METH_VARARGS, "add a scene object" },
  {"remove", (PyCFunction)PyGRE_Scene_remove, METH_VARARGS, "remove a scene object" },
  {"screenshot", (PyCFunction)PyGRE_Scene_screenshot, METH_VARARGS, "screenshot the current display" },
  {"update", (PyCFunction)PyGRE_Scene_update, METH_VARARGS, "update callback" },
  {"start", (PyCFunction)PyGRE_Scene_start, METH_VARARGS, "start rendering a scene" },
  {"stop", (PyCFunction)PyGRE_Scene_stop, METH_VARARGS, "stop rendering a scene" },
  {NULL}  /* Sentinel */
};

static PyTypeObject PyGRE_SceneType = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "pygre.Scene",             /*tp_name*/
  sizeof(PyGRE_Scene),             /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor)PyGRE_Scene_dealloc, /*tp_dealloc*/
  0,                         /*tp_print*/
  0,                         /*tp_getattr*/
  0,                         /*tp_setattr*/
  0,                         /*tp_compare*/
  0,                         /*tp_repr*/
  0,                         /*tp_as_number*/
  0,                         /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  0,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "GRE Scene",           /* tp_doc */
  0,                     /* tp_traverse */
  0,                     /* tp_clear */
  0,                     /* tp_richcompare */
  0,                     /* tp_weaklistoffset */
  0,                     /* tp_iter */
  0,                     /* tp_iternext */
  PyGRE_Scene_methods,             /* tp_methods */
  PyGRE_Scene_members,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)PyGRE_Scene_init,      /* tp_init */
  0,                         /* tp_alloc */
  PyType_GenericNew,                 /* tp_new */
};


//////////////////////////////////////////////////////////
///////////////////////////// Module
//////////////////////////////////////////////////////////

static PyMethodDef module_methods[] = {
  {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initpygre(void)
{
  PyObject* m;

  if ( PyType_Ready(&PyGRE_AttributeType) < 0 ) return;
  if ( PyType_Ready(&PyGRE_UniformType) < 0 ) return;
  if ( PyType_Ready(&PyGRE_DrawType) < 0 ) return;
  if ( PyType_Ready(&PyGRE_SceneObjectType) < 0 ) return;
  if ( PyType_Ready(&PyGRE_SceneType) < 0 ) return;

  m = Py_InitModule3("pygre", module_methods,
		     "GRE - Generic Render Engine. Python Bindings.");

  if (m == NULL) return;

  Py_INCREF(&PyGRE_AttributeType);
  PyModule_AddObject(m, "Attribute", (PyObject *)&PyGRE_AttributeType);

  Py_INCREF(&PyGRE_UniformType);
  PyModule_AddObject(m, "Uniform", (PyObject *)&PyGRE_UniformType);

  Py_INCREF(&PyGRE_DrawType);
  PyModule_AddObject(m, "Draw", (PyObject *)&PyGRE_DrawType);

  Py_INCREF(&PyGRE_SceneObjectType);
  PyModule_AddObject(m, "SceneObject", (PyObject *)&PyGRE_SceneObjectType);

  Py_INCREF(&PyGRE_SceneType);
  PyModule_AddObject(m, "Scene", (PyObject *)&PyGRE_SceneType);

  gre_initialize();

  size_t width, height;
  gre_get_screen_size(&width, &height);
  
  PyObject *moduleDict = PyModule_GetDict(m);
  PyObject *classDict = PyDict_New();
  PyObject *className = PyString_FromString("Display");
  PyObject *displayClass = PyClass_New(NULL, classDict, className);
  PyDict_SetItemString(moduleDict, "Display", displayClass);
  Py_DECREF(classDict);
  Py_DECREF(className);
  Py_DECREF(displayClass);
    
  PyDict_SetItemString(classDict, "size", Py_BuildValue("ff", (float)width, (float)height));
  PyDict_SetItemString(classDict, "width", Py_BuildValue("f", (float)width));
  PyDict_SetItemString(classDict, "height", Py_BuildValue("f", (float)height));
  PyDict_SetItemString(classDict, "aspect", Py_BuildValue("f", (float)width / (float)height));
}
