# DigitalTwinPark 开发交接文档

> 智慧园区数字孪生 Demo · UE 5.8 · 最后更新 2026-09-24

---

## 一、项目基本信息

| 项 | 值 |
|---|---|
| 项目路径 | `E:/UnrealProjects/DigitalTwinPark` |
| 引擎版本 | **UE 5.8**，安装路径 `E:/UnrealVersion/UE_5.8`（非默认盘） |
| 仓库 | https://github.com/XiaoZe980/DigitalTwinPark.git（main 分支） |
| C++ 模块名 | `DigitalTwinPark`，类名前缀统一 `DTP` |
| 日志分类 | `LogDTP`，格式 `UE_LOG(LogDTP, Log, TEXT("[DTP] ..."))` |

---

## 二、换电脑后如何跑起来

### 1. 拉取代码
```bash
git clone https://github.com/XiaoZe980/DigitalTwinPark.git
```

### 2. ⚠️ 恢复不进 git 的大资产（**关键，否则场景是空的**）

以下两个目录**被 .gitignore 排除**，需单独从网盘/U盘拷贝到 `Content/` 下：

| 目录 | 说明 |
|------|------|
| `Content/ModernCityEnvironment01/` | 城市环境资产包（约 2.3GB）—— 主场景的地形、建筑、道具 |
| `Content/UltraDynamicSky/` | Ultra Dynamic Sky/Weather（Fab 正版购买）—— 天空、天气系统 |

**这两个目录缺失会导致：主场景空白、天空/昼夜/天气功能全部失效。**

### 3. 编译
**必须先关闭 Unreal 编辑器**（Live Coding 会阻止命令行构建）：
```bash
"E:/UnrealVersion/UE_5.8/Engine/Build/BatchFiles/Build.bat" DigitalTwinParkEditor Win64 Development -Project="E:/UnrealProjects/DigitalTwinPark/DigitalTwinPark.uproject" -WaitMutex
```
编译成功标志：`Result: Succeeded`

### 4. 打开项目
双击 `DigitalTwinPark.uproject`，主场景为 `Content/Maps/ModernCityMap`。

---

## 三、已完成功能

### 核心交互
- **自由漫游**：WASD 水平移动、Q/E 升降、鼠标右键拖拽旋转、滚轮缩放（臂长插值平滑）
- **建筑选择**：左键点击建筑 → 外轮廓青色高亮 + 右下信息卡弹出；关闭卡片会同时取消选中
- **建筑聚焦**：选中后点"聚焦"按钮 → 相机保持当前方位飞近建筑（包围盒自适应距离）
- **相机预设**：4 个预设视角（总览/鸟瞰/入口/A栋）平滑飞行；带编辑器工具按钮可一键保存当前视角

### 数据链路
- **数据源切换**：Mock 模拟数据 ↔ HTTP 真实 API（HUD 按钮循环切换）
- **实时数据**：建筑人数/能耗/用水每 2 秒刷新；天气与告警 10 秒一换（避免场景与弹窗乱跳）
- **真实天气**：接入和风天气（深圳），顶栏显示实时天气 + 温度 + AQI + 时钟
- **园区总览仪表盘**：总人数 / 总能耗 / 总用水 / 活跃告警数

### 可视化与联动
- **建筑热力变色**：按实时能耗外轮廓绿→黄→红（HUD 开关控制）
- **告警弹窗**：3 行列表 + 按级别配色（信息蓝/警告黄/严重红），**点击告警条 → 相机飞向对应建筑**
- **告警联动聚焦**：新告警自动飞向对应建筑（**默认关闭**，HUD 可切换，避免打断游览）
- **建筑浮动标签**：建筑头顶常显名称 + 人数 + 能耗
- **道路车流**：20 辆车沿 Spline 循环行驶，支持蓝图车辆/静态网格/骨骼网格三类随机混合，每车速度独立随机

### 环境系统
- **UDS 天空**：Ultra Dynamic Sky 驱动，带 Onscreen Controls 时间滑块
- **昼夜切换**：HUD 按钮平滑过渡到白天/夜晚
- **夜间路灯**：`DTPNightLightManager` 按 UDS 时间自动开关路灯（光源 + 灯罩材质），自动为只有网格的路灯补建点光源
- **UDW 天气联动**：天气数据驱动真实天气视觉（晴/多云/雨/雪对应 UDW 预设）

### 部署
- **Pixel Streaming**：浏览器可访问（详见第六节）

---

## 四、架构速查

### C++ 关键类

