# json
C++ json
```cpp
#include <iostream>
#include "../include/json.h"

using namespace ty;

// ...

int main(int argc, char **argv)
{
  std::string json_str = "{\"a\":100023 , \"b\": true, \"c\":\"\\uD83D\\uDE00\\uFF08-\\u3002-\\uFF09\\uD83D\\uDE00\", \"d\":[1.01, \"OK是好的\", 33]}";
  json v = json::parse(json_str);
  int number = v["a"];
  if (v["b"])
  {
      std::cout << "v[\"b\"] is true" << std::endl;
  }
  else
  {
      std::cout << "v[\"b\"] is false" << std::endl;
  }
  
  std::cout << "number = " << number << std::endl;
  number = v["d"][2];
  std::cout << "number = " << number << std::endl;
  
  std::cout << "v[\"a\"] = " << v["a"] << std::endl;
  v["a"] = "C++ json";
  std::cout << "v[\"a\"] = " << v["a"] << std::endl;
  
  std::cout << "v:" << std::endl;
  std::cout << v << std::endl;
  
  json v2 = {
      json::pair("a", "这是a"),
      json::pair("b", "这是b"),
      json::pair("c", 0.001),
      json::pair("d", {1, 2, 3, 4, "第5个", "第6个"}),
  };
  
  std::cout << "v2:" << std::endl;
  std::cout << v2 << std::endl;
}
```
output：
```
v["b"] is true
number = 100023
number = 33
v["a"] = 100023
v["a"] = C++ json
v:
key: c value: 😀（-。-）😀
key: b value: 1
key: d value: 1.01 OK是好的 33 
key: a value: C++ json

v2:
key: c value: 0.001
key: b value: 这是b
key: d value: 1 2 3 4 第5个 第6个 
key: a value: 这是a

```
