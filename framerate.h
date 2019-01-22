#ifndef FRAMERATE_H
#define FRAMERATE_H
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>

class FrameRate
{
   private:
      unsigned short FPS;    // Frame Per Seconds
      double TPF; // Time Per Frame
      sf::Clock clock;      
      
   public:
      FrameRate(int fps);
      float operator()();
      const double& tpf();
      double reset();
};

//-------------------------------------------------------------------

inline FrameRate::FrameRate(int fps)
          : FPS(fps), TPF(1.0/fps), clock()
{}

//-------------------------------------------------------------------

inline float FrameRate::operator()()
{
   return clock.getElapsedTime().asSeconds();
}

//-------------------------------------------------------------------

inline const double& FrameRate::tpf()
{
   return TPF;
}

//-------------------------------------------------------------------

inline double FrameRate::reset()
{
   return clock.restart().asSeconds();
}


//-------------------------------------------------------------------

#endif