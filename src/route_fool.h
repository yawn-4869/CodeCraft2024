#ifndef __ROUTE_FOOL_H__
#define __ROUTE_FOOL_H__

#include <map>
#include "algorithm_tools.h"

namespace RouteFool {

// <robot_id, <goods_id, path>>
std::map<int, std::map<int, std::vector<Point>>> robot_2_goods;
// <robot_id, <berth_id, path>>                                                                                                                        
std::map<int, std::map<int, std::vector<Point>>> robot_2_berth;


}

#endif                                                                                                     