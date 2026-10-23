#ifndef VECTOR_H
#define VECTOR_H


class Vector2{
    public:
        static const Vector2 North;
        static const Vector2 South;
        static const Vector2 East;
        static const Vector2 West;

    private:
        int _x;
        int _y;

    public:
        Vector2();
        Vector2(int x, int y);
        int GetX() const;
        int GetY() const;

        Vector2 operator + (const Vector2 &rhs) const; 
        Vector2& operator += (const Vector2 &rhs);

        Vector2 operator - (const Vector2 &rhs) const; 
        Vector2& operator -= (const Vector2 &rhs);

        bool operator== (const Vector2 &rhs) const;
        bool operator!= (const Vector2 &rhs) const;
        //Vector2& operator = (const Vector2 &pos);
};

#endif