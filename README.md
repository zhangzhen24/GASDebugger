# GAS Debugger - Gameplay Ability System 调试工具

![Version](https://img.shields.io/badge/version-2.0.0-blue) ![UE](https://img.shields.io/badge/UE5-5.4+-orange) ![License](https://img.shields.io/badge/license-MIT-green)

**GAS Debugger** 是一个功能强大的 Unreal Engine 5 编辑器插件，用于实时调试和分析 Gameplay Ability System (GAS)。通过直观的可视化界面，帮助开发者深入了解 AbilitySystemComponent 的运行状态。

---

## 目录

- [功能特性](#功能特性)
- [快速开始](#快速开始)
- [界面说明](#界面说明)
- [架构设计](#架构设计)
- [技术细节](#技术细节)
- [常见问题](#常见问题)

---

## 功能特性

### 多窗口架构
- 支持同时打开多个调试窗口
- 每个窗口独立管理状态，可同时监控多个 Actor
- 窗口关闭时自动清理资源

### 四大调试面板

| 面板 | 功能描述 |
|------|----------|
| **Abilities** | 显示已授予的技能列表、状态、冷却时间、输入 ID |
| **Effects** | 显示活动的 Gameplay Effect、持续时间、层数、修改器 |
| **Tags** | 显示当前拥有的 Gameplay Tags（层级树形结构）和被阻止的标签 |
| **Attributes** | 显示所有属性集的基础值和当前值，支持搜索筛选 |

### 智能选择系统
- World 选择器：支持 Server/Client/Standalone 多世界选择
- Actor 选择器：自动列出包含 ASC 的 Actor
- Picking 模式：在编辑器视口中点击选择目标 Actor（按 END 键退出）

### 实时数据更新
- 自动订阅状态变化事件
- 手动刷新按钮
- 非侵入式只读访问，不影响游戏性能

---

## 快速开始

### 1. 打开调试窗口

通过以下方式打开 GAS Debugger 窗口：

```
菜单栏 -> Window -> GAS Debugger
```

或点击 Play 工具栏中的 GAS Debugger 按钮。

### 2. 选择调试目标

1. 点击窗口顶部的 **World** 下拉框，选择目标世界
2. 点击 **Actor** 下拉框，选择要调试的 Actor
3. 或勾选 **Picking** 复选框，在视口中点击目标 Actor

### 3. 查看数据

切换不同的标签页查看对应的 GAS 数据：

- **Abilities**：查看技能状态和冷却
- **Effects**：查看活动效果和修改器
- **Tags**：查看标签层级结构
- **Attributes**：查看属性值变化

---

## 界面说明

### Abilities 面板

以树形结构显示已授予的技能：

| 列 | 说明 |
|----|------|
| Name | 技能类名（点击可跳转到资源） |
| State | 技能状态（Ready/Active/Cooldown/Blocked） |
| IsActive | 是否正在激活 |
| Triggers | 技能触发器 |

**状态颜色编码**：
- 绿色：Ready（就绪）
- 蓝色：Active（激活中）
- 橙色：Cooldown（冷却中）
- 红色：Blocked（被阻止）

### Effects 面板

以树形结构显示活动的 Gameplay Effect：

| 列 | 说明 |
|----|------|
| Name | 效果类名 |
| Duration | 持续时间进度条（无限效果显示 ∞） |
| Stack | 叠加层数 |
| Level | 效果等级 |
| Prediction | 是否为预测效果 |
| Tags | 效果授予的标签 |

展开效果节点可查看其包含的修改器详情。

### Tags 面板

分为两个区域：
- **Owned Tags**：当前拥有的标签（层级树形结构）
- **Blocked Tags**：被阻止的标签

支持搜索筛选功能，快速定位目标标签。

### Attributes 面板

按 AttributeSet 分组显示属性：

| 列 | 说明 |
|----|------|
| Name | 属性名称 |
| Base Value | 基础值 |
| Current Value | 当前值（含修改器） |

**变化颜色编码**：
- 绿色：当前值 > 基础值
- 红色：当前值 < 基础值

---

## 架构设计

```
+-------------------------------------------+
|      FGASDebuggerModule                   |  插件入口
+-------------------------------------------+
                    |
+-------------------------------------------+
|   FGASDebuggerWindowInstance              |  多窗口管理
+-------------------------------------------+
                    |
+-------------------------------------------+
|   FGASDebuggerSharedState                 |  共享状态 & 事件广播
+-------------------------------------------+
                    |
+-------------------------------------------+
|      FGASDataProvider                     |  数据提取层
+-------------------------------------------+
                    |
+-------------------------------------------+
|      UI Widgets (Slate)                   |  用户界面
+-------------------------------------------+
```

### 核心类说明

| 类名 | 职责 |
|------|------|
| `FGASDebuggerModule` | 插件模块，管理窗口注册和生命周期 |
| `FGASDebuggerWindowInstance` | 窗口实例，封装单个调试窗口的状态 |
| `FGASDebuggerSharedState` | 共享状态管理，处理 World/Actor 选择和事件广播 |
| `FGASDataProvider` | 静态工具类，从 ASC 提取各类 GAS 数据 |
| `SGASDebuggerMainWindow` | 主窗口 Widget，包含选择器和标签页容器 |
| `SGASDebuggerTabBase` | 标签页基类，提供通用的状态订阅机制 |

### 数据流

1. 用户通过 `FGASDebuggerSharedState` 选择目标 Actor
2. 选择变化时触发 `OnSelectionChanged` 委托
3. 各标签页订阅委托，调用 `FGASDataProvider` 获取数据
4. 数据转换为树节点模型，更新 UI 显示

---

## 技术细节

### 文件结构

```
GASDebugger/
├── Source/GASDebugger/
│   ├── Public/
│   │   ├── GASDebuggerModule.h
│   │   ├── GASDebuggerCommands.h
│   │   ├── GASDebuggerStyle.h
│   │   └── GASDebuggerTypes.h
│   └── Private/
│       ├── Core/
│       │   ├── GASDebuggerSharedState.h/cpp
│       │   ├── GASDebuggerWindowInstance.h/cpp
│       │   └── GASDataProvider.h/cpp
│       ├── Widgets/
│       │   ├── SGASDebuggerMainWindow.h/cpp
│       │   ├── Tabs/
│       │   │   ├── SGASDebuggerTabBase.h/cpp
│       │   │   ├── SGASDebuggerAbilityTab.h/cpp
│       │   │   ├── SGASDebuggerEffectsTab.h/cpp
│       │   │   ├── SGASDebuggerTagsTab.h/cpp
│       │   │   └── SGASDebuggerAttributesTab.h/cpp
│       │   └── TreeNodes/
│       │       ├── GASAbilityTreeNode.h/cpp
│       │       ├── GASEffectTreeNode.h/cpp
│       │       ├── GASAttributeTreeNode.h/cpp
│       │       └── GASTagTreeNode.h/cpp
│       ├── GASDebuggerModule.cpp
│       ├── GASDebuggerCommands.cpp
│       └── GASDebuggerStyle.cpp
│   └── GASDebugger.Build.cs
├── GASDebugger.uplugin
└── Resources/
    ├── Icon128.png
    └── ButtonIcon_40x.png
```

### 数据结构

```cpp
// 技能信息
struct FGASAbilityInfo
{
    FGameplayAbilitySpecHandle Handle;
    TSubclassOf<UGameplayAbility> AbilityClass;
    int32 Level;
    bool bIsActive;
    float CooldownRemaining;
    float CooldownDuration;
    int32 InputID;
};

// 效果信息
struct FGASEffectInfo
{
    FActiveGameplayEffectHandle Handle;
    TSubclassOf<UGameplayEffect> EffectClass;
    float Duration;
    float TimeRemaining;
    int32 StackCount;
    int32 Level;
};

// 属性信息
struct FGASAttributeInfo
{
    FGameplayAttribute Attribute;
    float BaseValue;
    float CurrentValue;
    FString AttributeSetName;
};
```

### 依赖模块

**公共依赖**：
- Core

**私有依赖**：
- CoreUObject, Engine, InputCore
- Slate, SlateCore
- GameplayAbilities, GameplayTags
- UnrealEd, EditorStyle, WorkspaceMenuStructure (Editor)
- ToolMenus, Projects, PropertyEditor (Editor)

---

## 常见问题

### Q: 下拉框中没有显示目标 Actor？

**A**: 确保目标 Actor 拥有有效的 `UAbilitySystemComponent`。GAS Debugger 只会列出包含 ASC 的 Actor。

### Q: 数据没有更新？

**A**:
1. 点击 **Refresh** 按钮手动刷新
2. 确认已正确选择 World 和 Actor
3. 检查 ASC 是否有活动（技能激活、效果应用等）

### Q: 如何同时监控多个 Actor？

**A**: 点击 **New Window** 按钮打开新的调试窗口，每个窗口可以独立选择不同的目标 Actor。

### Q: Picking 模式如何使用？

**A**:
1. 勾选 **Picking** 复选框进入选择模式
2. 在编辑器视口中点击目标 Actor
3. 按 **END** 键退出选择模式

### Q: 点击技能/效果名称没反应？

**A**: 确保对应的资源文件存在于项目中。点击名称会尝试在编辑器中打开对应的蓝图资源。

---

## 许可证

本插件基于 MIT 许可证开源。
