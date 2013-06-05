CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

#PROJECT(MayaPlugin)
if(COMMAND cmake_policy)
     cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


IF ( UNIX AND NOT APPLE )
	ADD_DEFINITIONS(-DLINUX)
ENDIF()

SET(CMAKE_CXX_WARNING_LEVEL 4)

#SET (CMAKE_VERBOSE_MAKEFILE TRUE)
FIND_PATH(MAYA_BASE_DIR include/maya/MFn.h PATH 
			"C:/Program Files/Autodesk/Maya2014"
			"C:/Program Files/Autodesk/Maya2013"
			"C:/Program Files (x86)/Autodesk/Maya2013"
			/usr/autodesk/maya2009-x64
			/usr/autodesk/maya2008-x64 
			/usr/autodesk/maya 
			"C:/Program Files/Autodesk/Maya2012"
			"C:/Program Files (x86)/Autodesk/Maya2012"
			"C:/Program Files/Autodesk/Maya2011"
			"C:/Program Files (x86)/Autodesk/Maya2011"
			"C:/Program Files/Autodesk/Maya2010"
			"C:/Program Files (x86)/Autodesk/Maya2010"
			"C:/Program Files/Autodesk/Maya2009" 
			"C:/Program Files/Autodesk/Maya2008"
			"C:/Program Files/Autodesk/Maya8.5"
			"C:/Public/Programs/AutoD/Maya2011"

			)

