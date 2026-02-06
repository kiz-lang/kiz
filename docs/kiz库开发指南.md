# kiz库开发指南
**kiz**当前库比较缺乏，邀请您一起为kiz开发库

## 使用C++为kiz开发库
## 创建命名空间
以库名+lib命名
```
namespace xx_lib
```
## 创建库函数
```
#include "models.hpp" // 必备, 定义了模块
#include "kiz.hpp"  // 可选，用于库内抛出异常
#include "vm.hpp" // 可选, 包含call_method, is_true, obj_to_str, obj_to_debug_str等实用函数
namespace xx_lib {
model::Object* foo(model::Object* self, model::List* args) {
    /// 请在库函数中使用这样的函数签名
    /// 您不需要在函数中管理引用计数, kiz会帮你解决

    /// 抛出异常
    throw NativeFuncError("ErrName", "ErrMag");
}

}
```

model中功能拆解


## 使用kiz为kiz开发库