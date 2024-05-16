#ifndef tp_maps_headless_Map_h
#define tp_maps_headless_Map_h

#include "tp_maps_headless/Globals.h"

#include "tp_maps/Map.h"

namespace tp_maps_headless
{

//##################################################################################################
class TP_MAPS_HEADLESS_EXPORT Map : public tp_maps::Map
{
  TP_DQ;
public:
  //################################################################################################
  Map(bool enableDepthBuffer = true);

  //################################################################################################
  ~Map() override;

  //################################################################################################
  void makeCurrent() override;

  //################################################################################################
  void callAsync(const std::function<void()>& callback) override;

  //################################################################################################
  void poll();
};
}
#endif
