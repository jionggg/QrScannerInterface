#ifndef REDISCONNECTOR_H
#define REDISCONNECTOR_H

#include <QString>
#include <memory>
#include <sw/redis++/redis++.h>

class RedisConnector {
public:
    RedisConnector();              // 自动连接 Redis Cloud
    bool isConnected() const;      // 检查连接状态
    bool set(const QString& key, const QString& value);  // 写入 key
    QString get(const QString& key);                     // 读取 key

private:
    std::shared_ptr<sw::redis::Redis> redis;
    bool connected = false;
};

#endif // REDISCONNECTOR_H
