#include "RedisConnector.h"
#include <iostream>

using namespace sw::redis;

RedisConnector::RedisConnector() {
    try {
        ConnectionOptions opts;
        opts.host = "redis-15669.c1.ap-southeast-1-1.ec2.redns.redis-cloud.com";
        opts.port = 15669;
        opts.user = "default";
        opts.password = "2wk6l889oflYAEkDcJG6wpVxCH5xjsWR";
        opts.tls.enabled = true;
        opts.tls.sni = "redis-15669.c1.ap-southeast-1-1.ec2.redns.redis-cloud.com";
        opts.socket_timeout = std::chrono::seconds(3);

        redis = std::make_shared<Redis>(opts);
        connected = true;
    } catch (const Error &err) {
        std::cerr << "❌ Redis connection error: " << err.what() << std::endl;
        connected = false;
    }
}

bool RedisConnector::isConnected() const {
    return connected;
}

bool RedisConnector::set(const QString& key, const QString& value) {
    if (!connected) return false;
    try {
        redis->set(key.toStdString(), value.toStdString());
        return true;
    } catch (const sw::redis::Error& err) {
        std::cerr << "❌ Redis SET 抛出异常: " << err.what() << std::endl;
        return false;
    } catch (const std::exception& ex) {
        std::cerr << "❌ Redis SET std::exception: " << ex.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "❌ Redis SET 未知异常" << std::endl;
        return false;
    }
}


QString RedisConnector::get(const QString& key) {
    if (!connected) return QString();
    try {
        auto result = redis->get(key.toStdString());
        return result ? QString::fromStdString(*result) : QString();
    } catch (...) {
        return QString();
    }
}
