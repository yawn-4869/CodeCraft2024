#include <stdio.h>
#include <iostream>
#include <fstream>
#include "src/model.h"
#include "src/algorithm_tools.h"
#include "src/args.h"
#include "src/io_interface.h"
#include "src/log.h"

int main() {
    io::init(std::cin);
    std::cout << "OK" << "\n";
    fflush(stdout);
    
    for(int curr_frame = 0; curr_frame < ConstVariable::total_frame; curr_frame++) {
        io::readFrame(std::cin);
        int last_frame = ConstVariable::total_frame - curr_frame;

        // 生成货物优先队列及泊位优先队列
        for(int i = 0; i < ConstVariable::robot_num; ++i) {
            // Log("model::robots[i].have_goods: " + std::to_string(model::robots[i].have_goods));
            if(!model::robots[i].have_goods) {
                // Log("goods pq generate...");
                if(model::robots[i].goods_id == -1) {
                    // 没有货物且也没有标记, 给机器人分配货物
                    for(auto ite = model::goods.begin(); ite != model::goods.end(); ite++) {
                        if(ite->second.robot_id != -1) {
                            // 货物已有对应的机器人标记
                            continue;
                        }

                        // 大致预筛选, 减少时间复杂度
                        int m_distance = AlgorithmTools::calculateMDistance(model::robots[i].location, ite->second.location);
                        // Log( "(" + std::to_string(model::robots[i].location.x) + "," + std::to_string(model::robots[i].location.y) + ") (" +
                        //     std::to_string(ite->second.location.x) + "," + std::to_string(ite->second.location.y) + ")"
                        //     + "m_distance: " + std::to_string(m_distance));
                        if(m_distance > ite->second.rest_frame) {
                            // 更新路径列表
                            if(model::robots[i].robot_2_goods.count(ite->second.id)) {
                                model::robots[i].robot_2_goods.erase(ite->second.id);
                            }
                            continue;
                        }
                        
                        if(m_distance == 0) {
                            // 机器人就在货物处, 最高优先级，直接分配
                            ite->second.robot_id = model::robots[i].id;
                            model::robots[i].goods_id = ite->second.id;
                            if(_USE_LOG_) {
                                Log("[info] goods [" + std::to_string(ite->second.id) + "] assign to robot [" + std::to_string(ite->second.robot_id) + 
                                "]" + "path size: " + std::to_string(model::robots[i].robot_2_goods[ite->second.id].size()));
                            }
                            // 清空对应的货物优先级队列
                            std::priority_queue<std::pair<double, int>> tmp;
                            model::robots[i].goods_pq.swap(tmp);
                            continue;
                        }

                        // Log("A* start");
                        std::list<Point> path;
                        int need_frame = AlgorithmTools::findMinPath(model::current_map.init_map, model::robots[i].location, 
                        ite->second.location, path);
                        // Log("goods A* end: " + std::to_string(need_frame));

                        if(need_frame == 0) {
                            // 更新路径列表
                            if(model::robots[i].robot_2_goods.count(ite->second.id)) {
                                model::robots[i].robot_2_goods.erase(ite->second.id);
                            }
                            continue;
                        }
                        double goods_priority_value = AlgorithmTools::get_goods_priority_value(need_frame, ite->second);
                        if(goods_priority_value > 0) {
                            model::robots[i].robot_2_goods[ite->second.id] = path;
                            model::robots[i].goods_pq.push(std::make_pair(goods_priority_value, ite->second.id));
                        }
                    }
                }
            } else {
                // Log("berths pq generate...");
                if(model::robots[i].target_berth == -1) {
                    // 已有货物但是没有目标泊位, 给机器人分配泊位
                    for(int j = 0; j < model::berths.size(); ++j) {
                        if(model::berths[j].robot_id != -1) {
                            // 港口已有对应的机器人前往
                            continue;
                        }

                        // 大致预筛选, 减少时间复杂度
                        int m_distance = AlgorithmTools::calculateMDistance(model::robots[i].location, model::berths[j].top_left);
                        if(m_distance > last_frame) {
                            // 更新路径列表
                            if(model::robots[i].robot_2_berth.count(model::berths[j].id)) {
                                model::robots[i].robot_2_berth.erase(model::berths[j].id);
                            }
                            continue;
                        }
                        
                        if(m_distance == 0) {
                            // 机器人就在泊位处, 最高优先级，直接分配
                            model::berths[j].robot_id = model::robots[i].id;
                            model::robots[i].target_berth = model::berths[j].id;
                            // Log("[info] robot [" + std::to_string(model::robots[i].id) + "] generate berth path completed, target: " 
                            // + std::to_string(model::berths[i].id) + "path size: " + std::to_string(model::robots[i].robot_2_berth[model::berths[i].id].size()));

                            // 清空泊位优先级队列
                            std::priority_queue<std::pair<double, int>> tmp;
                            model::robots[i].berth_pq.swap(tmp);
                            continue;
                        }

                        std::list<Point> path;
                        Point end = model::berths[j].top_left;
                        if(model::berths[j].in_sea) {
                            end = model::berths[j].bottom_right;
                        }
                        int need_frame = AlgorithmTools::findMinPath(model::current_map.init_map, model::robots[i].location, 
                        end, path);

                        // Log("berth A* end: " + std::to_string(need_frame));

                        if(need_frame == 0) {
                            // 现在位置去往目的地没有路径, 更新路径列表
                            if(model::robots[i].robot_2_berth.count(model::berths[j].id)) {
                                model::robots[i].robot_2_berth.erase(model::berths[j].id);
                            }
                            continue;
                        }

                        double berth_priority_value = AlgorithmTools::get_berth_priority_value(need_frame, model::berths[j], last_frame);
                        if(berth_priority_value > 0) {
                            model::robots[i].robot_2_berth[model::berths[j].id] = path;
                            model::robots[i].berth_pq.push(std::make_pair(berth_priority_value, model::berths[j].id));
                        }
                    }
                }
            }
        }

        // 机器人标记货物
        // Log("robot start to tag goods");
        for(auto ite = model::goods.begin(); ite != model::goods.end(); ite++) {
            // 货物信息刷新
            ite->second.rest_frame--;
            if(ite->second.robot_id != -1) {
                continue;
            }

            std::priority_queue<std::pair<double, int>> robot_pq;
            for(int j = 0; j < model::robots.size(); ++j) {
                if(model::robots[j].have_goods || model::robots[j].goods_id != -1) {
                    // 已经分配了目标货物
                    continue;
                }

                if(model::robots[j].goods_pq.empty()) {
                    // 并没有能在货物消失前到达货物的路径
                    continue;
                }

                if(model::robots[j].goods_pq.top().second == ite->second.id) {
                    robot_pq.push(std::make_pair(model::robots[j].goods_pq.top().first, model::robots[j].id));
                    model::robots[j].goods_pq.pop();
                }
            }

            // 如果存在多个机器人优先级最高的货物是同一个, 那么比较对于货物来说最高优先级的机器人
            if(robot_pq.empty()) {
                // 并没有符合条件的机器人
                continue;
            }
            int robot_id = robot_pq.top().second;

            ite->second.robot_id = robot_id;
            model::robots[robot_id].goods_id = ite->second.id;
            if(_USE_LOG_) {
                Log("[info] goods [" + std::to_string(ite->second.id) + "] assign to robot [" + std::to_string(ite->second.robot_id) + 
                "]" + "path size: " + std::to_string(model::robots[robot_id].robot_2_goods[ite->second.id].size()));
            }
            // 清空对应的货物优先级队列
            std::priority_queue<std::pair<double, int>> tmp;
            model::robots[robot_id].goods_pq.swap(tmp);
        }

        // 标记泊位
        // Log("robot start to tag berth");
        for(int i = 0; i < model::berths.size(); ++i) {

            if(model::berths[i].robot_id != -1) {
                continue;
            }

            std::priority_queue<std::pair<double, int>> robot_pq;
            for(int j = 0; j < model::robots.size(); ++j) {
                if(!model::robots[j].have_goods || model::robots[j].target_berth != -1) {
                    // 没有货物或者已经分配了泊位
                    continue;
                }

                if(model::robots[j].berth_pq.empty()) {
                    // 并没有能在剩余帧前有能到达泊位的路径
                    continue;
                }

                if(model::robots[j].berth_pq.top().second == model::berths[i].id) {
                    robot_pq.push(std::make_pair(model::robots[j].berth_pq.top().first, model::robots[j].id));
                    model::robots[j].berth_pq.pop();
                }
            }

            // 如果存在多个机器人优先级最高的货物是同一个, 那么比较对于泊位来说最高优先级的机器人
            if(robot_pq.empty()) {
                // 并没有符合条件的机器人
                continue;
            }

            int robot_id = robot_pq.top().second;

            model::berths[i].robot_id = robot_id;
            model::robots[robot_id].target_berth = model::berths[i].id;

            // 虽然已经有机器人去的泊位还是可以分配机器人去
            // 但是berth附近有机器人会有极大的可能性发生碰撞, 尽量不要去同一个berth
            if(_USE_LOG_) {
                Log("[info] robot [" + std::to_string(model::robots[i].id) + "] generate berth path completed, target: " 
                + std::to_string(model::berths[i].id) + "path size: " + std::to_string(model::robots[i].robot_2_berth[model::berths[i].id].size()));
            }
            // 清空泊位优先级队列
            std::priority_queue<std::pair<double, int>> tmp;
            model::robots[robot_id].berth_pq.swap(tmp);
        }

        // 标记完成, 给出机器人行路指令
        // 只对有货物标记或者泊位标记的机器人发出移动指令, 其余的机器人随机移动
        // Log("robot start to move...");
        for(int i = 0; i < model::robots.size(); ++i) {
            if(!model::robots[i].have_goods) {
                if(model::robots[i].goods_id != -1) {
                    // 移动到货物
                    int goods_id = model::robots[i].goods_id;
                    if (model::robots[i].robot_2_goods[goods_id].empty()) {
                        printf("get %d\n", model::robots[i].id);

                        // 在货物列表清除货物
                        model::goods.erase(model::robots[i].goods_id);

                        // 重置状态
                        model::robots[i].have_goods = 1;
                        model::robots[i].goods_id = -1;
                        model::robots[i].next_location = Point(0, 0);

                        // 取了之后删除列表中的货物信息
                        model::goods.erase(goods_id);
                        if(_USE_LOG_) {
                            Log("[info] robot [" + std::to_string(model::robots[i].id) + "] get goods [" 
                            + std::to_string(goods_id) + "] success");
                        }
                    } else {
                        // TODO: 地图点位加锁
                        if(model::robots[i].status == 0) {
                            if(model::robots[i].next_location.x != 0 || model::robots[i].next_location.y != 0) {
                                model::robots[i].robot_2_goods[goods_id].push_front(model::robots[i].next_location);
                            }
                            continue;
                        }
                        Point p = model::robots[i].robot_2_goods[goods_id].front();
                        int dx = p.x - model::robots[i].location.x;
                        int dy = p.y - model::robots[i].location.y;
                        model::robots[i].next_location = p;
                        int move_id = -1;
                        if (dy == 0) {
                            // 上下移动
                            move_id = dx < 0 ? 2 : 3;
                        }

                        if (dx == 0) {
                            // 左右移动
                            move_id = dy < 0 ? 1 : 0;
                        }

                        printf("move %d %d\n", model::robots[i].id, move_id);

                        // 对已经走过的格子解锁
                        // model::current_map.init_map[model::robots[i].location.x][model::robots[i].location.y] = '.';

                        // 对未来要走的第五个格子加锁
                        // if(model::robots[i].robot_2_goods[goods_id].size() >= 5) {
                        //     auto ite = model::robots[i].robot_2_goods[goods_id].begin();
                        //     int k = ConstSetting::blocked_gird_num;
                        //     while(k > 0) {
                        //         Log(std::to_string(ite->x) + "," + std::to_string(ite->y) + " " + model::current_map.init_map[ite->x][ite->y]);
                        //         ite++;
                        //         k--;
                        //         model::current_map.init_map[ite->x][ite->y] = '#';
                        //     };
                        // }

                        if (_USE_LOG_) {
                            Log("[info] (" + std::to_string(model::robots[i].location.x) + "," + std::to_string(model::robots[i].location.y)
                            + ") " + model::current_map.init_map[model::robots[i].location.x][model::robots[i].location.y] + 
                            " -> (" + std::to_string(p.x) + "," + std::to_string(p.y) + ") " + model::current_map.init_map[p.x][p.y] +
                            " robot [" + std::to_string(model::robots[i].id) + "] move to: " + std::to_string(move_id) + " robot status: " + 
                            std::to_string(model::robots[i].status));
                        }

                        model::robots[i].robot_2_goods[goods_id].pop_front();
                    }
                } else {
                    // 没有货物分配, 随机移动
                    printf("move %d %d\n", model::robots[i].id, rand() % 4);
                }
            } else {
                if (model::robots[i].target_berth != -1) {
                    // 前往泊位
                    int berth_id = model::robots[i].target_berth;
                    // int goods_id = model::robots[i].goods_id;
                    if (model::robots[i].robot_2_berth[berth_id].empty()) {
                        printf("pull %d\n", model::robots[i].id);
                        // 重置机器人状态
                        model::robots[i].have_goods = 0;
                        model::robots[i].target_berth = -1;
                        model::robots[i].next_location = Point(0, 0);

                        // 泊位状态更新
                        model::berths[berth_id].goods_num++;
                        model::berths[berth_id].robot_id = -1;
                        // model::berths[berth_id].goods_value += model::goods[goods_id].price;

                        if(_USE_LOG_) {
                            Log("[info] robot [" + std::to_string(model::robots[i].id) + "] pull goods to berth [" 
                            + std::to_string(berth_id) + "] success");
                        }
                    } else {
                        // TODO: 地图点位加锁
                        if(model::robots[i].status == 0) {
                            if(model::robots[i].next_location.x != 0 || model::robots[i].next_location.y != 0) {
                                model::robots[i].robot_2_berth[berth_id].push_front(model::robots[i].next_location);
                            }
                            continue;
                        }
                        Point p = model::robots[i].robot_2_berth[berth_id].front();
                        int dx = p.x - model::robots[i].location.x;
                        int dy = p.y - model::robots[i].location.y;
                        model::robots[i].next_location = p;
                        int move_id = -1;
                        if (dy == 0) {
                            // 上下移动
                            move_id = dx < 0 ? 2 : 3;
                        }

                        if (dx == 0) {
                            // 左右移动
                            move_id = dy < 0 ? 1 : 0;
                        }

                        // 对已经走过的格子解锁
                        // model::current_map.init_map[model::robots[i].location.x][model::robots[i].location.y] = '.';
                        
                        // 对未来要走的五个格子加锁
                        // if(model::robots[i].robot_2_berth[berth_id].size() >= 5) {
                        //     auto ite = model::robots[i].robot_2_berth[berth_id].begin();
                        //     int k = ConstSetting::blocked_gird_num;
                        //     while(k > 0) {
                        //         ite++;
                        //         model::current_map.init_map[ite->x][ite->y] = '#';
                        //         k--;
                        //     }
                        // }

                        printf("move %d %d\n", model::robots[i].id, move_id);
                        if (_USE_LOG_) {
                            Log("[info] (" + std::to_string(model::robots[i].location.x) + "," + std::to_string(model::robots[i].location.y)
                            + ") " + model::current_map.init_map[model::robots[i].location.x][model::robots[i].location.y] + 
                            " -> (" + std::to_string(p.x) + "," + std::to_string(p.y) + ") " + model::current_map.init_map[p.x][p.y] +
                            " robot [" + std::to_string(model::robots[i].id) + "] move to: " + std::to_string(move_id) + "robot status: " + 
                            std::to_string(model::robots[i].status));
                        }
                        model::robots[i].robot_2_berth[berth_id].pop_front();
                    }

                } else {
                    // 没有目标泊位, 随机移动
                    printf("move %d %d\n", model::robots[i].id, rand() % 4);
                }
            }
        }

        
        // 轮船操作指令
        for(int i = 0; i < ConstVariable::berth_num; ++i) {
            Log("berth [" + std::to_string(model::berths[i].id) + "] goods num: " + std::to_string(model::berths[i].goods_num));
        }
        for(int i = 0; i < model::boats.size(); ++i) {
            int berth_id = model::boats[i].berth_id;
            if(berth_id == -1) {
                // 在虚拟点位, 要回到泊位
                // 泊位选取
                // std::priority_queue<std::pair<double, int>> boat_2_berth_pq;
                // for(int j = 0; j < model::berths.size(); ++j) {
                //     int loading_goods = std::min(model::berths[j].goods_num, model::boats[i].capacity);
                //     double berth_priority_value = model::berths[j].goods_value * 1.0 / 
                //                                 (model::berths[j].transport_time + model::berths[j].goods_num / model::berths[j].loading_speed);
                //     boat_2_berth_pq.push(std::make_pair(berth_priority_value, model::berths[j].id));
                // }
                // model::boats[i].berth_id = boat_2_berth_pq.top().second;
                // printf("ship %d %d\n", model::boats[i].id, model::boats[i].berth_id);
                // if (_USE_LOG_) {
                //     Log("[info] boats [" + std::to_string(model::boats[i].id) + "ship to berth [" + std::to_string(model::boats[i].berth_id)
                //     + "] need frame: " + std::to_string(model::berths[model::boats[i].berth_id].transport_time));
                // }

                for(int j = 0; j < ConstVariable::berth_num; ++j) {
                    if(model::berths[j].robot_id != -1 && model::berths[j].boat_id == -1) {
                        // 有机器人要去并且没有别的船去
                        printf("ship %d %d\n", model::boats[i].id, model::berths[j].id);

                        // 更新泊位状态
                        model::berths[j].boat_id = model::boats[i].id;

                        if (_USE_LOG_) {
                            Log("[info] boats [" + std::to_string(model::boats[i].id) + "] ship to berth [" + std::to_string(model::berths[j].id)
                            + "] need frame: " + std::to_string(model::berths[model::boats[i].berth_id].transport_time));
                        }
                    }
                }
            } else {
                // 在泊位, 决定是否需要运输货物
                // int loading_goods = std::min(model::berths[berth_id].goods_num, model::boats[i].capacity);
                // int need_frame = model::berths[berth_id].transport_time + 
                //                 loading_goods / model::berths[berth_id].loading_speed;
                
                // if(need_frame >= last_frame) {
                //     // 计算剩下的
                // }
                // 在泊位, 决定是否需要运输货物或者移动泊位
                if(model::berths[berth_id].goods_num > 0 ) {
                    // TODO: 运输货物: 有了就卖
                    if(model::boats[i].status == 1) {
                        if(model::boats[i].loading_time == -1) {
                            // 计算装货时间
                            int goods_num = std::min(model::berths[berth_id].goods_num, model::boats[i].capacity);
                            model::boats[i].loading_time = goods_num / model::berths[berth_id].loading_speed + 1;
                        } else if(model::boats[i].loading_time == 0) {
                            // 装货完成
                            printf("go %d\n", model::boats[i].id);
                            model::boats[i].loading_time = -1;
                            
                            // 泊位状态更新
                            int goods_num = std::min(model::berths[berth_id].goods_num, model::boats[i].capacity);
                            model::berths[berth_id].goods_num -= goods_num;
                            model::berths[berth_id].boat_id = -1;
                            if (_USE_LOG_) {
                                Log("[info] boats [" + std::to_string(model::boats[i].id) + "] leave berth [" + std::to_string(model::boats[i].berth_id)
                                + "] need frame: " + std::to_string(model::berths[model::boats[i].berth_id].transport_time));
                            }
                        } else {
                            // 继续装货
                            model::boats[i].loading_time--;
                        }
                    }
                } else {
                    // 没有货物
                    if(model::boats[i].status != 0) {
                        // 去别的泊位
                        for(int j = 0; j < ConstVariable::berth_num; ++j) {
                            if(model::berths[j].goods_num > 0 && model::berths[j].boat_id == -1) {
                                printf("ship %d %d\n", model::boats[i].id, model::berths[j].id);

                                // 更新泊位状态
                                model::berths[j].boat_id = model::boats[i].id;
                                if (_USE_LOG_) {
                                    Log("[info] boats [" + std::to_string(model::boats[i].id) + "] ship to berth [" + std::to_string(model::berths[j].id)
                                    + "] need frame: " + std::to_string(model::berths[model::boats[i].berth_id].transport_time));
                                }
                            }
                        }
                    }
                }
            }
        }

        std::cout << "OK" << std::endl;
        fflush(stdout);
    }
    // fout.close();
    // std::cerr.rdbuf(cerr_buf);
    return 0;
}
