import platform

vars = Variables()
# build target
vars.Add(EnumVariable('target', 'Building target', 'debug', allowed_values=('debug', 'profile', 'release')))
# float precision
vars.Add(EnumVariable('float', 'Building using float precision (32 or 64)', '32', allowed_values=('32', '64')))

# Modules
vars.Add(EnumVariable('modules', 'Building all modules', 'yes', allowed_values=('yes', 'no')))
# cgltf
vars.Add(EnumVariable('cgltf', 'Building cgltf module', 'yes', allowed_values=('yes', 'no')))
# nuklear
vars.Add(EnumVariable('nuklear', 'Building nuklear module', 'yes', allowed_values=('yes', 'no')))
# bullet
vars.Add(EnumVariable('bullet', 'Building bullet module', 'yes', allowed_values=('yes', 'no')))

env = Environment(variables = vars)
Help(vars.GenerateHelpText(env))

system = platform.system()

src_files = Glob('src/*.c') + Glob('src/*.cc')
INCLUDE = ["include"]
name = "SDL-OpenGL-Project"
bin = "build/bin/" + name
LIBS = ['SDL2main','SDL2']
LIBPATH = []
LINKFLAGS = []
CCFLAGS = []
CPPDEFINES = []

print(f"*** {env['PLATFORM']} ***")

cc = env['CC']
if cc == 'cl':
	pass
elif cc == 'gcc':
	pass

# windows utils
def find_visual_c_batch_file(env):
    from SCons.Tool.MSCommon.vc import get_default_version, get_host_target, find_batch_file

    version = get_default_version(env)
    (host_platform, target_platform, _) = get_host_target(env)
    return find_batch_file(env, version, host_platform, target_platform)[0]

def build_commandline(commands, num_jobs):
    common_build_prefix = [
        'cmd /V /C set "plat=$(PlatformTarget)"',
        '(if "$(PlatformTarget)"=="x64" (set "plat=x86_amd64"))',
        '(if "$(Configuration)"=="release" (set "tools=no"))',
        'call "' + batch_file + '" !plat!',
    ]

    # Windows allows us to have spaces in paths, so we need
    # to double quote off the directory. However, the path ends
    # in a backslash, so we need to remove this, lest it escape the
    # last double quote off, confusing MSBuild
    common_build_postfix = [
        "--directory=\"$(ProjectDir.TrimEnd('\\'))\"",
        "platform=windows",
        "target=$(Configuration)",
        "-j%s" % num_jobs,
    ]

    result = " ^& ".join(common_build_prefix + [" ".join([commands] + common_build_postfix)])
    return result

