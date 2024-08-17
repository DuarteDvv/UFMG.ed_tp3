#ifndef POINTID
#define POINTID

#include "string"
#include "Point.hpp"

struct PointID
{
   std::string id;
   Point p;

   bool operator<(PointID other)
   {
      return p < other.p;
   }

   bool operator<=(PointID other)
   {
      return p <= other.p;
   }

   bool operator>=(PointID other)
   {
      return p >= other.p;
   }

   bool operator>(PointID other)
   {
      return p > other.p;
   }
};

#endif