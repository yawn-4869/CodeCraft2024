#include <stdio.h>
#include <iostream>
#include <fstream>
#include "src/model.h"
#include "src/algorithm_tools.h"
#include "src/args.h"
#include "src/io_interface.h"
#include "src/log.h"
int boat_capacity;

void Init()
{
    // 初始化地图
    int cnt = 0; // 记录机器人id
    for(int i = 1; i <= ConstVariable::map_row_num; ++i) {
        char tmp[ConstVariable::map_col_num + 2];
        scanf("%s", tmp);
        model::current_map.init_map[i] = std::string(tmp);
        // 12页说明，帧数为1时给出机器人位置为机器人初始位置
        // TODO: 那么, 初始化机器人地图时候上面的机器人位置是无效的？
        // for(int j = 1; j <= ConstVariable::map_col_num; ++j) {
        //     char c = model::current_map.init_map[i][j];
        //     // 初始化
        //     if(c == 'A') {
        //         Robot robot(i, j);
        //         robot.id = cnt;
        //         model::robots[cnt] = robot;
        //         //model::robot_index[robot.location] = cnt;
        //         cnt++;
        //     }
        // }
    }

    // 初始化机器人数组
    model::robots.resize(ConstVariable::robot_num);
    
    // 初始化泊位数组
    model::berths.resize(ConstVariable::berth_num);
    for(int i = 0; i < ConstVariable::berth_num; ++i) {
        Berth berth;
        scanf("%d%d%d%d%d", &berth.id, &berth.location.x, &berth.location.y, &berth.transport_time, &berth.loading_speed);
        model::berths[i] = berth;
    }

    // 初始化船容量
    scanf("%d", &boat_capacity);
    model::boats.resize(ConstVariable::boat_num);

    if(_USE_LOG_) {
        std::cerr << "init success" << std::endl;
    }

    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}

int Input()
{
    int frame, money;
    scanf("%d%d", &frame, &money);
    model::current_map.current_frame = frame;
    model::current_map.current_money = money;

    // 货物信息
    int goods_num;
    scanf("%d", &goods_num);

    for(int i = 0; i < goods_num; ++i) {
        int x, y, val;
        scanf("%d%d%d", &x, &y, &val);
        Goods good(x, y, val);
        good.id = model::current_map.goods_num + i;
        model::goods.insert(std::make_pair(good.id, good));
    }

    model::current_map.goods_num += goods_num;

    for(auto ite = model::goods.begin(); ite != model::goods.end();) {
        if(ite->second.rest_frame == 0) {
            ite = model::goods.erase(ite);
        } else {
            ite++;
        }
    }

    // 机器人信息
    for(int i = 0; i < ConstVariable::robot_num; i ++) {
        // int is_good, x, y, sts;
        // scanf("%d%d%d%d\n", &is_good, &x, &y, &sts);
        // Robot robot(x, y);
        // robot.id = i;
        // robot.have_goods = is_good;
        // robot.status = sts;
        // model::robots[i] = robot;
        if(frame != 1) {
            scanf("%d%d%d%d\n", &model::robots[i].have_goods, &model::robots[i].location.x, 
            &model::robots[i].location.y, &model::robots[i].status);
        } else {
            int have_goods, x, y, sts;
            Robot robot;
            scanf("%d%d%d%d\n", &robot.have_goods, &robot.location.x, &robot.location.y, &robot.status);
            robot.id = i;
            robot.goods_id = -1;
            robot.target_berth = -1;
            model::robots[i] = robot;
        }
    }

    // 船信息
    for(int i = 0; i < ConstVariable::boat_num; i ++) {
        Boat boat;
        scanf("%d%d\n", &boat.status, &boat.berth_id);
        boat.id = i;
        boat.capacity = boat_capacity;
        model::boats[i] = boat;
    }

    char okk[100];
    scanf("%s", okk);
    return frame;
}