# bullet utils
def get_bullet_source():
    thirdparty_dir = "modules/bullet/"

    bullet2_src = [
    	# Bullet3Common
    	"Bullet3Common/b3AlignedAllocator.cpp",
    	"Bullet3Common/b3Vector3.cpp",
    	"Bullet3Common/b3Logging.cpp",
        # BulletCollision
        "BulletCollision/BroadphaseCollision/btAxisSweep3.cpp",
        "BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp",
        "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp",
        "BulletCollision/BroadphaseCollision/btDbvt.cpp",
        "BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp",
        "BulletCollision/BroadphaseCollision/btDispatcher.cpp",
        "BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp",
        "BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp",
        "BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp",
        "BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp",
        "BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp",
        "BulletCollision/CollisionDispatch/btCollisionDispatcherMt.cpp",
        "BulletCollision/CollisionDispatch/btCollisionObject.cpp",
        "BulletCollision/CollisionDispatch/btCollisionWorld.cpp",
        "BulletCollision/CollisionDispatch/btCollisionWorldImporter.cpp",
        "BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btCompoundCompoundCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp",
        "BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btGhostObject.cpp",
        "BulletCollision/CollisionDispatch/btHashedSimplePairCache.cpp",
        "BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp",
        "BulletCollision/CollisionDispatch/btManifoldResult.cpp",
        "BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp",
        "BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp",
        "BulletCollision/CollisionDispatch/btUnionFind.cpp",
        "BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp",
        "BulletCollision/CollisionShapes/btBoxShape.cpp",
        "BulletCollision/CollisionShapes/btBox2dShape.cpp",
        "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp",
        "BulletCollision/CollisionShapes/btCapsuleShape.cpp",
        "BulletCollision/CollisionShapes/btCollisionShape.cpp",
        "BulletCollision/CollisionShapes/btCompoundShape.cpp",
        "BulletCollision/CollisionShapes/btConcaveShape.cpp",
        "BulletCollision/CollisionShapes/btConeShape.cpp",
        "BulletCollision/CollisionShapes/btConvexHullShape.cpp",
        "BulletCollision/CollisionShapes/btConvexInternalShape.cpp",
        "BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp",
        "BulletCollision/CollisionShapes/btConvexPolyhedron.cpp",
        "BulletCollision/CollisionShapes/btConvexShape.cpp",
        "BulletCollision/CollisionShapes/btConvex2dShape.cpp",
        "BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp",
        "BulletCollision/CollisionShapes/btCylinderShape.cpp",
        "BulletCollision/CollisionShapes/btEmptyShape.cpp",
        "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp",
        "BulletCollision/CollisionShapes/btMiniSDF.cpp",
        "BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp",
        "BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp",
        "BulletCollision/CollisionShapes/btMultiSphereShape.cpp",
        "BulletCollision/CollisionShapes/btOptimizedBvh.cpp",
        "BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp",
        "BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp",
        "BulletCollision/CollisionShapes/btSdfCollisionShape.cpp",
        "BulletCollision/CollisionShapes/btShapeHull.cpp",
        "BulletCollision/CollisionShapes/btSphereShape.cpp",
        "BulletCollision/CollisionShapes/btStaticPlaneShape.cpp",
        "BulletCollision/CollisionShapes/btStridingMeshInterface.cpp",
        "BulletCollision/CollisionShapes/btTetrahedronShape.cpp",
        "BulletCollision/CollisionShapes/btTriangleBuffer.cpp",
        "BulletCollision/CollisionShapes/btTriangleCallback.cpp",
        "BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp",
        "BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp",
        "BulletCollision/CollisionShapes/btTriangleMesh.cpp",
        "BulletCollision/CollisionShapes/btTriangleMeshShape.cpp",
        "BulletCollision/CollisionShapes/btUniformScalingShape.cpp",
        "BulletCollision/Gimpact/btContactProcessing.cpp",
        "BulletCollision/Gimpact/btGenericPoolAllocator.cpp",
        "BulletCollision/Gimpact/btGImpactBvh.cpp",
        "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp",
        "BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp",
        "BulletCollision/Gimpact/btGImpactShape.cpp",
        "BulletCollision/Gimpact/btTriangleShapeEx.cpp",
        "BulletCollision/Gimpact/gim_box_set.cpp",
        "BulletCollision/Gimpact/gim_contact.cpp",
        "BulletCollision/Gimpact/gim_memory.cpp",
        "BulletCollision/Gimpact/gim_tri_collision.cpp",
        "BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp",
        "BulletCollision/NarrowPhaseCollision/btConvexCast.cpp",
        "BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp",
        "BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp",
        "BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp",
        "BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp",
        "BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp",
        "BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp",
        "BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp",
        "BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp",
        "BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp",
        "BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp",
        # BulletDynamics
        "BulletDynamics/Character/btKinematicCharacterController.cpp",
        "BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btContactConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btFixedConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btGearConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btGeneric6DofSpring2Constraint.cpp",
        "BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp",
        "BulletDynamics/ConstraintSolver/btHingeConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp",
        "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.cpp",
        "BulletDynamics/ConstraintSolver/btBatchedConstraints.cpp",
        "BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.cpp",
        "BulletDynamics/ConstraintSolver/btSliderConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btTypedConstraint.cpp",
        "BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp",
        "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp",
        "BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.cpp",
        "BulletDynamics/Dynamics/btSimulationIslandManagerMt.cpp",
        "BulletDynamics/Dynamics/btRigidBody.cpp",
        "BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp",
        # "BulletDynamics/Dynamics/Bullet-C-API.cpp",
        "BulletDynamics/Vehicle/btRaycastVehicle.cpp",
        "BulletDynamics/Vehicle/btWheelInfo.cpp",
        "BulletDynamics/Featherstone/btMultiBody.cpp",
        "BulletDynamics/Featherstone/btMultiBodyConstraint.cpp",
        "BulletDynamics/Featherstone/btMultiBodyConstraintSolver.cpp",
        "BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.cpp",
        "BulletDynamics/Featherstone/btMultiBodyFixedConstraint.cpp",
        "BulletDynamics/Featherstone/btMultiBodyGearConstraint.cpp",
        "BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.cpp",
        "BulletDynamics/Featherstone/btMultiBodyJointMotor.cpp",
        "BulletDynamics/Featherstone/btMultiBodyMLCPConstraintSolver.cpp",
        "BulletDynamics/Featherstone/btMultiBodyPoint2Point.cpp",
        "BulletDynamics/Featherstone/btMultiBodySliderConstraint.cpp",
        "BulletDynamics/Featherstone/btMultiBodySphericalJointMotor.cpp",
        "BulletDynamics/MLCPSolvers/btDantzigLCP.cpp",
        "BulletDynamics/MLCPSolvers/btMLCPSolver.cpp",
        "BulletDynamics/MLCPSolvers/btLemkeAlgorithm.cpp",
        # BulletInverseDynamics
        "BulletInverseDynamics/IDMath.cpp",
        "BulletInverseDynamics/MultiBodyTree.cpp",
        "BulletInverseDynamics/details/MultiBodyTreeInitCache.cpp",
        "BulletInverseDynamics/details/MultiBodyTreeImpl.cpp",
        # BulletSoftBody
        "BulletSoftBody/btSoftBody.cpp",
        "BulletSoftBody/btSoftBodyConcaveCollisionAlgorithm.cpp",
        "BulletSoftBody/btSoftBodyHelpers.cpp",
        "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.cpp",
        "BulletSoftBody/btSoftRigidCollisionAlgorithm.cpp",
        "BulletSoftBody/btSoftRigidDynamicsWorld.cpp",
        "BulletSoftBody/btSoftMultiBodyDynamicsWorld.cpp",
        "BulletSoftBody/btSoftSoftCollisionAlgorithm.cpp",
        "BulletSoftBody/btDefaultSoftBodySolver.cpp",
        "BulletSoftBody/btDeformableBackwardEulerObjective.cpp",
        "BulletSoftBody/btDeformableBodySolver.cpp",
        "BulletSoftBody/btDeformableMultiBodyConstraintSolver.cpp",
        "BulletSoftBody/btDeformableContactProjection.cpp",
        "BulletSoftBody/btDeformableMultiBodyDynamicsWorld.cpp",
        "BulletSoftBody/btDeformableContactConstraint.cpp",
        "BulletSoftBody/poly34.cpp",
        # clew
        "clew/clew.c",
        # LinearMath
        "LinearMath/btAlignedAllocator.cpp",
        "LinearMath/btConvexHull.cpp",
        "LinearMath/btConvexHullComputer.cpp",
        "LinearMath/btGeometryUtil.cpp",
        "LinearMath/btPolarDecomposition.cpp",
        "LinearMath/btQuickprof.cpp",
        "LinearMath/btSerializer.cpp",
        "LinearMath/btSerializer64.cpp",
        "LinearMath/btThreads.cpp",
        "LinearMath/btVector3.cpp",
        "LinearMath/TaskScheduler/btTaskScheduler.cpp",
        "LinearMath/TaskScheduler/btThreadSupportPosix.cpp",
        "LinearMath/TaskScheduler/btThreadSupportWin32.cpp",
        "LinearMath/btReducedVector.cpp",
    ]

    thirdparty_sources = [thirdparty_dir + file for file in bullet2_src]
    return thirdparty_sources

