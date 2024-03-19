#ifndef __ALGORITHM_TOOLS_H__
#define __ALGORITHM_TOOLS_H__

#include <vector>
#include <string>
#include <string.h>
#include <queue>
#include <algorithm>
#include <list>
#include <unordered_map>
#include <iostream>
#include "model.h"
#include "const_variable.h"
#include "args.h"
#include "log.h"

namespace AlgorithmTools {

// 迪杰斯特拉算法寻找最短路径
int minPathSum(const std::vector<std::string>& map, Point& start, Point& end, std::list<Point>& path) {
    
    std::vector<std::vector<int>> dist(ConstVariable::map_row_num + 2, std::vector<int>(ConstVariable::map_col_num + 2, INT_MAX));
    std::vector<std::vector<Point>> prev(ConstVariable::map_row_num + 2, std::vector<Point>(ConstVariable::map_col_num + 2, {-1, -1}));

    auto compare = [](const std::pair<int, Point>& a, const std::pair<int, Point>& b) {
        return a.first > b.first;
    };

    dist[start.x][start.y] = 0;

    std::priority_queue<std::pair<int, Point>, std::vector<std::pair<int, Point>>, decltype(compare)> pq(compare);
    pq.push({0, start});

    std::vector<int> dx = {-1, 1, 0, 0};
    std::vector<int> dy = {0, 0, -1, 1};

    while (!pq.empty()) {
        Point curr = pq.top().second;
        int curr_dist = pq.top().first;
        pq.pop();

        if (curr.x == end.x && curr.y == end.y) {
            Point p = end;
            while (p.x != -1 && p.y != -1) {
                path.push_back(p);
                p = prev[p.x][p.y];
            }
            std::reverse(path.begin(), path.end());
            path.pop_front();
            return path.size();
        }

        for (int i = 0; i < 4; ++i) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if (nx >= 1 && nx <= ConstVariable::map_row_num && ny >= 1 && ny <= ConstVariable::map_col_num) {
                if(map[nx][ny] == '#' || map[nx][ny] == '*') {
                    continue;
                }
                int newDist = curr_dist + 1;
                if (newDist < dist[nx][ny]) {
                    dist[nx][ny] = newDist;
                    pq.push({newDist, {nx, ny}});
                    prev[nx][ny] = curr;
                }
            }
        }
    }

    return -1;
}

int calculateMDistance(Point& p1, Point& p2) {
    return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y);
}

// A*算法寻找最短路径
int findMinPath(const std::vector<std::string>& map, Point& start, Point& end, std::list<Point>& path) {
    auto compare = [](const std::pair<int, Point>& a, const std::pair<int, Point>& b) {
        return a.first > b.first;
    };

    std::priority_queue<std::pair<int, Point>, std::vector<std::pair<int, Point>>, decltype(compare)> frontier(compare);
    std::unordered_map<Point, int> cost_so_far;
    std::unordered_map<Point, Point> came_from;

    std::vector<int> dx = { -1, 1, 0, 0 };
    std::vector<int> dy = { 0, 0, -1, 1 };

    frontier.push(std::make_pair(0, start));

    while (!frontier.empty()) {
        Point current = frontier.top().second;
        frontier.pop();

        if (current == end) {
            break;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];
            if (nx < 0 || nx >= map.size() || ny < 0 && ny >= map[0].size() || map[nx][ny] == '#' || map[nx][ny] == '*') {
                // 越界或者是障碍
                continue;
            }
            Point next(nx, ny);
            int new_cost = cost_so_far[current] + 1;
            if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) {
                cost_so_far[next] = new_cost;
                int priority = new_cost + calculateMDistance(next, end);
                frontier.push(std::make_pair(priority, next));
                came_from[next] = current;
            }
        }
    }

    // 因为泊位是一个区域，给定的终点可能是最先到达的也可能是先到了泊位区域内的其他点
    // 因此将路径中多余的'B'点去除
    Point p = end;
    Point tmp = p;
    while(map[p.x][p.y] == 'B') {
        tmp = p;
        p = came_from[tmp];
    }

    path.push_back(tmp);
    while (true) {
        p = came_from[tmp];
        tmp = p;
        
        if (p == start) {
            break;
        }
        path.push_back(p);
    }

    std::reverse(path.begin(), path.end());
    // path.push_back(tmp);

    return cost_so_far[end];
}

double get_goods_priority_value(int need_frame, Goods& target_goods) {
    if(need_frame > target_goods.rest_frame) {
        return 0.0;
    }
    return target_goods.price * 1.0 / need_frame;
}

double get_berth_priority_value(int need_frame, Berth& target_berth, int rest_frame) {
    // 优先级：货物数量+装货速度
    // 尽可能将货物集中到几个港口
    if(need_frame + target_berth.transport_time > rest_frame) {
        return 0.0;
    }
    int total_frame = need_frame + target_berth.transport_time;
    return (target_berth.goods_num + target_berth.loading_speed * 1.0) / total_frame;
}

}

#endif