# Skiplist-CPP


## 提供接口

* insertElement（插入数据）
* deleteElement（删除数据）
* searchElement（查询数据）
* displayList（展示已存数据）
* dumpFile（数据落盘）
* loadFile（加载数据）
* size（返回数据规模）


## 项目运行方式

```
make            // complie demo main.cpp
./bin/main      // run

sh stress_test_start.sh  // stress test
```

## 可优化的点

1.LRU 定时删除、定期删除和惰性删除的过期删除策略
2.rdb数据存盘机制，实现AOF机制
3.LSM结构
...


>参考程序员Carl
