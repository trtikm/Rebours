#include <rebours/MAL/descriptor/storage.hpp>
#include <rebours/utility/file_utils.hpp>
#include <rebours/utility/msgstream.hpp>
#include <rebours/utility/assumptions.hpp>
#include <rebours/utility/invariants.hpp>
#include <rebours/utility/development.hpp>
#include <rebours/utility/endian.hpp>
#include <unordered_set>
#include <algorithm>

namespace mal { namespace descriptor { namespace detail { namespace x86_64_Linux { namespace detail {


void  store_envvar_pointer(uint64_t  ptr, std::vector<uint8_t>&  pointers)
{
    uint8_t*  begin = reinterpret_cast<uint8_t*>(&ptr);
    uint8_t* const  end = begin + sizeof(ptr);
    if (!is_this_little_endian_machine())
        std::reverse(begin,end);
    for ( ; begin != end; ++begin)
        pointers.push_back(*begin);
}

void  append(std::vector<uint8_t>&  output, std::string const&  data, bool const  append_zero = true)
{
    for (auto const  c : data)
        output.push_back(c);
    if (append_zero)
        output.push_back('\0');
}


}}}}}


namespace mal { namespace descriptor { namespace detail { namespace x86_64_Linux {


void  build_stack_init_data(std::string const&  root_file, std::vector<std::string>&  cmd_line, std::vector<environment_variable_type>&  env_vars)
{
    cmd_line.push_back(fileutl::concatenate_file_paths("/home/user",fileutl::parse_name_in_pathname(root_file)));

    env_vars.push_back({"SESSION","ubuntu"});
    env_vars.push_back({"DESKTOP_SESSION","ubuntu"});
    env_vars.push_back({"SESSIONTYPE","gnome-session"});
    env_vars.push_back({"LANGUAGE","en"});
    env_vars.push_back({"SELINUX_INIT","YES"});
    env_vars.push_back({"SHELL","/bin/bash"});
    env_vars.push_back({"TERM","xterm"});
    env_vars.push_back({"COLORTERM","gnome-terminal"});
    env_vars.push_back({"USER","user"});
    env_vars.push_back({"LOGNAME","user"});
    env_vars.push_back({"HOME","/home/user"});
    env_vars.push_back({"XDG_DATA_DIRS","/usr/share/ubuntu:/usr/share/gnome:/usr/local/share/:/usr/share/"});
    env_vars.push_back({"PATH","/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games"});
}


void  build_stack_aeinfo(std::vector<uint8_t>&  data, std::vector<uint8_t>&  pointers, uint64_t const  begin_address, uint64_t const  end_address,
                         file_descriptor_ptr const  file_descriptor)
{
    std::unordered_set<std::string>  paths;
    for (auto const& elem : *file_descriptor->files_table())
    {
        uint64_t const  endpos = elem.second->path().find_last_of('/');
//        if (endpos == std::string::npos)
//            endpos = elem.second->path().find_last_of('\\');
        std::string const  path = elem.second->path().substr(0ULL,endpos);
        paths.insert(path);
    }

    std::string  user_name = "anonymous_user";
    for (std::string const&  p : paths)
    {
        static std::string const  prefix = "/home/";
        if (p.find(prefix) == 0ULL)
        {
            user_name = p.substr(prefix.size(),p.find_first_of('/',prefix.size())-prefix.size());
            break;
        }
    }

    std::vector<uint8_t>  envvars;
    detail::append(envvars,"SESSION=ubuntu");
    detail::append(envvars,"DESKTOP_SESSION=ubuntu");
    detail::append(envvars,"SESSIONTYPE=gnome-session");
    detail::append(envvars,"LANGUAGE=en");
    detail::append(envvars,"SELINUX_INIT=YES");
    detail::append(envvars,"SHELL=/bin/bash");
    detail::append(envvars,"TERM=xterm");
    detail::append(envvars,"COLORTERM=gnome-terminal");
    detail::append(envvars,msgstream() << "USER=" << user_name);
    detail::append(envvars,msgstream() << "LOGNAME=" << user_name);
    detail::append(envvars,msgstream() << "HOME=/home/" << user_name);
    detail::append(envvars,"XDG_DATA_DIRS=/usr/share/ubuntu:/usr/share/gnome:/usr/local/share/:/usr/share/");
    detail::append(envvars,"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games");

    uint64_t  var_begin = end_address - envvars.size();
    ASSUMPTION(var_begin >= begin_address);

    uint64_t  ptr = var_begin;
    for (std::string::value_type const&  c : envvars)
    {
        data.push_back(c);
        ++ptr;
        if (c == '\0')
        {
            detail::store_envvar_pointer(var_begin,pointers);
            var_begin = ptr;
        }
    }
    detail::store_envvar_pointer(0ULL,pointers);

    ASSUMPTION(begin_address + data.size() + pointers.size() <= end_address);
}


}}}}


