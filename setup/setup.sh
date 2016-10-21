# This file should be sourced to properly setup the environment for
# Gary Pavlis plane wave migration package - pwmig
# This script leans heavily on setup files used by antelope that the 
# author gratefully acknowledges.   

# This variable should point to the top of the install tree for 
# PWMIG binaries. 
# YOU MUST CHANGE THE NEXT LINE

BASE="/Users/wangyinz/PWMIGdebug"
# For now this is frozen as a release directory name.  May 
# may be done more elegantly
RELEASE=2.0
TOPDIR="${BASE}/${RELEASE}"
# Require the top level directory exist or abort
if [ ! -d "$TOPDIR" ]; then
    echo "You need to create a directory $TOPDIR to install this package"
    exit 1
fi
export PWMIG="$BASE/$RELEASE"

# Give boost related path
export BOOSTHDR=/opt/boost/include
export BOOSTLIB=/opt/boost/lib


# Set up the PATH and MANPATH environment variables.
# If some version of pwmig is already in the path, remove it.
a="$PWMIG/bin:/usr/sbin"

for i in `echo $PATH | sed 's/:/ /g'` ; do
    if [[ ! "$i" =~ "/opt/pwmig/*" && "$i" != "/usr/sbin" ]]; then
        a="$a:$i"
    fi
done
export PATH="$a"

if [ -z ${MANPATH+x} ];then
    a="$PWMIG/man"
    for i in `echo $MANPATH | sed 's/:/ /g'` ; do
        if [[ ! "$i" =~ "/opt/pwmig/*" ]]; then
            a="$a:$i"
        fi
    done
    export MANPATH="$a"
else
    export MANPATH="$PWMIG/man:/opt/local/man:/usr/local/man:/usr/share/man"
fi

export PWMIGMAKE="$PWMIG/include/pwmigmake"

if [ Darwin == Linux ] ;then
    export MANSECT=1:8:2:3:3p:3t:3y:4:5:6:7:9:tcl:n:l:p:o
fi

if [[ -d "$PWMIG/src/bin" && -r "$PWMIG/src/bin" ]]; then
    # if source is present, make it easy to browse by setting cdpath
    # and global tags file.  (There's a TAGS file for emacs enthusiasts also,
    # but I don't know if there's a corresponding environment variable.)
    #
    #   Consider adding to your .exrc and/or .vilerc file:
    #       set tags=tags\ /$PWMIG/src/tags"
    #
    cdpath= "~ $PWMIG/src/{,first,adm,lib/*,bin/*,data,data/*,docs,toolbox}"
fi

unset BASE
export pwmig=$PWMIGMAKE
