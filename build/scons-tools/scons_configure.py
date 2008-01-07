import os
import string

################################################################################

def write_configh_header():
	config_h_file=open('src/config.h', "w")
	config_h_file.write("""
// This file is autogenerated. It will be overwritten by the next run of scons

#ifndef CONFIG_H
#define CONFIG_H

""")
	return config_h_file

################################################################################

# TODO: split this up and write the data where it originates
def write_configh(config_h_file, env):
	if os.path.isabs(env['bindir']):
		bindir=env['bindir']
	else:
		bindir=os.path.join(env['install_prefix'], env['bindir'])

	if os.path.isabs(env['datadir']):
		datadir=env['datadir']
	else:
		datadir=os.path.join(env['install_prefix'], env['datadir'])

	if os.path.isabs(env['localedir']):
		localedir=env['localedir']
	else:
		localedir=os.path.join(env['install_prefix'], env['localedir'])

	if env['build'] != 'release' and env['prefer_localdata']:
		config_h_file.write("#define INSTALL_PREFIX \".\"\n")
		config_h_file.write("#define INSTALL_BINDIR \".\"\n")
		config_h_file.write("#define INSTALL_DATADIR \".\"\n")
		config_h_file.write("#define INSTALL_LOCALEDIR \"locale\"\n\n")
	else:
		config_h_file.write("#define INSTALL_PREFIX \""+env['install_prefix']+"\"\n")
		config_h_file.write("#define INSTALL_BINDIR \""+env['bindir']+"\"\n")
		config_h_file.write("#define INSTALL_DATADIR \""+env['datadir']+"\"\n")
		config_h_file.write("#define INSTALL_LOCALEDIR \""+env['localedir']+"\"\n\n")

def write_configh_footer(config_h_file):
	config_h_file.write("\n#endif\n")
	config_h_file.close()

################################################################################

def write_buildid(build_id):
	build_id_file=open('src/build_id.h', "w")

	build_id_file.write("""
#ifndef BUILD_ID_H
#define BUILD_ID_H

#define BUILD_ID \""""+build_id+"""\"

#endif
""")
	build_id_file.close()

################################################################################

def parse_cli(env):
	if env['enable_ggz']:
		env.Append(CCFLAGS='-DUSE_GGZ')
		env.Append(LIBS=['ggzmod', 'ggzcore', 'ggz'])

	#if env['cross']:
	#	print 'Cross-compiling does not work yet!'
	#	env.Exit(1)
	#	#TARGET='i586-mingw32msvc'
	#	#PREFIX='/usr/local/cross-tools'
	#	#env['ENV']['PATH']=PREFIX+'/'+TARGET+'/bin:'+PREFIX+'/bin'+env['ENV']['PATH']
	#	#env['CXX']=TARGET+'-g++'
	#	### manually overwrite
	#	###env['sdlconfig']=PREFIX+'/bin/'+TARGET+'-sdl_config'
	#	#env['sdlconfig']=PREFIX+'/'+TARGET+'/bin/'+TARGET+'-sdl-config'
	#else:
	#	TARGET='native'
	TARGET='native'

	return TARGET

################################################################################

def CheckPKG(context, name):
	context.Message( 'Checking for %s... ' % name )
	ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
	context.Result( ret )
	return ret

def CheckPKGConfig(context, version):
	context.Message( 'Checking for pkg-config... ' )
	ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
	context.Result( ret )
	return ret

def CheckSDLConfig(context, env):
	context.Message( 'Checking for sdl-config... ' )
	for p in env['PATH']:
		ret = context.TryAction(os.path.join(p, env['sdlconfig'])+' --version')[0]
		if ret==1:
			env['sdlconfig']=os.path.join(p, env['sdlconfig'])
			context.Result( ret )
			break
	return ret

def CheckParaguiConfig(context, env):
	context.Message( 'Checking for paragui-config... ' )
	for p in env['PATH']:
		ret = context.TryAction(os.path.join(p, env['paraguiconfig'])+' --version')[0]
		if ret==1:
			env['paraguiconfig']=os.path.join(p, env['paraguiconfig'])
			context.Result( ret )
			break
	return ret

def CheckSDLVersionAtLeast(context, major, minor, micro, env):
	context.Message( 'Checking SDL version >= %s ... ' % (repr(major)+'.'+repr(minor)+'.'+repr(micro)))
	version=os.popen(env['sdlconfig']+" --version", "r").read()
	(maj, min, mic)=version.split('.')
	if int(maj)>=int(major) and int(min)>=int(minor) and int(mic)>=int(micro):
		ret=1
	else: ret=0
	context.Result( ret )
	return ret

