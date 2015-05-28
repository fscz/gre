gre is a lightweight 3d capable opengl es 2.0 render engine for the 
Raspberry Pi.
 
 
It abstracts away many details of the render pipeline by providing
a single concise api file <gre.h>. In particular you never have to
call a gl-function, create or delete framebuffers, vertex buffers 
and so on. 
 
 
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
