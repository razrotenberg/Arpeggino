struct Button
{
    enum class Event : char
    {
        None, // nothing has happened
        
        Down,
        Up,
        Click,
        Press,
        ClickClick,
        ClickPress,
    };

    Button() = default;
    Button(char pin); // setting pin mode to INPUT
    
    Event check();    
    char pin() const { return _pin; }

private:
    enum class What : char
    {
        Idle,
        Touch,
        Press,
        Slip,
        Release,
        SecondPress,
        SecondRelease,
        Drain,
    };
    
    char _pin;
    What _what;
    unsigned short _when; // 16 bits can hold 65,535 possible values, which is enough for storing information for ~65 seconds
};