def CheckCompilerAttribute(context, compiler_attribute, env):
	context.Message( 'Trying whether __attribute__((%s)) works on your compiler ... ' % compiler_attribute)

	ret=context.TryLink("""class test {
	__attribute__(("""+compiler_attribute+"""))
	   int test_func(const char* fmt, ...) {}
};

int main(int argc, char **argv){return argc==0 && argv==0;}
\n\n""", '.cc') #must _use_ both arguments, otherwise -Werror will break

	context.Result(ret)
	return ret

def CheckCompilerFlag(context, compiler_flag, env):
	context.Message( 'Trying to enable compiler flag %s ... ' % compiler_flag)
	lastCCFLAGS = context.env['CCFLAGS'][:]
	context.env.Append(CCFLAGS = compiler_flag)
	ret = context.TryLink("""int main(int argc, char **argv) {return argc==0 && argv==0;}
			\n""", ".cc") #must _use_ both arguments, otherwise -Werror will break
	if not ret:
		context.env.Replace(CCFLAGS = lastCCFLAGS)
	context.Result( ret )
	return

def CheckLinkerFlag(context, link_flag, env):
	context.Message( 'Trying to enable linker   flag %s ... ' % link_flag)
	lastLINKFLAGS = context.env['LINKFLAGS'][:]
	context.env.Append(LINKFLAGS = link_flag)
	ret = context.TryLink("""int main(int argc, char **argv) {return argc==0 && argv==0;}
			\n""", ".cc") #must _use_ both arguments, otherwise -Werror will break
	if not ret:
		context.env.Replace(LINKFLAGS = lastLINKFLAGS)
	context.Result( ret )
	return

################################################################################

