/*
 * conf.hpp
 *
 *  Created on: Oct 25, 2020
 *      Author: ryuni
 */

#ifndef INC_CONF_HPP_
#define INC_CONF_HPP_

struct ConfStruct
{
        uint16_t can_id;
};

extern ConfStruct confStruct;

void readConf(void);
void writeConf(void);

inline ConfStruct * getConf(void)
{
    return &confStruct;
}

#endif /* INC_CONF_HPP_ */
