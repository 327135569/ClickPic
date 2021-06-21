
## 介绍
这是一个基于 OpenCV 实现的自动化工具.


1. 基于 OpenCV 的模板匹配, 目前一张图在 1920x1080 分辨率下是 200ms 左右的检测速度.
1. 精简的操作逻辑封装, 你写的脚本可处理任何误操作.
1. 基于图片匹配, 因此适合各种使用系统控件和自绘类型程序的自动化.
1. 使用 LUA 快速编写自动化逻辑.
1. 驱动层模拟鼠标和键盘.


`目前驱动只适配了 Windows 7, Windows 10 64位, 要适配其他系统大概需要解决一下驱动层 ClassServiceCallback 的问题`


## Build
1. Install Visual Studio 2019
1. Install CMake
1. Install Vcpkg
1. Install Node.js
1. `vcpkg install opencv4 --triplet=x86-windows-static`
1. Add `VCPKG_ROOT` to env
1. Run `npm run test` to build this project

## Usage

1. First you need load the driver
1. Write you lua script, and do command

```
main.exe {screen width} {screen height} {lua file path} {pics path}
```

## C API
|      | 行为 |
| ---- | ---- |
| forceClick | 等待一个图出现然后点击它, 最多等待 15s, 超时抛出 ScriptTimeout 异常 |
| click | 点击一个图, 图不存在返回 false |
| dclick | 双击一个图, 图不存在返回 false |
| dforceClick | 等待一个图出现然后双击, 15s 超时, 抛出异常 |
| refreshScreen | 刷新屏幕图 |
| paste | Ctrl+V 的方式输入内容 |
| picExists | 判断图是否出现 |
| forceWait | 等待一个图出现 |


## Lua API
|      | 行为 |
| ---- | ---- |
| forceClick | 等待一个图出现然后点击它, 最多等待 15s, 超时抛出 ScriptTimeout 异常 |
| click | 点击一个图, 图不存在返回 false |
| dclick | 双击一个图, 图不存在返回 false |
| dforceClick | 等待一个图出现然后双击, 15s 超时, 抛出异常 |
| refreshScreen | 刷新屏幕图 |
| paste | 粘贴 |
| picExists | 判断图是否出现 |
| forceWait | 等待一个图出现 |
| sleep | 睡眠, 毫秒 |

## Thanks
1. https://github.com/nbqofficial/norsefire
1. https://github.com/SergiusTheBest/FindWDK