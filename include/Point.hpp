#ifndef POINT
#define POINT

struct Point
{
   double x, y;
   Point(double x = 0, double y = 0) : x(x), y(y) {}

   bool operator<(Point other)
   {
      if (x == other.x)
      {
         return y < other.y;
      }
      return x < other.x;
   }

   bool operator<=(Point other)
   {
      if (x == other.x)
      {
         return y <= other.y;
      }
      return x <= other.x;
   }

   bool operator>(Point other)
   {
      if (x == other.x)
      {
         return y > other.y;
      }
      return x > other.x;
   }

   bool operator>=(Point other)
   {
      if (x == other.x)
      {
         return y >= other.y;
      }
      return x >= other.x;
   }
};

#endif