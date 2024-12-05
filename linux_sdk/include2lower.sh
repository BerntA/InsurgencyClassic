cd ..
sed -i -e "/#include/s/[<\"].*[\">]/\L&/" `find *|egrep "(\.h|\.c|\.cpp)$"|grep -v SocketW|grep -v ZThread`