if env['PLATFORM'] == 'win32':
	print('Windows Build')
	
	LIBS += [
		'opengl32',
		'kernel32',
		'ucrtd', # d suffix is debug
		'advapi32',
        'comdlg32',
        'gdi32',
        'odbc32',
        'odbccp32',
        'ole32',
        'oleaut32',
        'shell32',
        'user32',
        'uuid',
        'winspool',
        'vcruntime',
        'msvcrtd' # d suffix is debug
    ]

	INCLUDE += [
		'/Users/Riordan/scoop/apps/sdl2/current/include'
	]
	
	LIBPATH += [
		'/Users/Riordan/scoop/apps/sdl2/current/lib'
	]
	
	LINKFLAGS += [
		'/ENTRY:main',
		'/DEBUG',
		'/WX',
		#'/VERBOSE:LIB',
		'/NOLOGO',
		'/INCREMENTAL',
		'/MACHINE:X64',	#/MACHINE:{ARM|EBC|X64|X86}
		'/NXCOMPAT',
		'/DYNAMICBASE',
		'/SUBSYSTEM:WINDOWS', #{BOOT_APPLICATION|CONSOLE|EFI_APPLICATION|EFI_BOOT_SERVICE_DRIVER|EFI_ROM|EFI_RUNTIME_DRIVER|NATIVE|POSIX|WINDOWS}
		'/NODEFAULTLIB',
	]

	CPPDEFINES = [
		'_DEBUG',
		'WIN32',
		'CURL_STATICLIB',
		'SDL_MAIN_HANDLED'
	]

	CCFLAGS = [
		'/FC',
		'/W1',
		'/std:c++17',
		#'/RTC1',
		'/Od',
		#'/sdl',
		'/MDd',
		'/GS-' # disables /sdl
		#'/MTd' # WARNING: d at the end means debug version
	]

	# Modules
	if env['modules'] == 'yes':
		CPPDEFINES += [
			'__MODULES__'
		]
		if env['cgltf'] == 'yes':
			CPPDEFINES += [
				'__CGLTF__'
			]
			INCLUDE += [
				'modules/cgltf/'
			]
		if env['nuklear'] == 'yes':
			CPPDEFINES += [
				'__NUKLEAR__',
			]
			INCLUDE += [
				'modules/nuklear/'
			]

	env.Append(LINKFLAGS = LINKFLAGS)
	env.Append(LIBS = LIBS)
	env.Append(LIBPATH = LIBPATH)
	env.Append(CPPPATH = INCLUDE)
	env.Append(CCFLAGS = CCFLAGS)
	env.Append(CPPFLAGS = []) # Same as CCFLAGS on windows
	env.Append(CXXFLAGS = ['/DEBUG'])
	env.Append(CPPDEFINES = CPPDEFINES)

	# MSVS Build commands
	batch_file = find_visual_c_batch_file(env)

	env["MSVSBUILDCOM"] = build_commandline("scons", 1)
	env["MSVSREBUILDCOM"] = build_commandline("scons", 1)
	env["MSVSCLEANCOM"] = build_commandline("scons --clean", 1)
	env['CCPDBFLAGS'] = '/Zi /Fd${TARGET}.pdb'

	project = env.Program(target=bin, source=src_files, LIBS=LIBS, LIBPATH=LIBPATH, LINKFLAGS=LINKFLAGS)
	buildtarget = [s for s in project if str(s).endswith('exe')]
	platform = ['|x64', '|x86', '|Win32']

	env.MSVSProject(target = name + env['MSVSPROJECTSUFFIX'],
	                srcs = [str(file) for file in src_files],
	                #cppdefines=[],
	                #cppflags=[],
	                #cpppaths=[],
	                incs = INCLUDE,
	                #localincs = None,
	                #resources = None,
	                #misc = None,
	                buildtarget = project,
	                variant = env['target'] + "|Win32")
