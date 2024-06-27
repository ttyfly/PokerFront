#include <DirectXMath.h>
#include <wrl/client.h>

#include "common/math.h"

namespace pf_math
{
    RECT to_dx(const Rect& rect)
    {
        RECT res;
        res.left = rect.x_min;
        res.right = rect.x_max;
        res.top = rect.y_min;
        res.bottom = rect.y_max;
        return res;
    }
};
