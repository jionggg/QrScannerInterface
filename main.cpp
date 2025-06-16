#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <sw/redis++/redis++.h>

int main(int argc, char *argv[]) {
    try {
        sw::redis::Redis redis("tcp://default:2wk6l889oflYAEkDcJG6wpVxCH5xjsWR@redis-15669.c1.ap-southeast-1-1.ec2.redns.redis-cloud.com:15669");

        redis.set("hello", "zeiss");
        auto val = redis.get("hello");
        if (val) {
            std::cout << "✅ Redis value: " << *val << std::endl;
        } else {
            std::cout << "❌ Redis key not found" << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "❌ Redis error: " << e.what() << std::endl;
    }

    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
