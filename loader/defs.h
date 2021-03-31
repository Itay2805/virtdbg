#pragma once

#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define DIV_ROUNDUP(a, b) (((a) + ((b) - 1)) / (b))
