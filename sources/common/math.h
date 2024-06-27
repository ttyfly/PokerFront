namespace pf_math
{
    class Vec3
    {
    public:
        Vec3() : x(0.f), y(0.f), z(0.f) {}
        Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

        Vec3 operator+(const Vec3& other)
        {
            return Vec3(x + other.x, y + other.y, z + other.z);
        }

        Vec3 operator-(const Vec3& other)
        {
            return Vec3(x - other.x, y - other.y, z - other.z);
        }

        Vec3 operator*(const Vec3& other)
        {
            return Vec3(x * other.x, y * other.y, z * other.z);
        }

        Vec3 operator*(float multiplier)
        {
            return Vec3(x * multiplier, y * multiplier, z * multiplier);
        }

        Vec3 operator/(float divider)
        {
            return Vec3(x / divider, y / divider, z / divider);
        }

    public:
        float x;
        float y;
        float z;
    };

    class Quat
    {
    public:
        Quat() : w(1.f), x(0.f), y(0.f), z(0.f) {}
        Quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    public:
        float w;
        float x;
        float y;
        float z;
    };

    class Rect
    {
    public:
        float x_min;
        float y_min;
        float x_max;
        float y_max;
    };

    class Transform
    {
    public:
        Vec3 translation;
        Quat rotation;
        Vec3 scale;
    };
}