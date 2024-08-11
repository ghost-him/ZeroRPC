#pragma once

template<typename T>
class Singleton{
public:
    static T& getInstance() {
        static T stance;
        return stance;
    }
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
protected:
    Singleton() = default;
};
