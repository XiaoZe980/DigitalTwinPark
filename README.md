# DigitalTwinPark - 智慧园区数字孪生

基于 **Unreal Engine 5.8** 的智慧园区数字孪生展示系统。支持 3D 可视化、IoT 数据面板、天气系统、交通模拟，覆盖桌面/大屏/Web/移动端多平台交互。

![UE5.8](https://img.shields.io/badge/UE-5.8-blue)
![C++](https://img.shields.io/badge/C++-20-orange)
![License](https://img.shields.io/badge/License-MIT-green)

---

## 功能特性

| 模块 | 功能 |
|------|------|
| **建筑管理** | DataAsset 驱动配置，点击选中、高亮、实时数据弹窗 |
| **数据面板** | UMG 实时仪表盘，支持模拟数据和 HTTP REST API 两种数据源 |
| **天气系统** | 晴天/多云/雨/雪切换，Niagara 粒子特效 |
| **昼夜循环** | 24 小时光照模拟，可暂停/调速 |
| **交通模拟** | Spline 路径车流，车辆随机生成 |
| **多相机模式** | 自由漫游、预设视角、建筑聚焦、鸟瞰，SmoothStep 飞行动画 |
| **多平台交互** | 桌面键鼠、大屏触控、Pixel Streaming Web、移动端 |

---

## 技术架构

```
数据层
├── IDTPDataProvider (策略接口)
│   ├── DTPMockDataProvider   ← 模拟数据
│   └── DTPHttpDataProvider   ← HTTP REST API
└── UDTPDataSubsystem         ← 缓存 + 广播

展示层
├── UMG Widgets (数据绑定)
├── ADTPBuildingActor (场景建筑)
└── ADTPPlayerController (输入)

环境层
├── ADTPWeatherManager
├── ADTPDayNightCycle
└── ADTPTrafficManager
```

## 项目结构

```
Source/DigitalTwinPark/
├── Core/          游戏框架 (GameMode, PlayerController, Pawn)
├── Data/          数据层 (类型定义, 数据源, 子系统)
├── Building/      建筑系统 (Actor, DataAsset, Manager)
├── UI/            UMG Widget 基类
├── Camera/        相机系统 (预设, 飞行动画)
├── Environment/   环境系统 (天气, 昼夜, 交通)
└── Interaction/   输入适配
```

---

## 快速开始

### 环境要求

- **Unreal Engine 5.8**
- **Visual Studio 2022**
- **Windows 10/11**

### 运行

1. 克隆仓库
   ```bash
   git clone https://github.com/XiaoZe980/DigitalTwinPark.git
   ```

2. 双击 `DigitalTwinPark.uproject` 打开项目

3. 首次打开会提示编译，选 **Yes**

4. 在编辑器中运行 `init_dtp.py` 一键创建蓝图：
   - 菜单 → **Tools** → **Execute Python Script**
   - 选择 `Content/init_dtp.py`

### 数据源切换

在 `DTPGameInstance` 中调用：

```cpp
// 模拟数据（默认）
GetDataSubsystem()->UseMockData();

// HTTP 真实数据
GetDataSubsystem()->UseHttpData("http://your-api:8080/api");
```

---

## 开发计划

- [x] C++ 框架搭建
- [x] 数据层实现（Mock + HTTP）
- [x] 建筑系统
- [x] 相机系统
- [x] 环境系统
- [ ] Input Actions 配置
- [ ] UMG 界面设计
- [ ] Pixel Streaming 部署
- [ ] 3D 模型导入

---

## License

MIT