def do_configure(config_h_file, conf, env):
	print #prettyprinting

	print "Checking for a working C++ compiler ...",
	if not conf.TryLink("""class c{}; int main(){class c the_class;}""", '.cc'):
		print "no"
		print "Can't even compile the simplest C++ program! Your setup is beyond broken. Repair it, then try again."
		env.Exit(1)
	else:
		print "yes"

	if not conf.CheckCompilerAttribute('deprecated', env):
		print "Your compiler does not support __attribute__((deprecated)) which is neccessary for widelands. Please get a decent compiler."
		env.Exit(1)

	if not conf.CheckCompilerAttribute('noreturn', env):
		print "Your compiler does not support __attribute__((noreturn)) which is neccessary for widelands. Please get a decent compiler."
		env.Exit(1)

	if not conf.CheckCompilerAttribute('noinline', env):
		print "Your compiler does not support __attribute__((noinline)) which is neccessary for widelands. Please get a decent compiler."
		env.Exit(1)

	if not conf.CheckCompilerAttribute('format(printf, 2, 3)', env):
		print "Your compiler does not support __attribute__((format(printf, 2, 3))) which is neccessary for widelands. Please get a decent compiler."
		env.Exit(1)

	setlocalefound=0
	if (conf.CheckFunc('setlocale') or conf.CheckLibWithHeader('', 'locale.h', 'C', 'setlocale("LC_ALL", "C");', autoadd=0)):
		setlocalefound=1

	textdomainfound=0
	if (conf.CheckFunc('textdomain') or conf.CheckLib(library='intl', symbol='textdomain', autoadd=1)):
		textdomainfound=1

	if setlocalefound and textdomainfound:
		print '--> NLS subsystem found.'
	else:
		#TODO: use dummy replacements that just pass back the original string
		print '--> No usable NLS subsystem found. Please install gettext.'
		env.Exit(1)

	if not conf.CheckFunc('getenv'):
		print '--> Your system does not support getenv(). Tilde epansion in filenames will not work.'
	else:
		config_h_file.write("#define HAS_GETENV\n");

	if not conf.CheckSDLConfig(env):
		print 'Could not find sdl-config! Is SDL installed?'
		env.Exit(1)

	if not conf.CheckSDLVersionAtLeast(1, 2, 8, env):
		print 'Could not find an SDL version >= 1.2.8!'
		env.Exit(1)
	else:
		env.ParseConfig(env['sdlconfig']+' --libs --cflags')

	#disabled until somebody finds time and courage to actually work on this #fweber
	#if not conf.CheckParaguiConfig(env):
	#	print 'Could not find paragui. That\'s no problem unless you\'re a developer working on this.'
	#	#print 'Could not find paragui-config! Is paragui installed?'
	#	#env.Exit(1)
	#else:
	#	env.ParseConfig(env['paraguiconfig']+' --libs --cflags')
	#	config_h_file.write("#define HAS_PARAGUI\n\n");

	if not conf.CheckLibWithHeader('z', header='zlib.h', language='C', autoadd=1):
		print 'Could not find the zlib library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLibWithHeader('png', header='png.h', language='C', autoadd=1):
		print 'Could not find the png library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLib(library='SDL_image', symbol='IMG_Load', autoadd=1):
		print 'Could not find the SDL_image library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLib(library='SDL_ttf', symbol='TTF_Init', autoadd=1):
		print 'Could not find the SDL_ttf library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLib(library='SDL_net', symbol='SDLNet_TCP_Open', autoadd=1):
		print 'Could not find the SDL_net library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLib(library='SDL_gfx', symbol='zoomSurface', autoadd=1):
		print 'Could not find the SDL_gfx library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLib(library='SDL_mixer', symbol='Mix_OpenAudio', autoadd=1):
		print 'Could not find the SDL_mixer library! Is it installed?'
		env.Exit(1)

	if conf.TryLink(""" #define USE_RWOPS
			#include <SDL_mixer.h>
			int main(){
				Mix_LoadMUS_RW("foo.ogg");
			}
			""", '.c'):
		config_h_file.write("#define NEW_SDL_MIXER 1\n");
		config_h_file.write("//next line is needed by SDL_mixer\n");
		config_h_file.write("#define USE_RWOPS\n");
		print 'SDL_mixer supports Mix_LoadMUS_RW(). Good'
	else:
		config_h_file.write("#define NEW_SDL_MIXER 0\n");
		print 'Your SDL_mixer does not support Mix_LoadMUS_RW(). Widelands will run without problems, but consider updating SDL_mixer anyway.'

	if conf.CheckLib('efence', symbol='EF_newFrame', language='C', autoadd=0):
		if env.efence:
			conf.CheckCompilerFlag('-include stdlib.h -include string.h -include efence.h', env)
			conf.CheckCompilerFlag('-include new -include fstream -include efencepp.h', env)
			conf.CheckLinkerFlag('-lefence', env)
	else:
		if env.efence:
			print 'Could not find efence, so doing a debug-efence build is impossible !'
			env.Exit(1)

	conf.CheckCompilerFlag('-fstack-protector-all', env)
	conf.CheckCompilerFlag('-fbounds-check', env)
	conf.CheckCompilerFlag('-pipe', env)
	conf.CheckCompilerFlag('-Wall', env)
	conf.CheckCompilerFlag('-Wcast-align', env)
	conf.CheckCompilerFlag('-Wcast-qual', env)
	conf.CheckCompilerFlag('-Wconversion', env)
	conf.CheckCompilerFlag('-Wdisabled-optimization', env)
	conf.CheckCompilerFlag('-Wextra', env)
	#conf.CheckCompilerFlag('-Wfloat-equal', env)
	#conf.CheckCompilerFlag('-Wformat=2', env)
	conf.CheckCompilerFlag('-Winline', env)
	conf.CheckCompilerFlag('-Winvalid-pch', env)
	conf.CheckCompilerFlag('-Wmissing-format-attribute', env)
	conf.CheckCompilerFlag('-Wmissing-include-dirs', env)
	conf.CheckCompilerFlag('-Wmissing-noreturn', env)
	conf.CheckCompilerFlag('-Wno-comment', env)
	conf.CheckCompilerFlag('-Wnormalized=nfc', env)
	conf.CheckCompilerFlag('-Wold-style-cast', env)
	#conf.CheckCompilerFlag('-Werror=old-style-cast', env) Some SDL macros still use old-style casts. Get them fixed!
	#conf.CheckCompilerFlag('-Wpadded', env)
	conf.CheckCompilerFlag('-Wpointer-arith', env)
	conf.CheckCompilerFlag('-Wunsafe-loop-optimizations', env)
	conf.CheckCompilerFlag('-Wshadow', env)
	conf.CheckCompilerFlag('-Wstack-protector', env)
	conf.CheckCompilerFlag('-Wstrict-aliasing=2', env)
	#conf.CheckCompilerFlag('-Wunreachable-code', env)
	conf.CheckCompilerFlag('-Wwrite-strings', env)

	#conf.CheckCompilerFlag('-Wno-deprecated-declarations', env)
	#conf.CheckCompilerFlag('-Wno-unused-variable', env)
	#conf.CheckCompilerFlag('-Wno-unused-parameter', env)
	#conf.CheckCompilerFlag('-Werror', env)

	if env.optimize:
		# !!!! -fomit-frame-pointer breaks execeptions !!!!
		conf.CheckCompilerFlag('-fexpensive-optimizations', env)
		conf.CheckCompilerFlag('-finline-functions', env)
		conf.CheckCompilerFlag('-ffast-math', env)
		conf.CheckCompilerFlag('-funroll-loops', env)
		conf.CheckCompilerFlag('-O3', env)
	else:
		conf.CheckCompilerFlag('-O0', env)

	if env.profile:
		conf.CheckCompilerFlag('-pg', env)
		conf.CheckCompilerFlag('-fprofile-arcs', env)
		conf.CheckLinkerFlag('-pg', env)
		conf.CheckLinkerFlag('-fprofile-arcs', env)

	if env.debug:
		conf.CheckCompilerFlag('-g', env)
		conf.CheckCompilerFlag('-fmessage-length=0', env)

	if env.strip:
		conf.CheckLinkerFlag('-s', env)
