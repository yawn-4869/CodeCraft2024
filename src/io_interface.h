#ifndef __IO_INTERFACE__
#define __IO_INTERFACE__

#include <iostream>
#include "args.h"
#include "const_variable.h"
#include "model.h"
#include "log.h"

namespace io {
    void init(std::istream& io_in) {
        // 初始化地图
        for(int i = 0; i < ConstVariable::map_row_num; ++i) {
            std::string str;
            std::getline(io_in, str);
            model::current_map.init_map[i] = str;
        }

        // 初始化机器人数组
        model::robots.resize(ConstVariable::robot_num);
        
        // 初始化泊位数组
        model::berths.resize(ConstVariable::berth_num);
        for(int i = 0; i < ConstVariable::berth_num; ++i) {
            Berth berth;
            io_in >> berth.id >> berth.top_left.x >> berth.top_left.y >> berth.transport_time >> berth.loading_speed;
            berth.in_sea = 0;
            berth.bottom_right = Point(berth.top_left.x + 3, berth.top_left.y + 3);
            berth.robot_id = -1;
            berth.boat_id = -1;
            berth.goods_num = 0;
            model::berths[i] = berth;
        }

        // 初始化船容量
        io_in >> model::current_map.boat_capacity;
        model::boats.resize(ConstVariable::boat_num);

        // 读入标识结束的“ok”
        std::string ok;
        io_in >> ok;
        if (_USE_LOG_) {
            std::cerr << "[info] init success, ok = " << ok << std::endl;
        }
    }

    void readFrame(std::istream& io_in) {
        int frame, money;
        io_in >> frame >> money;

        // if(frame - model::current_map.current_frame > 1) {
        //     if (_USE_LOG_) {
        //         std::cerr << "[eror] !!!skip flame !" << std::endl;
        //     }
        // }

        model::current_map.current_frame = frame;
        model::current_map.current_money = money;

        // 货物信息
        int goods_num;
        io_in >> goods_num;
        for(int i = 0; i < goods_num; ++i) {
            Goods tmp;
            io_in >> tmp.location.x >> tmp.location.y >> tmp.price;
            tmp.id = model::current_map.goods_num + i;
            tmp.rest_frame = ConstVariable::goods_live_frames;
            tmp.robot_id = -1;
            model::goods[tmp.id] = tmp;
        }

        model::current_map.goods_num += goods_num;

        // 去除过期货物
        for(auto ite = model::goods.begin(); ite != model::goods.end();) {
            if(ite->second.rest_frame == 0) {
                ite = model::goods.erase(ite);
            } else {
                break;
            }
        }

        // 机器人信息
        for(int i = 0; i < ConstVariable::robot_num; i ++) {
            if(frame != 1) {
                Robot &tmp = model::robots[i];
                io_in >> tmp.have_goods >> tmp.location.x >> tmp.location.y >> tmp.status;
            } else {
                Robot tmp;
                io_in >> tmp.have_goods >> tmp.location.x >> tmp.location.y >> tmp.status;
                tmp.id = i;
                tmp.target_berth = -1;
                tmp.goods_id = -1;
                model::robots[i] = tmp;
            }
        }

        // 船信息
        for(int i = 0; i < ConstVariable::boat_num; i ++) {
            if(frame != 1) {
                Boat &tmp = model::boats[i];
                io_in >> tmp.status >> tmp.berth_id;
            } else {
                Boat tmp;
                io_in >> tmp.status >> tmp.berth_id;
                tmp.id = i;
                tmp.capacity = model::current_map.boat_capacity;
                model::boats[i] = tmp;
            }
        }
 
        if (_USE_LOG_) {
            Log("[info] read goods success, current frame: " + std::to_string(frame) + " money: " + std::to_string(money) + 
            "goods_num: " + std::to_string(goods_num));
        }

        std::string ok;
        io_in >> ok;
    }
}

#endif