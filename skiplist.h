//#include <bits/stdc++.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>    // 引入互斥锁
#include <fstream>  // 引入文件操作

#define STORE_FILE "store/dumpFile"  // 存储文件路径

std::mutex mtx; // 定义互斥锁
std::string delimiter = ":";  // 定义键值对分隔符

template<typename K, typename V>
class Node{
public:
    Node(){}
    Node(K k, V v, int);
    ~Node();
    K get_key() const; //const??
    V get_value() const;
    void set_value(V);
    Node<K, V> **forwards; //该数组记录了该节点在每一层的下一个节点的指针。
    int node_level;

private:
    K key;
    V val;
    
    
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level){
    this->key = k;
    this->val = v;
    this->node_level = level;
    this->forwards = new Node<K, V>*[level + 1];  //指针数组
    //初始化 
    //for (int i = 0; i <= level; ++i) this->forwards[i] = nullptr;
    memset(this->forwards, 0, sizeof(Node<K, V>*) * (level + 1));
    
};


template<typename K, typename V>
Node<K, V>::~Node(){
    delete []forwards;
}

template<typename K, typename V>
K Node<K, V>::get_key() const{
    return key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const{
    return val;
}

template<typename K, typename V>
void Node<K, V>::set_value(V value){
    this->val = value;
}


template <typename K, typename V>  //泛型编程,SkipList 可以被实例化为任何具有两个类型参数的类
class SkipList{
public:
    SkipList(){};
    SkipList(int);
    ~SkipList();
    Node<K, V> *createNode(K, V, int);  //节点创建
    int getRandomLevel();  //层级分配
    bool searchElement(K);  //节点搜索
    int insertElement(K, V);  //节点插入
    void deleteElement(K);  //节点删除
    void displayList();  //节点展示
    int size();  //节点计数,获取节点数量
    void dumpFile();  //数据持久化
    void loadFile();  //数据加载
    void clear(Node<K, V> *);  //垃圾回收

private:
    Node<K, V> * header;  //头节点
    int maxLevel;  //最大层数
    int skipListLevel;  //当前层数(最高)
    int elementCount;  //节点数量
    std::ofstream fileWriter;  //文件读写
    std::ifstream fileReader;

private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);  // 从字符串获取键值对
    bool is_valid_string(const std::string& str);  // 验证字符串是否有效
};

template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level)  //确定属于哪个模板实例
{
    this->maxLevel = max_level;
    this->skipListLevel = 0;
    this->elementCount = 0;
    K k;  
    V v;
    // 默认值初始化键，占位作用（不保存实际的数据）
    // 在实际实现中，这个默认键值通常不会参与实际的跳表操作，因此其值并不重要。
    this->header = new Node<K, V>(k, v, maxLevel);
}

template <typename K, typename V>
Node<K, V> *SkipList<K, V>::createNode(const K k, const V v, int level)
{
    Node<K, V> *node = new Node<K, V>(k, v, level);
    return node;
}

template <typename K, typename V>
int SkipList<K, V>:: getRandomLevel(){
    int k = 1;
    while (rand() % 2)
    {
        k++;
    }
    k = (k < maxLevel) ? k : maxLevel;
    return k;
}

template <typename K, typename V>
bool SkipList<K, V>:: searchElement(K k)
{
    Node<K, V> *cur = header;
    for (int i = skipListLevel; i >= 0; i--)  // 从跳表当前的最高层开始搜索？？
    {
        while(cur->forwards[i] && cur->forwards[i]->get_key() < k)
        {
            cur = cur->forwards[i];
        }
    }
    cur = cur->forwards[0];
    if (cur && cur->get_key() == k) return true;
    else return false;
}

template <typename K, typename V>
int SkipList<K, V>::insertElement(const K k, const V v)
{
    mtx.lock();
    //1. 确定节点层级
    //2. 寻找插入位置
    //3. 更新指针关系
    Node<K, V> *cur = this->header;
    Node<K, V> *update[maxLevel + 1]; //记录每层待更新指针的(前驱)节点
    memset(update, 0, sizeof(Node<K, V> *) * (maxLevel + 1));
    //搜索插入位置
    for (int i = skipListLevel; i >= 0; i--)
    {
        while (cur->forwards[i] && cur->forwards[i]->get_key() < k)
        {
            cur = cur->forwards[i];
        }
        update[i] = cur; //保存每层待更新节点
    }
    cur = cur->forwards[0];
    if (cur && cur->get_key() == k) 
    {
        mtx.unlock();
        return 1;
    }
    if (!cur || cur->get_key() != k) 
    {
        int curLevel = getRandomLevel();
        if (curLevel > skipListLevel)
        {
            // 对所有新的更高层级，将头节点设置为它们的前驱节点
            for (int i = skipListLevel + 1; i <= curLevel; i++)
            {
                update[i] = header;
            }
            skipListLevel = curLevel;
        }
        Node<K, V> *insertNode = createNode(k, v, curLevel);
        //插入节点并更新指针
        for (int i = 0; i <= curLevel; i++)
        {
            insertNode->forwards[i] = update[i]->forwards[i];
            update[i]->forwards[i] = insertNode;
        }
        ++elementCount;
    }
    mtx.unlock();
    return 0;
    
}

