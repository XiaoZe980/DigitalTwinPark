"""
DigitalTwinPark - 一键初始化脚本
在 UE 编辑器中运行此脚本：Tools → Execute Python Script，或 Python 控制台粘贴
功能：自动创建所有蓝图类、HUD Widget、并配置建筑 DataAsset
"""
import unreal

# ============================================================================
# 配置
# ============================================================================
CONTENT_BASE = "/Game"
BUILDINGS_PATH = f"{CONTENT_BASE}/Buildings"
DATA_PATH = f"{CONTENT_BASE}/Data"
UI_PATH = f"{CONTENT_BASE}/UI"

# 5栋建筑配置
BUILDINGS = [
    {"name": "BP_Building_A", "data_asset": "DA_Building_A", "display_name": "A座·研发中心"},
    {"name": "BP_Building_B", "data_asset": "DA_Building_B", "display_name": "B座·数据中心"},
    {"name": "BP_Building_C", "data_asset": "DA_Building_C", "display_name": "C座·行政楼"},
    {"name": "BP_Building_D", "data_asset": "DA_Building_D", "display_name": "D座·会议中心"},
    {"name": "BP_Building_E", "data_asset": "DA_Building_E", "display_name": "E座·员工公寓"},
]

# ============================================================================
# 工具函数
# ============================================================================
def ensure_path(path):
    """确保 Content Browser 路径存在"""
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        print(f"  [创建目录] {path}")

def asset_exists(path):
    return unreal.EditorAssetLibrary.does_asset_exist(path)

def log(msg):
    print(f"[DTP] {msg}")

def create_blueprint(asset_path, parent_class, bp_name):
    """创建蓝图类"""
    if asset_exists(asset_path):
        log(f"  [跳过] {bp_name} 已存在")
        return unreal.load_asset(asset_path)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        bp_name,
        asset_path.rsplit("/", 1)[0],
        unreal.Blueprint,
        factory
    )
    if bp:
        unreal.EditorAssetLibrary.save_asset(asset_path)
        log(f"  [创建] {bp_name}")
    return bp

def set_building_config(bp_path, data_asset_name):
    """设置蓝图 BuildingConfig 属性"""
    try:
        data_asset_path = f"{DATA_PATH}/{data_asset_name}"
        if not asset_exists(data_asset_path):
            log(f"  [警告] {data_asset_name} 不存在，跳过配置")
            return

        data_asset = unreal.load_asset(data_asset_path)
        if not data_asset:
            return

        bp = unreal.load_asset(bp_path)
        if not bp:
            return

        # 获取生成的蓝图类
        bp_class = bp.generated_class()
        if not bp_class:
            log(f"  [警告] {bp.get_name()} 无法获取 generated_class")
            return

        cdo = unreal.get_default_object(bp_class)
        if cdo:
            cdo.set_editor_property("BuildingConfig", data_asset)
            unreal.EditorAssetLibrary.save_asset(bp_path)
            log(f"  [配置] {bp.get_name()} <- {data_asset_name}")
    except Exception as e:
        log(f"  [错误] {bp_path}: {e}")

# ============================================================================
# 主流程
# ============================================================================
def main():
    log("=" * 50)
    log("DigitalTwinPark 一键初始化开始")
    log("=" * 50)

    # 1. 确保目录存在
    log("\n[1/4] 检查目录...")
    for p in [BUILDINGS_PATH, DATA_PATH, UI_PATH]:
        ensure_path(p)

    # 2. 创建建筑蓝图
    log("\n[2/4] 创建建筑蓝图...")
    building_actor = unreal.load_class(None, "/Script/DigitalTwinPark.DTPBuildingActor")
    for b in BUILDINGS:
        bp_path = f"{BUILDINGS_PATH}/{b['name']}"
        create_blueprint(bp_path, building_actor, b['name'])
        set_building_config(bp_path, b['data_asset'])

    # 3. 创建管理器蓝图
    log("\n[3/4] 创建管理器蓝图...")
    building_manager = unreal.load_class(None, "/Script/DigitalTwinPark.DTPBuildingManager")
    create_blueprint(f"{BUILDINGS_PATH}/BP_BuildingManager", building_manager, "BP_BuildingManager")

    weather_manager = unreal.load_class(None, "/Script/DigitalTwinPark.DTPWeatherManager")
    create_blueprint(f"{BUILDINGS_PATH}/BP_WeatherManager", weather_manager, "BP_WeatherManager")

    daynight = unreal.load_class(None, "/Script/DigitalTwinPark.DTPDayNightCycle")
    create_blueprint(f"{BUILDINGS_PATH}/BP_DayNightCycle", daynight, "BP_DayNightCycle")

    # 4. 创建 UI 蓝图
    log("\n[4/4] 创建 UI 蓝图...")
    hud_widget = unreal.load_class(None, "/Script/UMG.UserWidget")
    # 注意：HUDWidget 的 C++ 类是 UDTPHUDWidget，但 CreateWidget 需要 Blueprint 派生
    # 这里创建基于 UDTPHUDWidget 的 Widget 蓝图
    dtp_hud = unreal.load_class(None, "/Script/DigitalTwinPark.DTPHUDWidget")
    create_blueprint(f"{UI_PATH}/WBP_HUD", dtp_hud, "WBP_HUD")

    log("\n" + "=" * 50)
    log("初始化完成！")
    log("=" * 50)
    log("\n下一步:")
    log("  1. 打开地图，把 BP_Building_A~E 拖入场景")
    log("  2. 把 BP_BuildingManager 拖入场景")
    log("  3. 把 BP_WeatherManager 拖入场景")
    log("  4. 编辑 WBP_HUD 设计 UI 界面")

if __name__ == "__main__":
    main()