| 类 | 职责 | 位置 |
|---|---|---|
| `ADTPPawn` | 观察者 Pawn，移动/旋转/缩放，挂载 `CameraManager` | Core/ |
| `ADTPPlayerController` | 输入分发、建筑选择、HUD 创建、各类功能转发入口 | Core/ |
| `UDTPCameraManager` | 预设飞行、建筑聚焦、**自动巡游** | Camera/ |
| `UDTPDataSubsystem` | 数据缓存与广播（GameInstanceSubsystem） | Data/ |
| `IDTPDataProvider` | 数据源接口（策略模式） | Data/ |
| `UDTPMockDataProvider` | 模拟数据（天气/告警低频，建筑/交通高频） | Data/ |
| `UDTPHttpDataProvider` | HTTP 数据源 + 和风天气 | Data/ |
| `ADTPBuildingActor` | 建筑（选中/高亮/热力/浮动标签） | Building/ |
| `ADTPBuildingManager` | 建筑注册、数据同步、热力变色 | Building/ |
| `ADTPWeatherManager` | 天气状态 + **UDW 联动** | Environment/ |
| `ADTPNightLightManager` | 夜间路灯自动控制 | Environment/ |
| `ADTPTrafficManager` | Spline 车流 | Environment/ |
| `UDTPHUDWidget` | HUD 基类，含各类格式化/汇总函数 | UI/ |
| `UDTPBuildingLabelWidget` | 建筑浮动标签基类 | UI/ |

### 数据流
```
IDTPDataProvider (Mock / HTTP)
      ↓
UDTPDataSubsystem（缓存 + 定时器 + 多播委托广播）
      ↓
各订阅者：BuildingManager（同步建筑）/ HUD（刷新文本）/ WeatherManager（驱动 UDW）
```

### 蓝图资产位置
| 资产 | 路径 |
|---|---|
| 主场景 | `Content/Maps/ModernCityMap` |
| HUD | `Content/UI/WBP_HUD` |
| 建筑标签 | `Content/UI/WBP_BuildingLabel` |
| 交互建筑 | `Content/Blueprints/BP_Building_A~E` |
| 各管理器 | `Content/Blueprints/` |
| 建筑数据/相机预设 | `Content/Data/`（`DA_Building_*` / `CP_*`） |
| 建筑合并网格 | `Content/Mesh/SM_Builiding_A~E` |

---

## 五、第三方服务配置

### 和风天气（真实天气）
代码中已有的默认值（`DTPHttpDataProvider.h`）：
- **API Host**：`https://p93wt4nmwg.re.qweatherapi.com`（账号专属，2026 年起和风停用共享地址 `devapi.qweather.com`）
- **API Key**：在 HUD 蓝图 `Set Weather Source` 节点中配置
- **经纬度**：深圳 `114.06,22.54`

> 换电脑后若天气不工作，检查：① Key 是否有效 ② Host 是否为账号专属地址 ③ 经纬度格式为「经度,纬度」

### 本地测试服务器（Mock HTTP 数据）
- 位置：`E:/DTPTestServer/`（**不在项目 git 内，需单独拷贝**）
- 编译：`build.bat`（VS 2022 环境）
- 运行：`DTPTestServer.exe` → 监听 `http://localhost:8080/api`
- 作用：提供建筑/天气/交通/告警的模拟 REST 接口，用于验证 HTTP 数据源切换

---

## 六、Pixel Streaming（浏览器访问）

### 启动信令服务器
```bash
cd "E:/UnrealVersion/UE_5.8/Engine/Plugins/Media/PixelStreaming2/Resources/WebServers/SignallingWebServer"
node dist/index.js --serve --console_messages verbose --log_config
```

### 浏览器访问
```
http://localhost/player.html
```
（前端默认已开启 HoveringMouse 悬浮鼠标模式，UI 可直接点击）

### UE 端配置（已在 `Config/DefaultEngine.ini` 配好）
```ini
[ConsoleVariables]
PixelStreaming2.Editor.UseRemoteSignallingServer=True
PixelStreaming2.ConnectionURL="ws://127.0.0.1:8888"
```

### ⚠️ 若换电脑需重建基础设施
1. 用 `get_ps_servers.bat` 下载基础设施（GitHub 下载不稳时改用 codeload URL + `curl --retry 8`）
2. `npm install --ignore-scripts`（**必须加 `--ignore-scripts`**，否则卡在 mediasoup 下载）
3. 构建：`node ../node_modules/typescript/bin/tsc -p tsconfig.cjs.json`（Common → Signalling → SignallingWebServer），前端 `npx webpack --config webpack.dev.js`
4. 修改 `SignallingWebServer/config.json` 的 `http_root` 为本机实际路径

