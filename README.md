### 机器人控制

#### 标记

* 机器人状态为未携带货物，对要前往的货物点进行标记。

* 机器人状态为携带货物, 对要前往的泊点进行标记，标记后其他机器人不可以选择该泊点。

* have_goods = 0 -- 取货优先级算法: goods_priority_value 
```
to_goods_frame > goods_live_frame => 0 

priority_value = price / to_goods_frame

根据priority_value生成货物优先级队列, 每个机器人维护自己的货物优先级队列

如果货物已经被其他机器人标记，则顺移至未被标记的货物, 直至队列为空
```

* have_goods = 1 -- 泊位优先级算法：berth_priority_value 
```

根据berth_priority_value生成泊位优先级队列, 每个机器人维护自己的泊位优先级队列

to_berth_frame + transport_time > last_frame => 0

berth_priority_value = (last_goods_price + robot_goods_price) / (goods_sum_num / loading_speed) * (transport_time + to_berth_frame)

如果泊位已经被其他机器人标记，则顺移至未被标记的泊位, 直至队列为空
```

* 问题：如何在有墙的条件下计算to_goods_frame 和 to_berth_frame, 是否需要考虑碰撞?
* 解决：在计算优先级的时候先不考虑碰撞, 机器人行路过程检测四周格子是否存在其他机器人，如果存在，转弯让直行
        路径费用计算: 迪杰斯特拉

#### 碰撞避免
* 简单的碰撞避免: 机器人对将要移动的地图的点位进行加锁，检测点位是否加锁，加锁了就等待一帧
* 与墙壁或海碰撞：机器人的移动目标位置和墙壁重合
* 与机器人碰撞：两个机器人的目标位置重合

### 轮船控制

#### 指定船移动到泊位

#### 指定船