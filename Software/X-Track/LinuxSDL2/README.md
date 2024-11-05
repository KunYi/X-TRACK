# X-TRACK Linux SDL2

## 编译

使用 make
```sh
make -jN　# N 是整数，代表使用多少任务进行平行编译，一般为系统核心数目
```

使用 cmake & ninja
```sh
cmake -B build -S . -GNinja
cmake --build build
```

## 执行

```sh
./xtrack
```

## 其他说明

* 1.可以通过修改Makefile的`LV_COLOR_DEPTH`配置屏幕的颜色深度，默认32bpp。
* 2.关机自动保存功能不支持。
* 3.轨迹记录功能未测试，可能不支持。

## Demo
![image](./media/demo_sdl.png)
