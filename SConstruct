import platform

vars = Variables()
vars.Add(EnumVariable('mode', 'Building mode', 'debug', allowed_values=('debug', 'profile', 'release')))
env = Environment(variables = vars)
Help(vars.GenerateHelpText(env))

system = platform.system()

src_files = Glob('src/*.c')
include = ["include", '/Users/Riordan/scoop/apps/sdl2/2.0.10/include']
bin = "build/bin/SDL-OpenGL-Project"
libs = ['SDL2main','SDL2']
lib_path = ['/Users/Riordan/scoop/apps/sdl2/2.0.10/lib']

if system == 'Windows':
	print('Windows Build')
	libs += ['opengl32']

# Linux only
#if env['mode'] == 'debug':
#    env.Append(CCFLAGS = ['-Wall', '-g', '-O0', '-DDEBUG'])
#elif env['mode'] == 'release':
#    env.Append(CCFLAGS = ['-Wall', '-O3', '-DNDEBUG'])
#    env.Append(LINKFLAGS = ['-s'])
#elif env['mode'] == 'profile':
#    env.Append(CCFLAGS = ['-Wall', '-pg', '-O0', '-DNDEBUG'])



project = env.Program(target=bin, source=src_files, CPPPATH=include, LIBS=libs, LIBPATH=lib_path)

#env.Install('/usr/bin', [project])
#env.Alias('install', '/usr/bin')