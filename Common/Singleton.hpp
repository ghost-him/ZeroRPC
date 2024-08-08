#pragma once

template<typename T>
class Singleton{
public:
    static T& getInstance() {
        static T stance;
        return stance;
    }
};