IF (MAYA_BASE_DIR)
	message ("Maya found at location " ${MAYA_BASE_DIR})

	SET(MAYA_INC_DIR ${MAYA_BASE_DIR}/include)
	SET(MAYA_LIB_DIR ${MAYA_BASE_DIR}/lib)

	SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BULLET_PHYSICS_BINARY_DIR}/lib/Debug/plug-ins)
	SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BULLET_PHYSICS_BINARY_DIR}/lib/Release/plug-ins)

	
	LINK_DIRECTORIES(${MAYA_LIB_DIR})

	ADD_DEFINITIONS(-D_BOOL)
	ADD_DEFINITIONS(-DREQUIRE_IOSTREAM)

	SET(TARGET_H 
	 box_shape.h              bulletExport.h bt_sphere_shape.h         dSolverNode.h         rigid_body_impl.h
		box_shape_impl.h        collision_shape.h         mathUtils.h           rigidBodyNode.h
		bt_box_shape.h          collision_shape_impl.h    mayaUtils.h           solver.h
		bt_collision_shape.h    collisionShapeNode.h      mesh_shape.h          bt_hacd_shape.h solver_impl.h
		bt_convex_hull_shape.h  convex_hull_shape.h       mesh_shape_impl.h     sphere_shape.h
		bt_mesh_shape.h         convex_hull_shape_impl.h  plane_shape.h         sphere_shape_impl.h
		bt_plane_shape.h        dRigidBodyArrayCmd.h      plane_shape_impl.h
		bt_rigid_body.h         dRigidBodyCmd.h           rigidBodyArrayNode.h
		bt_solver.h             dSolverCmd.h              rigid_body.h 		pdbIO.h
		shared_ptr.h		drawUtils.h convex_decomposition_hacd.h	bt_soft_body.h
		dSoftBodyCmd.h	soft_body_impl_t.h	soft_body_t.h	softBodyNode.h
	)

	SET(TARGET_SRC 
		pluginMain.cpp bulletExport.cpp  rigidBodyNode.cpp rigidBodyArrayNode.cpp collisionShapeNode.cpp
		convex_decomposition_hacd.cpp
		  solver.cpp bt_solver.cpp bt_rigid_body.cpp dSolverNode.cpp dSolverCmd.cpp dRigidBodyCmd.cpp dRigidBodyArrayCmd.cpp
		  pdbIO.cpp drawUtils.cpp  constraint/dHingeConstraintCmd.cpp	constraint/dSliderConstraintCmd.cpp
		constraint/hingeConstraintNode.cpp	constraint/sixdofConstraintNode.cpp constraint/dNailConstraintCmd.cpp
		constraint/dsixdofConstraintCmd.cpp	constraint/nailConstraintNode.cpp	constraint/sliderConstraintNode.cpp
		bt_soft_body.cpp	dSoftBodyCmd.cpp	soft_body_t.cpp	softBodyNode.cpp
	)	

	INCLUDE_DIRECTORIES(${MAYA_INC_DIR} ${BULLET_PHYSICS_SOURCE_DIR}/src ${BULLET_PHYSICS_SOURCE_DIR}/Extras/HACD ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin )

	IF (WIN32)
	   SET(MAYA_LIBS OpenMaya Foundation OpenMayaAnim OpenMayaFX OpenMayaUI)
	ELSE()
		SET(MAYA_LIBS OpenMayalib Foundation OpenMayaAnim OpenMayaFX OpenMayaUI)
	ENDIF()

	IF (BUILD_COLLADA)
		ADD_LIBRARY(DynamicaPlugin SHARED ${TARGET_H} colladaExport.h ${TARGET_SRC} colladaExport.cpp )
		TARGET_LINK_LIBRARIES( DynamicaPlugin BulletColladaConverter BulletDynamics BulletCollision LinearMath ColladaDom BulletSoftBody XML HACD ${MAYA_LIBS} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
	ELSE()
		ADD_LIBRARY(DynamicaPlugin SHARED ${TARGET_H} ${TARGET_SRC})
		TARGET_LINK_LIBRARIES(DynamicaPlugin BulletDynamics BulletCollision BulletSoftBody LinearMath HACD ${MAYA_LIBS} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY}) 
	ENDIF()

	IF (WIN32)
		IF (CMAKE_CL_64)
		SET_TARGET_PROPERTIES(DynamicaPlugin PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MACHINE:X64")
		SET(PLUG_NSI_FILE bullet_64bit.nsi)
	   ELSE (CMAKE_CL_64)
		SET_TARGET_PROPERTIES(DynamicaPlugin PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MACHINE:X86")
		SET(PLUG_NSI_FILE bullet_32bit.nsi)
	   ENDIF (CMAKE_CL_64)
	   SET_TARGET_PROPERTIES(DynamicaPlugin PROPERTIES SUFFIX ".mll")
	   LINK_DIRECTORIES(${MAYA_LIB_DIR})
	   SET(CMAKE_SHARED_LINKER_FLAGS "/export:initializePlugin /export:uninitializePlugin")  
	ELSE (WIN32)
	 
	  LINK_DIRECTORIES(${MAYA_LIB_DIR})
	ENDIF(WIN32)

	#OPTION(BUILD_STANDALONE "Build a standalone exporter program")
	#
	#IF (BUILD_STANDALONE)
	#  IF (WIN32)
	#    
	#  ELSE (WIN32)
	#    SET(MAYA_LIBS OpenMayalib Foundation OpenMayaAnim)
	#    ADD_EXECUTABLE(me ${TARGET_H} ${TARGET_SRC} standalone.cpp)
	#    LINK_DIRECTORIES(${MAYA_LIB_DIR})
	#    TARGET_LINK_LIBRARIES(me ${MAYA_LIBS} pthread m imf)
	#  ENDIF (WIN32)      
	#ENDIF (BUILD_STANDALONE)



IF (WIN32 AND NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
	ADD_CUSTOM_COMMAND(
		TARGET DynamicaPlugin 
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/${PLUG_NSI_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${PLUG_NSI_FILE}
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/BulletDynamica.6_module ${CMAKE_CURRENT_BINARY_DIR}

		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/AEdCollisionShapeTemplate.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/AEdCollisionShapeTemplate.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/AEdHingeConstraintTemplate.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/AEdHingeConstraintTemplate.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/AEdNailConstraintTemplate.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/AEdNailConstraintTemplate.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/AEdRigidBodyArrayTemplate.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/AEdRigidBodyArrayTemplate.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/AEdRigidBodyTemplate.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/AEdRigidBodyTemplate.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/AEdSixdofConstraintTemplate.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/AEdSixdofConstraintTemplate.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/AEdSliderConstraintTemplate.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/AEdSliderConstraintTemplate.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/AEdSolverTemplate.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/AEdSolverTemplate.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/dynamica.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/dynamica.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/dynamicaUI.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/dynamicaUI.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/dynamicaUITools.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/dynamicaUITools.mel
		COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts/mbUtils.mel ${CMAKE_CURRENT_BINARY_DIR}/scripts/mbUtils.mel
		
	)

#	file(COPY ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scripts DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
	file(COPY ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/icons DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
	file(COPY ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/scenes DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
	file(COPY ${BULLET_PHYSICS_SOURCE_DIR}/Extras/MayaPlugin/doc DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

	
ENDIF (WIN32 AND NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)

ELSE()
	MESSAGE("Maya not found, try adding your Maya path in Bullet/Extras/MayaPlugin/CMakeListst.txt under FIND_PATH(MAYA_BASE_DIR ...")
ENDIF()

