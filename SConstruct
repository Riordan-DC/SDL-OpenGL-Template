import platform

vars = Variables()
vars.Add(EnumVariable('mode', 'Building mode', 'debug', allowed_values=('debug', 'profile', 'release')))
env = Environment(variables = vars)
Help(vars.GenerateHelpText(env))

system = platform.system()

src_files = Glob('src/*.c')
include = ["include"]
name = "SDL-OpenGL-Project"
bin = "build/bin/" + name
libs = ['SDL2main','SDL2']
lib_path = []
link_flags = []

cc = env['CC']
if cc == 'cl':
	pass
elif cc == 'gcc':
	pass

def find_visual_c_batch_file(env):
    from SCons.Tool.MSCommon.vc import get_default_version, get_host_target, find_batch_file

    version = get_default_version(env)
    (host_platform, target_platform, _) = get_host_target(env)
    return find_batch_file(env, version, host_platform, target_platform)[0]

if system == 'Windows':
	print('Windows Build')
	libs += ['opengl32', 'Kernel32', 'libucrt']
	include += ['/Users/Riordan/scoop/apps/sdl2/current/include']
	lib_path += ['/Users/Riordan/scoop/apps/sdl2/current/lib']
	link_flags += ['/ENTRY:main', '/debug'] #'/VERBOSE:LIB'

	env.Append(LINKFLAGS = link_flags)
	env.Append(LIBS = libs)
	env.Append(LIBPATH = lib_path)
	env.Append(CPPPATH = include)
	env.Append(CCFLAGS = ['/FC'])
	env.Append(CPPFLAGS = ['/Od'])
	env.Append(CXXFLAGS = ['/DEBUG'])

	#print(env.Dump())
	# MSVS Build commands
	batch_file = find_visual_c_batch_file(env)
	def build_commandline(commands, num_jobs):
	    common_build_prefix = [
	        'cmd /V /C set "plat=$(PlatformTarget)"',
	        '(if "$(PlatformTarget)"=="x64" (set "plat=x86_amd64"))',
	        #'set "tools=%s"' % env["tools"],
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
	        "progress=no",
	        #"tools=!tools!",
	        "-j%s" % num_jobs,
	    ]

	    #if env["tests"]:
	    #    common_build_postfix.append("tests=yes")

	    #if env["custom_modules"]:
	    #    common_build_postfix.append("custom_modules=%s" % env["custom_modules"])

	    result = " ^& ".join(common_build_prefix + [" ".join([commands] + common_build_postfix)])
	    return result

	env["MSVSBUILDCOM"] = build_commandline("scons", 1)
	env["MSVSREBUILDCOM"] = build_commandline("scons", 1)
	env["MSVSCLEANCOM"] = build_commandline("scons --clean", 1)

	project = env.Program(target=bin, source=src_files, LIBS=libs, LIBPATH=lib_path, LINKFLAGS=link_flags)
	buildtarget = [s for s in project if str(s).endswith('exe')]
	platform = ['|x64', 'Win32']

	env.MSVSProject(target = name + env['MSVSPROJECTSUFFIX'],
	                srcs = ['src/main.c'],
	                #cppdefines=[],
	                #cppflags=[],
	                #cpppaths=[],
	                incs = include,
	                #localincs = None,
	                #resources = None,
	                #misc = None,
	                buildtarget = project,
	                variant = env['mode'] + "|x64")
else:
	# Linux only
	#if env['mode'] == 'debug':
	#    env.Append(CCFLAGS = ['-Wall', '-g', '-O0', '-DDEBUG'])
	#elif env['mode'] == 'release':
	#    env.Append(CCFLAGS = ['-Wall', '-O3', '-DNDEBUG'])
	#    env.Append(LINKFLAGS = ['-s'])
	#elif env['mode'] == 'profile':
	#    env.Append(CCFLAGS = ['-Wall', '-pg', '-O0', '-DNDEBUG'])



	project = env.Program(target=bin, source=src_files, CPPPATH=include, LIBS=libs, LIBPATH=lib_path, LINKFLAGS=link_flags)

	#env.Install('/usr/bin', [project])
	#env.Alias('install', '/usr/bin')