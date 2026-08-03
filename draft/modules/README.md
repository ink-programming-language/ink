# Ink Package 与 Module 设计

本目录记录 Ink package、subpackage、module、源码布局与导入解析规则。只有已经确认的规则才创建独立议题文件。

## 议题状态

| 编号 | 议题 | 状态 | 文件 |
| --- | --- | --- | --- |
| 01 | 文件系统 package 与单文件 module | 已确认 | [01-filesystem-packages-and-file-modules.md](01-filesystem-packages-and-file-modules.md) |
| 02 | 相对 module 导入 | 已确认 | [02-relative-module-imports.md](02-relative-module-imports.md) |
| 03 | 实际 module 依赖图无环 | 已确认 | [03-acyclic-active-module-dependencies.md](03-acyclic-active-module-dependencies.md) |

本目录讨论语言级 package/module 身份和布局。包管理器下载协议、清单文件格式、版本求解与仓库协议在后续包管理议题中另行确定。
