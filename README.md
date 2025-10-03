# QCodeToolkit - 代码笔记管理工具

## 项目概述

QCodeToolkit 是一个基于 Qt 的代码笔记管理工具，目前版本为 0.5.0（开发中）。该项目旨在为开发者提供一个简单的代码片段和笔记管理环境。

---

## 依赖管理

### 必需依赖

#### Qt 框架

- **版本**: Qt 5.14.2 或更高版本
- **模块**: core, gui, widgets, svg, sql
- **说明**: 项目基于 Qt 框架开发，需要安装完整的 Qt 开发环境

#### QScintilla

- **用途**: 代码编辑器组件
- **安装方式**:
  1. 从 [Riverbank Computing](https://www.riverbankcomputing.com/software/qscintilla/download) 下载源码
  2. 参考 [QScintilla 安装教程](https://blog.csdn.net/qq_43680827/article/details/122611652) 进行编译安装
  3. 确保 Qt Creator 中已配置 QScintilla 支持

### 第三方库（包含在项目中）

#### QMarkdownTextEdit

- **用途**: Markdown 文本编辑支持
- **位置**: `3rdparty/qmarkdowntextedit/`
- **说明**: 项目已包含该库的源码，通过 `.pri` 文件集成

#### spdlog

- **用途**: 高性能日志记录
- **位置**: `3rdparty/include/`
- **说明**: 仅包含头文件版本，无需额外安装

### 构建工具

- **编译器**: MinGW (Windows)
- **构建系统**: qmake
- **C++标准**: C++17

---

## 当前状态

### ✅ 已实现功能

- **基础界面框架**
  - 无边框主窗口（FrameLessWindowBase）
  - 自定义标题栏（TitleBar）
  - 基本的菜单栏和树形面板

- **笔记系统**
  - FixedNoteTab - 固定布局笔记（当前唯一稳定可用的布局）
  - 基础的内容单元：文本、代码、图片
  - 标签管理系统（TagsWidget）

- **代码编辑**
  - 基于 QScintilla 的代码编辑器
  - 基础的语法高亮支持

- **数据管理**
  - 文件系统操作（FileManager）
  - SQLite 数据库存储（DatabaseManager）
  - 项目元数据管理（MetaCtk）

- **设置系统**
  - 外观设置页面（AppearanceSetPage）部分实现
  - 基础的主题切换功能

### ⚠️ 已知问题和限制

#### 界面问题

- **无边框窗口**：在多显示器环境下的窗口停靠功能不稳定
- **布局系统**：DynamicNoteTab 布局不稳定，暂不使用
- **响应式设计**：部分组件的自适应布局需要优化

#### 功能限制

- **笔记布局**：目前只有 FixedNoteTab 可以正常使用
- **设置系统**：仅外观设置部分可用，其他设置页面尚未实现
- **搜索功能**：基础实现，高级搜索特性缺失

#### 稳定性问题

- 某些边界条件下的崩溃问题
- 内存管理需要进一步优化
- 错误处理机制不够完善

---

## 项目结构

```
QCodeToolkit/
├── core/ # 核心管理器类
│ ├── DatabaseManager # 数据库管理
│ ├── FileManager # 文件管理
│ ├── ProjectManager # 项目管理
│ └── SettingManager # 设置管理
├── gui/ # 界面组件
│ ├── mainwindow/ # 主窗口（无边框窗口、标题栏）
│ ├── tabs/ # 选项卡（固定笔记、动态笔记等）
│ ├── noteeditor/ # 笔记编辑器（文本、代码、图片单元）
│ ├── custom/ # 自定义控件
│ ├── setting/ # 设置系统
│ ├── codeeditor/ # 代码编辑器（基于 QScintilla）
│ ├── widgets/ # 各种小部件
│ └── popover/ # 弹出层组件
├── types/ # 数据类型定义
├── util/ # 工具类
│ ├── FontManager # 字体管理
│ ├── LogManager # 日志管理
│ ├── MetaCtk # 元数据管理
│ ├── SqlDatabase # 数据库工具
│ └── StyleManager # 样式管理
└── 3rdparty/ # 第三方库（QMarkdownTextEdit 和 spdlog（头文件包含））
```

---

## 构建说明

### 环境要求

- Windows 10
- Qt 5.14.2
- MinGW 编译器
- QScintilla2 库（需单独安装）

## 使用说明

### 基本操作

1. **创建项目**：通过树形面板创建新的代码项目
2. **编辑笔记**：使用 FixedNoteTab 布局编辑笔记内容
3. **管理标签**：为笔记添加分类标签
4. **代码编辑**：在代码单元中编写和查看代码片段

### 注意事项

- 目前建议只使用 FixedNoteTab 进行笔记编辑
- 避免在多显示器环境下使用窗口停靠功能
- 定期保存工作，避免因不稳定因素导致数据丢失

## 开发计划

### 短期目标（v0.6.0）

- 完善设置系统的其他页面
- 优化无边框窗口的多显示器支持，以及跨平台需求
- 优化文件管理和文件存储方式

### 长期目标

- 实现完整的笔记导入/导出功能
- 添加代码片段的版本管理
- 支持插件扩展机制

## 贡献说明

当前项目处于早期开发阶段，欢迎反馈以下方面的问题：

- 稳定性问题
- 功能需求建议
- 用户体验改进

## 许可证

MIT License © 2025 无声目

---

*QCodeToolkit 0.5.0 - 开发中版本，功能有限但持续改进中*
