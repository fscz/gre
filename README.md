#gre 

is a lightweight 3d capable opengl es 2.0 render engine for the Raspberry Pi. It comes with a python binding so you can quickly jump into developping your graphics on rpi.
 

###get and build 
```bash
git clone https://github.com/fscz/gre.git
cd gre
./configure
make
```

###using gre
#### Python
gre comes with a python binding. The binding source is located in gre/binding/pygre.c.
Make will build a file gre/build/pygre.so. Copy this file to your local python project folder and import it like in the following example.

```python
from pygre import Attribute, Uniform, Draw, SceneObject, Scene, Display
```

Here is the module documentation for the Python Binding.
#####Display
Display contains information about the screen.
######Properties
- width: screen width (float value)
- height: scren height (float value)
- size: (width,height)
- aspect: width / height


#####Attribute
Attribute is a GL Attribute
######Methods
`__init__(self, bytes:char*, datatype:Int, numComponents:Int, useBuffer:Bool)`
- bytes: struct.pack('4f', 1.0, 0.0, 0.0, 1.0)
- datatype: byte/0x1400, unsigned_byte/0x1401, short/0x1402, unsigned_short/0x1403, float/0x1406
- numComponents: 1-4
- useBuffer: True/False specify if vertex attribute is bufferd
 

#####Uniform
Uniform is a GL Uniform
######Methods
`__init__(self, textureHandle:Int, sampleMode:Int)`
- textureHandle: this can be used to refer to a Handle that already exists in the GL Context, for example if you want to refer to a texture that is written by an ongoing omx video decode
- sampleMode: repeat/0x2901, clamp/0x812f, mirrored_repeat/0x8370
`__init__(self, bytes:char*)`
- bytes: struct.pack('16f', rotationMatrix)
`__init__(self, data:bytes, width:Int, height:Int, textureFormat:Int, textureType:Int, sampleMode:Int)`
- data: struct.pack... raw image data
- width: image width
- height: image height
- textureFormat: alpha/0x1906, rgb/0x1907, rgba/0x1908, luminance/0x1909, luminance_alpha/0x190a
- textureType: unsigned_byte:0x1401, unsigned_short_4_4_4_4/0x8033, unsigned_short_5_5_5_1/0x8034, unsgined_short_5_6_5/0x8363
- sampleMode: repeat/0x2901, clamp/0x812f, mirrored_repeat/0x8370
`update(self, bytes:char*)`
- bytes: struct.pack...# size of data must be same the size of the original data.


#####Draw
Draw is a container object that describes details of the related open gl draw call.
######Methods
`__init__(self, type:Int, start:Int, count:Int, indices:Attribute)`
- type: points/0x0000, lines/0x0001, line_loop/0x0002, line_strip/0x0003, triangles/0x0004, triangle_strip/0x0005, triangle_fan/0x0006
- start: 0-? an offset (of vertices) into the data
- count: 1-? number of vertices starting from offset
- indices: None or an Attribute. If you specify None this will result in a call to glDrawArrays, that is an unbuffered draw call. If you specify an Attribute for the indices this will result in a call to glDrawElements
 

#####SceneObject
A SceneObject is essentially a vertex shader, a fragment shader and a map of parameters in the form of Uniforms and Attributes
######Methods
`__init__(self, stagebit, vShader:String, fShader:String, drawMethod:Draw)`
- stagebit: gre is a light pre-pass renderer and renders scene objects in different stages. If you want a SceneObject to be rendered in stages 1 and 3, that is you want to render geometry, you specify 5 as the stage bit (2^0 + 2^2). For an overlay element you specify 16 (2^4).
  - Stage 1: Geometry
  - Stage 2: Lights
  - Stage 3: Geometry again (in light pre-pass geometry is rendered twice)
  - Stage 4: Particle systems
  - Stage 5: Overlay elements
- vShader: A Python String containing valid glsl vertex shader code
- fShader: A Python String containing valid glsl fragment shader code
- drawMethod: A Draw object



