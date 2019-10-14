#ifndef SIMULATE_H
#define SIMULATE_H

class simulate
{
public:
    simulate();
    ~simulate();
    void test();
    void inject(int type, int code, int value);

private:
    int fd;
    bool ok;
};

#endif // SIMULATE_H
