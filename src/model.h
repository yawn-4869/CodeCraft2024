#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <string>
#include <string.h>
#include <map>
#include <list>
#include <queue>

#include "const_variable.h"

typedef struct Point {
    int x, y;
    Point() {}
    Point(int pos_x, int pos_y) : x(pos_x), y(pos_y){}
    bool operator==(const Point& other) const {
        return this->x == other.x && this->y == other.y;
    }
} Point;

namespace std {
template <> struct hash<Point> {
  std::size_t operator()(const Point& p) const noexcept {
    return std::hash<int>()(p.x ^ (p.y << 16));
  }
};
}

// typedef struct Robot {
//     int id; // 数组中的下标
//     Point location{0, 0}; // 坐标
//     int have_goods; // 是否有货物
//     int status; // 状态 0 -- 恢复 1 -- 运行
//     int target_berth; // 目标泊位
//     int goods_id; // 携带的货物id
//     Robot() {}
//     Robot(int pos_x, int pos_y) 
//         : location(pos_x, pos_y), have_goods(0), status(0), target_berth(-1), goods_id(-1) {}
// };

/*货物*/
typedef struct Goods {
    int id; // 数组中的下标
    Point location{0, 0}; // 坐标
    int price; // 金额
    int rest_frame; // 剩余存活时间
    int robot_id; // 机器人id
    Goods() {}
    Goods(int pos_x, int pos_y, int _price) 
        : price(_price), location(pos_x, pos_y), rest_frame(ConstVariable::goods_live_frames), robot_id(-1) {}
} Goods; 

/*泊位*/
typedef struct Berth {
    int id; // 数组中的下标
    Point top_left{0, 0}; // 坐标, 泊位为4*4的矩形, 存储左上角坐标
    Point bottom_right{0, 0}; // 坐标, 泊位为4*4的矩形, 存储右下角坐标
    int in_sea; // 判定左上角是否在海里 0 -- false 1 -- true
    int transport_time; // 泊位轮船运输到虚拟点的时间, 即产生价值的时间
    int loading_speed; // 装载速度，即每帧可以装载的物品数
    int goods_num; // 已存放的货物数量
    int goods_value; // 已存放的货物价值
    Berth() {}
    Berth(int pos_x, int pos_y, int time, int velocity) 
        : top_left(pos_x, pos_y), transport_time(time), loading_speed(velocity), goods_num(0), goods_value(0) {}
} Berth;

/*轮船*/
typedef struct Boat {
    int id; // 数组中的下标
    int status; // 状态 0 -- 移动中 1 -- 运行(装货/运输完成) 2 -- 泊位等待
    int berth_id; // 目标泊位
    int capacity; // 船容量
    int is_first{ 0 }; // 访问第一个泊位还是第二个
    Boat() {}
    Boat(int _status, int bid) : status(_status), berth_id(bid) {}
} Boat;

/*地图*/
typedef struct Map {
    int current_frame;
    int current_money;
    int boat_capacity; // 统一的船容量
    int goods_num; // 目前地图货物数量

    std::vector<std::string> init_map;
    Map() : init_map(ConstVariable::map_row_num + 2, std::string(ConstVariable::map_col_num + 2, '#')) {}
} Map;

class Robot {
public:
    Robot() {}
    Robot(int pos_x, int pos_y) 
        : location(pos_x, pos_y), have_goods(0), status(0), target_berth(-1), goods_id(-1) {}
public:
    int id; // 数组中的下标
    Point location{0, 0}; // 坐标
    int have_goods; // 是否有货物
    int status; // 状态 0 -- 恢复 1 -- 运行
    int target_berth; // 目标泊位
    int goods_id; // 携带的货物id
    std::map<int, std::list<Point>> robot_2_goods;
    std::map<int, std::list<Point>> robot_2_berth;
    // 货物优先队列
    std::priority_queue<std::pair<double, int>> goods_pq;
    // 泊位优先队列
    std::priority_queue<std::pair<double, int>> berth_pq;
};


namespace model {
    std::vector<Point> points;
    std::vector<Robot> robots;
    //std::unordered_map<Point, int> robot_index;
    // std::vector<Goods> goods;
    std::map<int, Goods> goods;
    std::vector<Berth> berths;
    std::vector<Boat> boats;
    Map current_map = Map();
};

#endif