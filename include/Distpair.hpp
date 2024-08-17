#ifndef DISTPAIR
#define DISTPAIR

#include <iostream>

struct Distpair
{
   std::string id;
   double dist;

   Distpair(std::string id = "", double dist = 0) : id(id), dist(dist) {};

   bool operator<(const Distpair &other) const
   {
      return dist < other.dist;
   }

   bool operator<=(const Distpair &other) const
   {
      return dist <= other.dist;
   }

   bool operator>(const Distpair &other) const
   {
      return dist > other.dist;
   }

   bool operator>=(const Distpair &other) const
   {
      return dist >= other.dist;
   }
};

#endif