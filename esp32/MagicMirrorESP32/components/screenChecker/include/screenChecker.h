/**
 * @brief This file is very unlikely to be used in future projects. So wont write in ez-reuse way.
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

#ifndef SCREENCHECKER_H
#define SCREENCHECKER_H

namespace ScreenChecker
{
    struct SCStatus_
    {
        bool isPowered;
        bool isTurnedOn;
    };
    extern SCStatus_ SCStatus; 
    int init();
} // namespace ScreenChecker

#endif
