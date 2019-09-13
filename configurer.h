#pragma once

#include <LiquidCrystal.h> 
#include <Midiate.h> 

namespace configurer
{

struct Base
{
    Base(
        midiate::Looper::Config & config,
        LiquidCrystal & lcd,
        int col,
        int row,
        int size
    );

    void print();
    
    int col() const { return _col; };
    int row() const { return _row; };

    virtual void init() {};
    virtual bool set(short pot) = 0;

private:
    virtual int _print() = 0;

protected:
    midiate::Looper::Config & _config;
    LiquidCrystal           & _lcd;

private:
    int _col;
    int _row;
    int _size;
};

struct Note : public Base
{
    Note(midiate::Looper::Config & config, LiquidCrystal & lcd);

    bool set(short pot) override;

private:
    int _print() override;
};

struct Mode : public Base
{
    Mode(midiate::Looper::Config & config, LiquidCrystal & lcd);

    bool set(short pot) override;

private:
    int _print() override;
};

struct Octave : public Base
{
    Octave(midiate::Looper::Config & config, LiquidCrystal & lcd);

    void init() override;
    bool set(short pot) override;

private:
    int _print() override;
};

struct BPM : public Base
{
    BPM(midiate::Looper::Config & config, LiquidCrystal & lcd);

    void init() override;
    bool set(short pot) override;

private:
    int _print() override;
};

struct Style : public Base
{
    Style(midiate::Looper::Config & config, LiquidCrystal & lcd);

    void init() override;
    bool set(short pot) override;

private:
    int _print() override;
};

struct Rhythm : public Base
{
    Rhythm(midiate::Looper::Config & config, LiquidCrystal & lcd);

    void init() override;
    bool set(short pot) override;

private:
    int _print() override;
};

} // configurer