If you want to get started quickly you may also want to check out [rage - raspberry gaming engine](https://github.com/fscz/rage). It is a little game engine that uses gre via the python
binding and contains Classes like: Mesh, PointLight and Obj-file reader, input handling and so on.
It's also a good starting point to see how you can use gre from python. Check out the Game class
from rage that set's up a mainloop.

```python
from pygre import Scene
from input import HIDInput, get_devices

import sys
import signal
import os
import collections
import threading

class Game(Scene):
    def __init__(self):
        self.__lock__ = threading.Lock()
        self.downed_keys = {}
        Scene.__init__(self)
        self.__events = {}
        self.input_providers = []
        for devname in get_devices():
            evdev = HIDInput(self, devname)            
            self.input_providers.append(evdev)
            evdev.start()
        def handle_sigint(signum, frame):
            for evdev in self.input_providers:
                evdev.stop()                
            self.stop()
        signal.signal(signal.SIGINT, handle_sigint)

    def quit(self):
        for evdev in self.input_providers:
            evdev.stop()                
        self.stop()

    def dispatch(self, *args):
        with self.__lock__:
            if args[2] == 0:
                del self.__events[args[0]]
            else:
                self.__events[args[0]] = args

    @property
    def events(self):
        pass

    @events.getter
    def events(self):
        return self.__events

    def __update__(self, overruns):
        with self.__lock__:            
            self.update(overruns)       
```
This is the Game class that comes with rage.

####C
If you choose to work in C, copy either libgre.a or libgre.so from the build directory in your local gre directory. Most of the time you will need only 1 header file from build/include directory: gre.h

A sample might look like this:
```c
#include "gre/gre.h"
#include <gl2.h>

static const char* vShader =
    "attribute vec4 a_position;    \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_Position = a_position;  \n"
    "}                            \n";

static const char* fShader =
    "precision mediump float;\n"
    "uniform vec4 u_color\n;"
    "void main()                                  \n"
    "{                                            \n"
    "  gl_FragColor = u_color;\n"
    "}                                            \n"; 
    
void update(void* userData) {
  GREHandle sceneHandle = (GREHandle)userData;
  
  gre_scene_stop(sceneHandle);
}

int main(int argc, const char* argv[]) {
  gre_initialize();
  
  GREHandle drawTriangles = gre_create_draw_arrays(DRAW_TRIANGLES, 0, 6);
  GREHandle quadHandle = gre_create_so(16, vShader, fShader, drawTriangles);
  
  GLfloat vVertices[] = {
    0.0f, -1.0f,
    1.0f, -1.0f,
    0.0f, 1.0f,
    0.0f, 1.0f, 
    1.0f, -1.0f,
    1.0f, 1.0f,
  };
  GREHandle a_position = gre_create_attribute(vVertices, TYPE_FLOAT, sizeof(float)*12, 2, 0);
  gre_so_attribute_set ( quadHandle, "a_position", a_position );
  
  float colorRed[] = {
    0.0f, 0.0f, 1.0f, 1.0f
  };
  GREHandle uniformColor = gre_create_uniform(colorRed);
  gre_so_uniform_set ( quadHandle, "u_color",  uniformColor);
  
  GREHandle sceneHandle = gre_scene_alloc();
  
  gre_scene_add_so (sceneHandle, quadHandle ); 
  
  gre_scene_start( sceneHandle, 30, &update, sceneHandle );  // blocks until stopped
  
  gre_tear_down();
  
  return 0;
}

```

###concepts
gre operates on the following core types:
: Attribute - An opengl Attribute. gre also offers automatic buffering.
: Uniform - An opengl Uniform. A Uniform can be a texture, any float/int based data.
: Shader - A Shader is a char*
: SceneObject - A SceneObject is a { 
                       vertexShader:Shader, 
                       fragmentShader:Shader, 
                       uniforms:map<char*,Uniform>, 
                       attributes:map<char*,Attribute>,
                       drawMethod:Draw }
: Draw - A value holder struct that details how a SceneObject is drawn (GL_TRIANGLES,etc)
: Scene - A Scene is essentially a list of SceneObjects and some internal state values

The above types are normally created by calling one the gre_[type]_alloc or gre_create_[type]
functions. The latter are convenience functions that are composed of simpler functions.
gre_create_[type] and gre_[type]_alloc functions return a GREHandle that is subsequently used
to refer to the object. 
 
This requires some explanation: OpenGL defines different types of objects the lifecycle
of wich needs to be managed by the programmer. Such objects include Framebuffers, Renderbuffers,
Vertex Buffers, Textures and so on. Such objects generally need to be created, setup and filled 
with data. In many cases data needs to be transferred from main memory to the graphics card. 
This is especially true for Vertex Buffers and Textures. A programmer thus has to manage data
on both the graphics card and the main memory. gre abstracts the management of graphics memory
away by allocating the respective GL objects automatically, when they are needed. 

For example when rendering a mesh, vertex data is usually a large buffer of positions, normals and
so on. This data is automatically transferred to the graphics card, if a SceneObject is setup with
1. An Attribute object in its attributes map and 2. it's vertex shader requires the attribute. 
By passing the buffered=true flag on the Attribute object it is indicated that data should not be 
transferred on every draw call, rather it is transferred once and stored on the graphics card.

Following this idea of buffering data on the graphics card, it is desirable to reuse buffers. For different
Scene Objects. It is quite common for example to reuse the same mesh data for multiple objects in a scene.
gre allows that by using the same GREHandle for the given Attribute on multiple Scene Objects. 
 
The same mechanism works for shaders and Textures. 

gre performs reference counting for users of Vertex Buffers and Textures and so on. If the number of users falls
to 0, the object is also automatically cleared from the graphics card.