template <typename K, typename V>
void SkipList<K, V>::deleteElement(const K k)
{
    mtx.lock();
    //1. 定位删除节点位置
    //2. 更新指针关系
    //3. 释放内存
    Node<K, V> *cur = this->header;
    Node<K, V> *update[maxLevel + 1]; //记录每层待删除节点的前驱
    memset(update, 0, sizeof(Node<K, V> *) * (maxLevel + 1));
    //定位删除节点位置
    for (int i = skipListLevel; i >= 0; i--)
    {
        while (cur->forwards[i] && cur->forwards[i]->get_key() < k)
        {
            cur = cur->forwards[i];
        }
        update[i] = cur; //保存每层待更新节点
    }
    cur = cur->forwards[0];
    if (cur && cur->get_key() == k) 
    {
        //从下往上更新并删除节点
        for (int i = 0; i <= skipListLevel; i++)
        {
            if (update[i]->forwards[i] != cur) break;
            update[i]->forwards[i] = cur->forwards[i];
        }
        // 调整跳表的层级
        while (skipListLevel > 0 && header->forwards[skipListLevel] == nullptr)
        {
            skipListLevel--;
        }
        delete cur;
        --elementCount;
    }
    mtx.unlock();
    return;
}

template<typename K, typename V>
void SkipList<K, V>::displayList(){
    for (int i = 0; i <= skipListLevel; i++)  //从0开始，不然发生错误段错误 (核心已转储)
    {
        Node<K, V>* node = this->header->forwards[i];
        std::cout << "Level" << i << ":";
        while(node)
        {
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forwards[i];
        }
        std::cout << std::endl;
    }
}


//数据持久化
template<typename K, typename V>
void SkipList<K, V>::dumpFile()
{
    fileWriter.open(STORE_FILE);
    Node<K, V>* node = header->forwards[0];
    while (node)
    {
        fileWriter << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forwards[0];
    }
    
    fileWriter.flush();
    fileWriter.close();
 }

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string &str)
{
    return !str.empty() && str.find(delimiter) !=  std::string::npos; //如果查找失败，返回string::npos
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value)
{
    if (!is_valid_string(str)) return ;
    *key = str.substr(0, str.find(delimiter));
    //??
    //*value = str.substr(str.find(delimiter) + 1, str.size() - (str.find(delimiter) + 1 - str.begin()));
    *value = str.substr(str.find(delimiter) + 1, str.size() - str.size());
}


template<typename K, typename V>  //数据加载
void SkipList<K, V>::loadFile()
{
    fileReader.open(STORE_FILE);
    std::string line;
    std::string *key = new std::string();
    std::string *value = new std::string();
    
    while (getline(fileReader, line))
    {
        get_key_value_from_string(line, key, value);
        if(key->empty() || value->empty()) continue;
        
        insertElement(stoi(*key), *value);
        std::cout << "key:" << *key << "value" << *value << std::endl;
    }
    
    delete key;
    delete value;
    fileReader.close();
}

template<typename K, typename V>//节点计数,获取节点数量
int SkipList<K, V>::size()
{
    return elementCount;
} 

template<typename K, typename V> //垃圾回收  
void SkipList<K, V>::clear(Node<K, V> * cur)
{
    if (cur->forwards[0])
    {
        clear(cur->forwards[0]);
    }
    delete(cur);
} 

// 析构函数
template<typename K, typename V> 
SkipList<K, V>::~SkipList() {

    if (fileWriter.is_open()) {
        fileWriter.close();
    }
    if (fileReader.is_open()) {
        fileReader.close();
    }

    //递归删除跳表链条
    if(header->forwards[0]!=nullptr){
        clear(header->forwards[0]);
    }
    delete(header); 
}












// int main()
// {
//     int N, K, M;
//     std::cin >> N >> K >> M;
//     SkipList<int, int> *skiplist = new SkipList<int, int>(10);
//     for (int i = 0; i < N; i++)
//     {
//         int k, v;
//         std::cin >> k >> v;
//         if (skiplist->insertElement(k, v) == 0)
//         {
//             std::cout << "Insert Success" << std::endl;
//         } 
//         else 
//         {
//             std::cout << "Insert Failed" << std::endl;
//         }
//     }
//     for (int i = 0; i < K; i++)
//     {
//         int k;
//         std::cin >> k ;
//         skiplist->deleteElement(k);
//     }
//     for (int i = 0; i < M; i++)
//     {
//         int k;
//         std::cin >> k;
//         if (skiplist->searchElement(k))
//         {
//             std::cout << "Search Success" << std::endl;
//         } 
//         else 
//         {
//             std::cout << "Search Failed" << std::endl;
//         }
//     }
//     return 0;
// }





