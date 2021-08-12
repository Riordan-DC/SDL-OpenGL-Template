import platform

vars = Variables()
vars.Add(EnumVariable('target', 'Building target', 'debug', allowed_values=('debug', 'profile', 'release')))
env = Environment(variables = vars)
Help(vars.GenerateHelpText(env))

system = platform.system()

src_files = Glob('src/*.c')
INCLUDE = ["include"]
name = "SDL-OpenGL-Project"
bin = "build/bin/" + name
LIBS = ['SDL2main','SDL2']
LIBPATH = []
LINKFLAGS = []
CCFLAGS = []
CPPDEFINES = []

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
	                srcs = ['src/main.c', 'src/glad.c'],
	                #cppdefines=[],
	                #cppflags=[],
	                #cpppaths=[],
	                incs = INCLUDE,
	                #localincs = None,
	                #resources = None,
	                #misc = None,
	                buildtarget = project,
	                variant = env['target'] + "|Win32")
else:
	# Linux only
	#if env['target'] == 'debug':
	#    env.Append(CCFLAGS = ['-Wall', '-g', '-O0', '-DDEBUG'])
	#elif env['target'] == 'release':
	#    env.Append(CCFLAGS = ['-Wall', '-O3', '-DNDEBUG'])
	#    env.Append(LINKFLAGS = ['-s'])
	#elif env['target'] == 'profile':
	#    env.Append(CCFLAGS = ['-Wall', '-pg', '-O0', '-DNDEBUG'])

	project = env.Program(target=bin, source=src_files, CPPPATH=INCLUDE, LIBS=LIBS, LIBPATH=LIBPATH, LINKFLAGS=LINKFLAGS)

	#env.Install('/usr/bin', [project])
	#env.Alias('install', '/usr/bin')