elif env['PLATFORM'] == 'posix':
	print('POSIX Build')

	env['CC'] = env['CXX']

	# Linux only
	if env['target'] == 'debug':
	    env.Append(CCFLAGS = ['-Wall', '-g', '-O0', '-DDEBUG'])
	elif env['target'] == 'release':
	    env.Append(CCFLAGS = ['-Wall', '-O3', '-DNDEBUG'])
	    env.Append(LINKFLAGS = ['-s'])
	elif env['target'] == 'profile':
	    env.Append(CCFLAGS = ['-Wall', '-pg', '-O0', '-DNDEBUG'])

	# Core for POSIX
	INCLUDE += [
		'/usr/include/SDL2/'
	]
	
	LIBPATH += [
		'/usr/lib/x86_64-linux-gnu/'
	]

	LIBS += [
		'dl',
		'GL'
	]

	CPPDEFINES += [
		'__POSIX__'
	]

	# Modules
	if env['modules'] == 'yes':
		CPPDEFINES += [
			'__MODULES__'
		]
		if env['cgltf'] == 'yes':
			CPPDEFINES += [
				'__CGLTF__'
			]
			INCLUDE += [
				'modules/cgltf/'
			]
		if env['nuklear'] == 'yes':
			CPPDEFINES += [
				'__NUKLEAR__',
			]

			INCLUDE += [
				'modules/nuklear/'
			]
		if env['bullet'] == 'yes':
			CPPDEFINES += [
				'__BULLET__',
				'BT_USE_OLD_DAMPING_METHOD',
			]
			if env['float'] == '64': CPPDEFINES += ['BT_USE_DOUBLE_PRECISION=1']
			INCLUDE += [
				'modules/bullet/',
				#'modules/bullet/Bullet3Collision',
				#'modules/bullet/Bullet3Collision/BroadPhaseCollision',
				#'modules/bullet/Bullet3Collision/BroadPhaseCollision/shared',
				#'modules/bullet/Bullet3Collision/NarrowPhaseCollision',
				#'modules/bullet/Bullet3Collision/NarrowPhaseCollision/shared',
				'modules/bullet/Bullet3Common',
				'modules/bullet/Bullet3Common/shared',
				#'modules/bullet/Bullet3Dynamics',
				#'modules/bullet/Bullet3Dynamics/ConstraintSolver',
				#'modules/bullet/Bullet3Dynamics/shared',
				#'modules/bullet/Bullet3Geometry',
				#'modules/bullet/Bullet3OpenCL',
				#'modules/bullet/Bullet3OpenCL/BroadphaseCollision',
				#'modules/bullet/Bullet3OpenCL/BroadphaseCollision/kernels',
				#'modules/bullet/Bullet3OpenCL/Initialize',
				#'modules/bullet/Bullet3OpenCL/NarrowphaseCollision',
				#'modules/bullet/Bullet3OpenCL/NarrowphaseCollision/kernels',
				#'modules/bullet/Bullet3OpenCL/ParallelPrimitives',
				#'modules/bullet/Bullet3OpenCL/ParallelPrimitives/kernels',
				#'modules/bullet/Bullet3OpenCL/Raycast',
				#'modules/bullet/Bullet3OpenCL/Raycast/kernels',
				#'modules/bullet/Bullet3OpenCL/RigidBody',
				#'modules/bullet/Bullet3OpenCL/RigidBody/kernels',
				#'modules/bullet/Bullet3Serialize',
				#'modules/bullet/Bullet3Serialize/Bullet2FileLoader',
				#'modules/bullet/Bullet3Serialize/Bullet2FileLoader/autogenerated',
				'modules/bullet/BulletCollision',
				'modules/bullet/BulletCollision/BroadphaseCollision',
				'modules/bullet/BulletCollision/CollisionDispatch',
				'modules/bullet/BulletCollision/CollisionShapes',
				'modules/bullet/BulletCollision/Gimpact',
				'modules/bullet/BulletCollision/NarrowPhaseCollision',
				'modules/bullet/BulletDynamics',
				'modules/bullet/BulletDynamics/Character',
				'modules/bullet/BulletDynamics/ConstraintSolver',
				'modules/bullet/BulletDynamics/Dynamics',
				'modules/bullet/BulletDynamics/Featherstone',
				'modules/bullet/BulletDynamics/MLCPSolvers',
				'modules/bullet/BulletDynamics/Vehicle',
				'modules/bullet/BulletInverseDynamics',
				'modules/bullet/BulletInverseDynamics/details',
				'modules/bullet/BulletSoftBody',
				'modules/bullet/clew',
				'modules/bullet/LinearMath',
				'modules/bullet/LinearMath/TaskScheduler',
			]
			src_files += get_bullet_source()

	env.Append(LINKFLAGS = LINKFLAGS)
	env.Append(LIBS = LIBS)
	env.Append(LIBPATH = LIBPATH)
	env.Append(CPPPATH = INCLUDE)
	env.Append(CCFLAGS = CCFLAGS)
	env.Append(CPPFLAGS = []) # Same as CCFLAGS on windows
	env.Append(CXXFLAGS = ['-std=c++11'])
	env.Append(CPPDEFINES = CPPDEFINES)

	project = env.Program(
		target=bin, 
		source=src_files, 
		CPPPATH=INCLUDE, 
		LIBS=LIBS, 
		LIBPATH=LIBPATH, 
		LINKFLAGS=LINKFLAGS)

	#env.Install('/usr/bin', [project])
	#env.Alias('install', '/usr/bin')
else:
	print(f"Platform: {env['PLATFORM']} does not have a supported build script")


