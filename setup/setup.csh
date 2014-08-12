# This file should be sourced to properly setup the environment for
# Gary Pavlis plane wave migration package - pwmig
# This script leans heavily on setup files used by antelope that the 
# author gratefully acknowledges.   

# This variable should point to the top of the install tree for 
# PWMIG binaries. 

set BASE=/opt/pwmig
# For now this is frozen as a release directory name.  May 
# may be done more elegantly
set RELEASE=2.0
set TOPDIR=${BASE}/${RELEASE}
# Require the top level directory exist or abort
if ( ! -d $TOPDIR ) then
    echo "You need to create a directory $TOPDIR to install this package"
    exit 1
endif
setenv PWMIG $BASE/$RELEASE
endif


# Set up the PATH and MANPATH environment variables.
# If some version of pwmig is already in the path, remove it.
set a="$PWMIG/bin /usr/sbin"

foreach i ( $path ) 
    if ( "$i" !~ /opt/pwmig/* && "$i" != /usr/sbin ) then
        set a=($a $i)
    endif
end
set path=($a)

if ( $?MANPATH ) then
    set a=$PWMIG/man
    foreach i ( `echo $MANPATH | sed 's/:/ /g'` )
        if ( "$i" !~ /opt/pwmig/* ) then
            set a=($a $i)
        endif
    end
    setenv MANPATH `echo $a | sed 's/ /:/g' `
else
    setenv MANPATH $PWMIG/man:/opt/local/man:/usr/local/man:/usr/share/man
endif

setenv PWMIGMAKE     $PWMIG/include/pwmigmake

if ( Darwin == Linux ) then
    setenv MANSECT 1:8:2:3:3p:3t:3y:4:5:6:7:9:tcl:n:l:p:o
endif

if ( -d $PWMIG/src/bin && -r $PWMIG/src/bin ) then
    # if source is present, make it easy to browse by setting cdpath
    # and global tags file.  (There's a TAGS file for emacs enthusiasts also,
    # but I don't know if there's a corresponding environment variable.)
    #
    #   Consider adding to your .exrc and/or .vilerc file:
    #       set tags=tags\ /$PWMIG/src/tags"
    #
    set cdpath= ( ~ $PWMIG/src/{,first,adm,lib/*,bin/*,data,data/*,docs,toolbox} )
endif

unset BASE
set pwmig=$PWMIGMAKE