int main() {
    // 日志输出
    // auto cerr_buf = std::cerr.rdbuf();
    // std::fstream fout("./log.txt", std::ios::out);
    // if (fout.is_open())
    //     std::cerr.rdbuf(fout.rdbuf());
    // else
    //     std::cerr << "[error] log file open failed" << std::endl;
    
    // io::init(std::cin);
    // fflush(stdout);
    Init();
    
    for(int curr_frame = Input(); curr_frame <= ConstVariable::total_frame; curr_frame = Input()) {
        // int curr_frame = Input();
        int last_frame = ConstVariable::total_frame - curr_frame;
        Log("current frame: " + std::to_string(curr_frame));

        // 生成货物优先队列及泊位优先队列
        Log("test1");
        for(int i = 0; i < ConstVariable::robot_num; ++i) {
            if(!model::robots[i].have_goods) {
                if(model::robots[i].goods_id == -1) {
                    // 没有货物且也没有标记, 给机器人分配货物
                    for(auto ite = model::goods.begin(); ite != model::goods.end(); ite++) {
                        if(ite->second.robot_id != -1) {
                            continue;
                        }

                        std::list<Point> path;
                        int need_frame = AlgorithmTools::findMinPath(model::current_map.init_map, model::robots[i].location, 
                        ite->second.location, path);

                        if(need_frame == 0) {
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
                if(model::robots[i].target_berth == -1) {
                    // 已有货物但是没有目标泊位, 给机器人分配泊位
                    for(int j = 0; j < model::berths.size(); ++j) {
                        std::list<Point> path;
                        int need_frame = AlgorithmTools::findMinPath(model::current_map.init_map, model::robots[i].location, 
                        model::berths[j].location, path);

                        if(need_frame == 0) {
                            continue;
                        }

                        int goods_id = model::robots[i].goods_id;
                        double berth_priority_value = AlgorithmTools::get_berth_priority_value(model::goods[goods_id].price, need_frame, 
                        model::berths[j], last_frame);
                        if(berth_priority_value) {
                            model::robots[i].robot_2_berth[model::berths[j].id] = path;
                            model::robots[i].berth_pq.push(std::make_pair(berth_priority_value, model::berths[j].id));
                        }
                    }
                    // 机器人标记泊位, 已经有机器人去的泊位还是可以分配机器人去
                    // 所以不需要像货物一样进行二次筛选
                    int target_berth = model::robots[i].berth_pq.top().second;
                    model::robots[i].target_berth = target_berth;
                }
            }
        }

        // 机器人标记货物
        Log("test2");
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
                Log("[info] goods [" + std::to_string(ite->second.id) + "] assign to robot [" 
                + std::to_string(ite->second.robot_id) + "]");
            }
        }

        // 标记完成, 给出机器人行路指令
        // 只对有货物标记或者泊位标记的机器人发出移动指令, 其余的机器人原地等待
        Log("test3");
        for(int i = 0; i < model::robots.size(); ++i) {
            if(!model::robots[i].have_goods) {
                if(model::robots[i].goods_id != -1) {
                    // 移动到货物
                    int goods_id = model::robots[i].goods_id;
                    if (model::robots[i].robot_2_goods[goods_id].empty()) {
                        printf("get %d\n", model::robots[i].id);

                        // 重置状态
                        model::robots[i].have_goods = 1;
                        model::robots[i].goods_id = -1;

                        // 取了之后删除列表中的货物信息
                        model::goods.erase(goods_id);
                        if(_USE_LOG_) {
                            Log("[info] robot [" + std::to_string(model::robots[i].id) + "] get goods [" 
                            + std::to_string(goods_id) + "] success");
                        }
                    } else {
                        // TODO: 地图点位加锁
                        if(model::robots[i].status == 0) {
                            continue;
                        }
                        Point p = model::robots[i].robot_2_goods[goods_id].front();
                        int dx = p.x - model::robots[i].location.x;
                        int dy = p.y - model::robots[i].location.y;
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
                        
                        Log("[info] [current point]: (" + std::to_string(model::robots[i].location.x) + "," + std::to_string(model::robots[i].location.y)
                        + ") [next point]: (" + std::to_string(p.x) + "," + std::to_string(p.y) + ") robot [" + 
                        std::to_string(model::robots[i].id) + "] move to: " + std::to_string(move_id) + "robot status: " + 
                        std::to_string(model::robots[i].status));

                        model::robots[i].robot_2_goods[goods_id].pop_front();
                    }
                } else {
                    // 没有货物分配, 随机移动
                    printf("move %d %d\n", model::robots[i].id, rand() % 4);
                }
            }

            if(model::robots[i].have_goods) {
                if (model::robots[i].target_berth != -1) {
                    // 前往泊位
                    int berth_id = model::robots[i].target_berth;
                    if (model::robots[i].robot_2_berth[berth_id].empty()) {
                        printf("pull %d\n", model::robots[i].id);
                        // 重置状态
                        model::robots[i].have_goods = 0;
                        model::robots[i].target_berth = -1;

                        if(_USE_LOG_) {
                            Log("[info] robot [" + std::to_string(model::robots[i].id) + "] pull goods to berth [" 
                            + std::to_string(berth_id) + "] success");
                        }
                    } else {
                        // TODO: 地图点位加锁
                        if(model::robots[i].status == 0) {
                            continue;
                        }
                        Point p = model::robots[i].robot_2_berth[berth_id].front();
                        int dx = p.x - model::robots[i].location.x;
                        int dy = p.y - model::robots[i].location.y;
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
                        if (_USE_LOG_) {
                            Log("[info] [current point]: (" + std::to_string(model::robots[i].location.x) + "," + std::to_string(model::robots[i].location.y)
                            + ") [next point]: (" + std::to_string(p.x) + "," + std::to_string(p.y) + ") robot [" + 
                            std::to_string(model::robots[i].id) + "] move to: " + std::to_string(move_id) + "robot status: " + 
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
        for(int i = 0; i < model::boats.size(); ++i) {
            int berth_id = model::boats[i].berth_id;
            if(berth_id == -1) {
                // 在虚拟点位, 要回到泊位
                // 泊位选取
                std::priority_queue<std::pair<double, int>> boat_2_berth_pq;
                for(int j = 0; j < model::berths.size(); ++j) {
                    int loading_goods = std::min(model::berths[j].goods_num, model::boats[i].capacity);
                    double berth_priority_value = model::berths[j].goods_value * 1.0 / 
                                                (model::berths[j].transport_time + model::berths[j].goods_num / model::berths[j].loading_speed);
                    boat_2_berth_pq.push(std::make_pair(berth_priority_value, model::berths[j].id));
                }
                model::boats[i].berth_id = boat_2_berth_pq.top().second;
                printf("ship %d %d\n", model::boats[i].id, model::boats[i].berth_id);
            } else {
                // 在泊位, 决定是否需要运输货物
                // TODO: 目前算法, 满了才运输
                // int loading_goods = std::min(model::berths[berth_id].goods_num, model::boats[i].capacity);
                // int need_frame = model::berths[berth_id].transport_time + 
                //                 loading_goods / model::berths[berth_id].loading_speed;
                
                // if(need_frame >= last_frame) {
                //     // 计算剩下的
                // }

                if(model::berths[berth_id].goods_num >= model::boats[i].capacity) {
                    printf("go %d\n", model::boats[i].id);
                }

            }
        }

        puts("OK");
        fflush(stdout);
    }
    // fout.close();
    // std::cerr.rdbuf(cerr_buf);
    return 0;
}