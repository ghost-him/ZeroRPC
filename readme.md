# ZeroRPC

用于提升自己编程技能的一个从0开始的项目。

目前还在开发中。

todo:

- 增强鲁棒性：call函数调用失败时的处理，连接失败的处理
- 构建异常模块，用于快速定位异常位置
- RPCServer与RPCClient提供线程池，定时器的接口，方便用户直接调用内置模块

## 快速上手

### 全局函数

RPCServer

```cpp
int add(int a, int b) {
    return a + b;
}

std::string append(std::string a, std::string b) {
    return a + b;
}

int main() {
    RpcServer server(23333);
    server.registerMethod("add", add);
    server.registerMethod("append", append);
    server.run();
    return 0;
}
```

RPCClient

```cpp
int main() {
    RpcClient client("127.0.0.1", 23333);
    client.run();
    int res = client.call<int>("add", 1, 2);
    std::cerr << "res : " << res << std::endl;
    std::string appendedString = client.call<std::string>("append", "hello ", "world");
    std::cerr << "ans: " << appendedString << std::endl;
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
```

### 使用了自定义结构函数

RPCServer

```cpp
class myClass :public enable_serializable { // 1.公开继承自enable_serializable类
public:
    int a, b;
    myClass operator+(const myClass& other) const {
        myClass ret;
        ret.a = this->a + other.a;
        ret.b = this->b + other.b;
        return ret;
    }
    SERIALIZE(a, b) // 2.设置所有的成员变量
};

myClass add4(myClass a, myClass b) {
    return a + b;
}

int main() {
    RpcServer server(23333);
    server.registerMethod("add4", add4);
    server.run();
    return 0;
}
```

RPCClient

```cpp
// 客户端也要有相同的类的定义
class myClass :public enable_serializable {
public:
    int a, b;
    myClass operator+(const myClass& other) const {
        myClass ret;
        ret.a = this->a + other.a;
        ret.b = this->b + other.b;
        return ret;
    }
    SERIALIZE(a, b)
};

int main() {
    RpcClient client("127.0.0.1", 23333);
    client.run();
    myClass a, b;
    a.a = 10;
    a.b = 20;
    b.a = 30;
    b.b = 40;
    auto ret = client.call<myClass>("add4", a, b);
    std::cout << ret.a << " " << ret.b << std::endl;
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
```
### 使用数据压缩功能

客户端与服务端使用的压缩算法要一致！

RPCServer

```cpp
int main () {
    RpcServer server(23333);
    // 设置服务器的压缩算法
    server.set_compress_algo(CompressionType::Brotli);  // 使用Brotli算法压缩数据
    server.registerMethod("add", add);
    server.run();
    return 0;
}
```

RPCClient

```cpp
int main() {
    RpcClient client("127.0.0.1", 23333);
    // 设置客户端的压缩算法
    client.set_compress_algo(CompressionType::Brotli);
    client.run();
    int res = client.call<int>("add", 1, 2);
    std::cerr << "res : " << res << std::endl;
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
```

### 调用成员函数

RPCServer

```cpp
class Generator {
public:
    int get_new_id() {
        return _id++;
    }
private:
    int _id { 1 };
};

int main {
    RpcServer server(23333);
    Generator gen;
    server.registerMethod("requestID", [&](){
        return gen.get_new_id();
    });
    server.run();
    return 0;
}
```

RPCClient

```cpp
int main {
    RpcClient client("127.0.0.1", 23333);
    client.run();
    while(1) {
        std::cout << client.call<int32_t>("requestID") << std::endl;
        std::cout << "send test message" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds (1000));
    }
    return 0;
}
```

