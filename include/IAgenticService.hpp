#pragma once

class IAgenticService {
public:
    virtual ~IAgenticService() = default;
    virtual int run() = 0;
};
