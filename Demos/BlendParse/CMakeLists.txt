# This is basically the overall name of the project in Visual Studio this is the name of the Solution File


# For every executable you have with a main method you should have an add_executable line below.
# For every add executable line you should list every .cpp and .h file you have associated with that executable.  


# This is the variable for Windows.  I use this to define the root of my directory structure.
SET(GLUT_ROOT ${BULLET_PHYSICS_SOURCE_DIR}/Glut)

# You shouldn't have to modify anything below this line 
########################################################

INCLUDE_DIRECTORIES(
${BULLET_PHYSICS_SOURCE_DIR}/src  ${BULLET_PHYSICS_SOURCE_DIR}/Demos/OpenGL }
${BULLET_PHYSICS_SOURCE_DIR}/Extras/Serialize/BulletFileLoader
${BULLET_PHYSICS_SOURCE_DIR}/Extras/Serialize/BlenderSerialize
)

LINK_LIBRARIES(
OpenGLSupport  BlenderSerialize BulletFileLoader BulletDynamics  BulletCollision LinearMath    ${GLUT_glut_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY}
)

OPTION(USE_GRAPHICS_OBJECTS "Blender import with textured graphics objects" ON)

IF(USE_GRAPHICS_OBJECTS)
	ADD_DEFINITIONS(-DUSE_GRAPHICS_OBJECTS)
	INCLUDE_DIRECTORIES(
		${BULLET_PHYSICS_SOURCE_DIR}/Extras
	)
	LINK_LIBRARIES( jpeg)
ENDIF()

ADD_EXECUTABLE(AppBlendParse
	main.cpp
	BasicDemo.cpp 
	BasicDemo.h
	abs-file.h
	blendtype.h
	BulletBlendReaderNew.cpp
	BulletBlendReaderNew.h
	readblend.c
	readblend.h
)



IF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
	ADD_CUSTOM_COMMAND(
		TARGET AppBlendParse
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Demos/BlendParse/PhysicsAnimationBakingDemo.blend ${CMAKE_CURRENT_BINARY_DIR}/PhysicsAnimationBakingDemo.blend
	)
ENDIF ()