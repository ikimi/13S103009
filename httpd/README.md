# Tinyhttpd

kimi's tiny httpd server

-----------------------

-文件结构:
    ./iinclude      包含自定义的头文件
    ./cgi/bin       包含cgi测试程序
    ./*.c           包含自定义库文件
    ./head.jpg      用于测试的静态图片
    ./index.html    用于测试的静态html文档 

-编译方法:
    gcc -c read_requestline.c
    gcc -c init.c
    gcc -c serve_dynamic.c
    gcc -c serve_static.c
    gcc -c parse_requestheader.c
    gcc -c clienterror.c
    gcc -c parse_uri.c
    gcc -c service.c

    gcc httpd.c read_requestline.o init.o serve_dynamic.o serve_static.o
        parse_requestheader.o parse_uri.o clienterror.o service.o -o httpd

-运行方法
    ./httpd [port]
