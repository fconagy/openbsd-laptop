
./configure --enable-examples-build  --enable-x11-examples-build --enable-debug-log --prefix=/usr/local/stow/libfprint/

# prefix is /usr/local by default

make clean
make -j 4

####################################################################################################
# 
# End
# 
####################################################################################################