//    /LC_PAPER=fr_FR.UTF-8
//    XDG_VTNR=7
//    LC_ADDRESS=fr_FR.UTF-8
//    XDG_SESSION_ID=c2
//    CLUTTER_IM_MODULE=xim
//    XDG_GREETER_DATA_DIR=/var/lib/lightdm-data/marek
//    SELINUX_INIT=YES
//    LC_MONETARY=fr_FR.UTF-8
//    SESSION=ubuntu
//    GPG_AGENT_INFO=/run/user/1000/keyring-nF68s0/gpg:0:1
//    VTE_VERSION=3409
//    SHELL=/bin/bash
//    XDG_MENU_PREFIX=gnome-
//    TERM=xterm
//    LC_NUMERIC=fr_FR.UTF-8
//    WINDOWID=65012285
//    UPSTART_SESSION=unix:abstract=/com/ubuntu/upstart-session/1000/2544
//    GNOME_KEYRING_CONTROL=/run/user/1000/keyring-nF68s0
//    GTK_MODULES=overlay-scrollbar:unity-gtk-module
//    USER=marek
//    LS_COLORS=rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arj=01;31:*.taz=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lz=01;31:*.xz=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=00;36:*.au=00;36:*.flac=00;36:*.mid=00;36:*.midi=00;36:*.mka=00;36:*.mp3=00;36:*.mpc=00;36:*.ogg=00;36:*.ra=00;36:*.wav=00;36:*.axa=00;36:*.oga=00;36:*.spx=00;36:*.xspf=00;36:
//    LC_TELEPHONE=fr_FR.UTF-8
//    XDG_SESSION_PATH=/org/freedesktop/DisplayManager/Session0
//    XDG_SEAT_PATH=/org/freedesktop/DisplayManager/Seat0
//    SSH_AUTH_SOCK=/run/user/1000/keyring-nF68s0/ssh
//    SESSION_MANAGER=local/gartus:@/tmp/.ICE-unix/2725,unix/gartus:/tmp/.ICE-unix/2725
//    DEFAULTS_PATH=/usr/share/gconf/ubuntu.default.path
//    XDG_CONFIG_DIRS=/etc/xdg/xdg-ubuntu:/usr/share/upstart/xdg:/etc/xdg
//    PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games
//    DESKTOP_SESSION=ubuntu
//    QT_IM_MODULE=ibus
//    QT_QPA_PLATFORMTHEME=appmenu-qt5
//    LC_IDENTIFICATION=fr_FR.UTF-8
//    JOB=dbus
//    PWD=/home/marek/root/cpptests/bin/tests
//    XMODIFIERS=@im=ibus
//    LANG=en_US.UTF-8
//    GNOME_KEYRING_PID=2542
//    GDM_LANG=en
//    MANDATORY_PATH=/usr/share/gconf/ubuntu.mandatory.path
//    LC_MEASUREMENT=fr_FR.UTF-8
//    IM_CONFIG_PHASE=1
//    COMPIZ_CONFIG_PROFILE=ubuntu
//    GDMSESSION=ubuntu
//    SESSIONTYPE=gnome-session
//    HOME=/home/marek
//    SHLVL=1
//    XDG_SEAT=seat0
//    LANGUAGE=en
//    GNOME_DESKTOP_SESSION_ID=this-is-deprecated
//    LOGNAME=marek
//    XDG_DATA_DIRS=/usr/share/ubuntu:/usr/share/gnome:/usr/local/share/:/usr/share/
//    DBUS_SESSION_BUS_ADDRESS=unix:abstract=/tmp/dbus-Q7HliKKgpd
//    QT4_IM_MODULE=ibus
//    LESSOPEN=| /usr/bin/lesspipe %s
//    INSTANCE=
//    TEXTDOMAIN=im-config
//    DISPLAY=:0.0
//    XDG_RUNTIME_DIR=/run/user/1000
//    XDG_CURRENT_DESKTOP=Unity
//    GTK_IM_MODULE=ibus
//    LESSCLOSE=/usr/bin/lesspipe %s %s
//    LC_TIME=fr_FR.UTF-8
//    TEXTDOMAINDIR=/usr/share/locale/
//    LC_NAME=fr_FR.UTF-8
//    XAUTHORITY=/home/marek/.Xauthority
//    COLORTERM=gnome-terminal
//    _=./environment_vars_Linux_Debug
