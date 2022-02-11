#pragma once

#include "types.h"

inline u8 buttonid = 1;
inline u8 strobe = 0;

u8 controls_keys(u8 id);
void controls_write(u8 val);
u8 controls_read();