# Suduo
一个基于[muduo](https://github.com/chenshuo/muduo)改造而来的`Reactor`模型的多线程网络库。

## 简介
这是一个基于`muduo`改造而来的网络库。主要的改动：
- 用更多的标准库组件替换了之前手写的实现和 C 的函数
- 更多的使用了`unique_ptr`和`shared_ptr`来管理堆上的对象
- 在不损失性能的情况下，简化了一些东西的实现
- 替换了命名的风格

继承了`muduo`的所有的组件的测试和性能测试项目
## 环境
`suduo`是在以下的环境里构建的：

系统：WSL2 Arch Linux

编译器：GCC 11.2.0

构造工具：cmake 3.22.2

C++17
## 构建
`suduo`本身不需要任何的额外库，但是如果想要使用`bench`做性能测试或者`tests`做组件测试，则需要引入`boost`

`bench`和`tests`默认是被注释掉关闭的，如有需要可以手动打开。`tests`需要在`net`和`base`里分别打开

可以直接使用`cmake`构建，然后使用生成到`build/bin`里的静态库

或者直接将该项目引入到你需要的项目里

## 性能
结论：在`muduo`的所有性能测试中，`suduo`都表现出了至少持平，部分情况下略优的性能表现

详细的性能分析，等时间更充足时再补上

## 致谢
感谢[chenshuo](https://github.com/chenshuo)的[muduo](https://github.com/chenshuo/muduo)项目