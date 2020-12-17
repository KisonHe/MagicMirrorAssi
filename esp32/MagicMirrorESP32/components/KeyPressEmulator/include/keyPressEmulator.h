/**
 * @brief Nothing
 *
 * @ingroup PackageName
 * (Note: this needs exactly one @defgroup somewhere)
 *
 * @note Must use in rtos
 * 
 * @author KisonHe
 * Contact: 
 *
 */
#ifndef KEYPRESSEMULATOR_H
#define KEYPRESSEMULATOR_H

// int createStartKPETimer();

class keyPressEmulator
{
private:
    int gpioNum;
    int isHigh2Press;
    int _deinitPin();
    int _initPin();
    bool lock;
public:
    int doPress();
    keyPressEmulator(int gpioNum_, int isHigh2Press_);
    // ~keyPressEmulator();
};


#endif
