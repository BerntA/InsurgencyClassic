cd ..
rm tmp.sh
find *|egrep "(\.h|\.c|\.cpp)$"|grep -v SocketW|grep -v ZThread|sed -n -e "h;s/^.*$/\L&/;G;/^\(.*\)\n\1$/b;s/^\(.*\)\n\(.*\)$/svn mv --force \2 \1/;p">tmp.sh
find * -type d|grep -v .svn|grep -v SocketW|grep -v ZThread|sed -n -e "h;s/^.*$/\L&/;G;/^\(.*\)\n\1$/b;s/^\(.*\)\n\(.*\)$/svn mv --force \2 \1/;p">>tmp.sh
sh tmp.sh
rm tmp.sh