---

## 七、踩过的坑（**重要，避免重复踩**）

### UE 通用
| 坑 | 说明 |
|---|---|
| **蓝图委托绑定叫 `Assign On`** | UE 5.8 中搜索用 `Assign On XXX`，落图后节点名为 `Bind Event to On XXX`（旧版叫 Add On，搜不到） |
| **`BlueprintImplementableEvent` 不能被蓝图调用** | override 后是红色事件，只能由 C++ 触发。蓝图内要调用需用 `BlueprintCallable` 的 C++ 函数 |
| **改蓝图资产的 Class Defaults 不影响场景实例** | 场景里已放置的实例有自己的属性副本。要么改实例本身，要么对实例 **Reset to Default** |
| **局部变量名不要用 `Pawn` / `Controller`** | 会遮蔽 `AController::Pawn` 等成员，触发 C4458 编译错误，用 `MyPawn` / `MyController` |
| **Border 不响应鼠标事件** | UMG 的 Border 没有蓝图可绑定的鼠标事件，需用 `Wrap With Button` 包一层 |
| **编辑器工具用 `CallInEditor`** | 在 Details 面板显示成按钮，只在编辑器生效（如保存相机预设） |
| **`World->FindFirstActorByClass<T>()` 语法报错** | 改用 `UGameplayStatics::GetActorOfClass()` |

### 本项目的关键坑
| 坑 | 说明 |
|---|---|
| **运行时 Pawn 拿不到关卡编辑 Pawn 的配置** | GameMode 动态生成 Pawn，与手动拖入场景的"编辑工具 Pawn"是不同实例。相机预设已改为 C++ 按路径自动加载 |
| **光源参数不能逐项复制** | ❗运行时改 `IntensityUnits` **不会自动换算数值**。`Candelas+12`（正常）与 `Unitless+12`（几乎不亮）差几十倍。**必须用 `DuplicateObject` 整体复制光源组件** |
| **大量点光源导致 VSM 溢出** | 报 `[VSM] One Pass Projection max lights overflow`，表现为相机贴近地面时灯全灭。已：关闭路灯阴影 + ini 提高 `r.Shadow.Virtual.OnePassProjection.MaxLightsPerPixel=32` |
| **PixelStreaming2 会清空基础设施目录** | 若未配置 `UseRemoteSignallingServer`，PIE 时会自动启动内嵌信令并触发 `get_ps_servers` 反复下载，可能清空目录 |
| **`GEngine is not valid` 报错** | PixelStreaming2 在编辑器模式必然打印，**无害可忽略** |

---

## 八、开发约定

### 提交规范
- **中文、结构化、说清「改了什么 + 为什么」**
- 代码与资产分开提交（便于审查）
- 大资产包不进 git

### 代码风格
- 中文注释为主，文件头 `// DigitalTwinPark - <中文名>`
- UObject 指针统一 `TObjectPtr<T>`，组件用 `VisibleAnywhere`、配置用 `EditAnywhere`
- Category 统一格式 `"DigitalTwinPark|子分类"`
- 优先 C++ 实现（能 C++ 就 C++，不写 Python 脚本）

---

## 九、待开发方向

| 优先级 | 功能 | 说明 |
|---|---|---|
| 高 | **Pixel Streaming 打包** | 打出独立 exe，脱离编辑器部署（正式交付形态） |
| 中 | **历史趋势图表** | 人数/能耗折线图随时间滚动（数字孪生大屏标配） |
| 中 | **快捷键** | `1~4` 切相机预设、`T` 切热力、`M` 切数据源、`空格` 巡游 |
| 中 | **AQI 真实值** | 和风 `v7/air/now` 已废弃（403），需查新版接口 |
| 低 | **UDS 控件风格合并** | 把 Onscreen Controls 时间滑块改成与自研 HUD 统一的暗色科技风 |
| 低 | **白天灯罩材质确认** | 检查 `DayMaterial` 配置，避免白天灯罩仍发光 |

---

## 十、资产维护提醒

⚠️ **两个大资产包不在 git 中**，换电脑/分享项目时必须单独备份：

1. `Content/ModernCityEnvironment01/`（约 2.3GB，城市环境）
2. `Content/UltraDynamicSky/`（Fab 正版购买，UDS/UDW）

外加 `E:/DTPTestServer/`（本地测试服务器，可选）。

建议：网盘 + U盘双备份，或使用 Git LFS（需额外配置）。
