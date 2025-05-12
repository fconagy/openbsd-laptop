
make install

# /usr/include/libfprint/fprint.h
# /usr/lib64/libfprint.so -> libfprint.so.0.0.0
# /usr/lib64/libfprint.so.0 -> libfprint.so.0.0.0
# /usr/lib64/libfprint.so.0.0.0
# 
# /usr/local/stow/libfprint/lib/
# libfprint.a
# libfprint.la
# libfprint.so -> libfprint.so.0.0.0
# libfprint.so.0 -> libfprint.so.0.0.0
# libfprint.so.0.0.0

# mkdir fc18
# mv /usr/lib64/libfprint.so.0.0.0 fc18
cp /usr/local/stow/libfprint/lib/libfprint.so.0.0.0 /usr/lib64/libfprint.so.0.0.0

####################################################################################################
# 
# End
# 
####################################################################################################
