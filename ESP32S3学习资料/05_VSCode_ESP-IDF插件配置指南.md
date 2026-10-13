# VSCode + ESP-IDF 插件配置指南

## 一、前置条件

### 1. 安装 ESP-IDF
确保已安装 ESP-IDF v5.x 版本，推荐路径：
```
C:\Espressif\frameworks\esp-idf-v5.2
```

### 2. 安装 USB 转串口驱动
ESP32-S3 通常使用 CH340/CP2102/FT232 等芯片：
- CH340: 自动安装或从 WCH 官网下载
- CP210x: 从 Silicon Labs 官网下载
- CH9102: 从 WCH 官网下载

---

## 二、安装 VSCode 插件

### 1. 安装 Espressif IDF 插件
1. 打开 VSCode
2. 按 `Ctrl+Shift+X` 打开扩展
3. 搜索 `Espressif IDF`
4. 点击安装（扩展 ID: `espressif.esp-idf-extension`）

### 2. 其他推荐插件
| 插件名 | 用途 |
|--------|------|
| C/C++ (ms-vscode.cpptools) | C/C++ 语法高亮与调试 |
| CMake Tools | CMake 项目支持 |
| Python | Python 开发支持 |
| Serial Monitor | 串口监视 |
| GitLens | Git 增强 |

---

## 三、ESP-IDF 插件配置

### 1. 配置 ESP-IDF 路径
1. 按 `Ctrl+,` 打开设置
2. 搜索 `ESP-IDF`
3. 找到以下配置项并填入对应路径：

```
ESP-IDF: Idf Path        → C:\Espressif\frameworks\esp-idf-v5.2
ESP-IDF: Python Path     → C:\Espressif\python_env\idf5.2_py3.11_env\Scripts\python.exe
ESP-IDF: Tools Path      → C:\Espressif\tools
ESP-IDF: Git Path        → C:\Program Files\Git\cmd\git.exe
ESP-IDF: CMake Path      → C:\Espressif\tools\cmake\3.24.0\bin\cmake.exe
ESP-IDF: Ninja Path      → C:\Espressif\tools\ninja\1.12.1\ninja.exe
ESP-IDF: OpenOCD Path    → C:\Espressif\tools\openocd-esp32\v0.12.0-esp32-20230721\openocd-esp32\bin\openocd.exe
ESP-IDF: Terminal Path   → C:\Windows\System32\cmd.exe
```

### 2. 配置文件方式（settings.json）
```json
{
    "idf.espIdfPath": "C:\\Espressif\\frameworks\\esp-idf-v5.2",
    "idf.pythonBinPath": "C:\\Espressif\\python_env\\idf5.2_py3.11_env\\Scripts\\python.exe",
    "idf.toolsPath": "C:\\Espressif\\tools",
    "idf.gitPath": "C:\\Program Files\\Git\\cmd\\git.exe",
    "idf.cmakePath": "C:\\Espressif\\tools\\cmake\\3.24.0\\bin\\cmake.exe",
    "idf.ninjaPath": "C:\\Espressif\\tools\\ninja\\1.12.1\\ninja.exe",
    "idf.openOcdPath": "C:\\Espressif\\tools\\openocd-esp32\\v0.12.0-esp32-20230721\\openocd-esp32\\bin\\openocd.exe",
    "idf.terminalPath": "C:\\Windows\\System32\\cmd.exe",
    "idf.port": "COM3",
    "idf.baudRate": "115200"
}
```

### 3. 验证配置
1. 按 `F1` 打开命令面板
2. 输入 `ESP-IDF: Doctor`
3. 查看诊断结果，确保所有组件状态为绿色

---

## 四、使用插件创建工程

### 1. 创建新工程
1. `F1` → `ESP-IDF: Create ESP-IDF Project`
2. 选择开发板：`esp32s3`
3. 选择模板：`hello_world`
4. 选择路径（如 `D:\Projects\my_esp32s3`）
5. 等待工程创建完成

### 2. 打开现有工程
1. `File` → `Open Folder`
2. 选择工程根目录（包含 `CMakeLists.txt` 的目录）
3. 等待 CMake 配置完成

### 3. 常用命令（F1）

| 命令 | 功能 |
|------|------|
| `ESP-IDF: Build your project` | 编译工程 |
| `ESP-IDF: Flash your project` | 烧录固件 |
| `ESP-IDF: Monitor your device` | 打开串口监视 |
| `ESP-IDF: Build, Flash and Monitor` | 编译+烧录+监视 |
| `ESP-IDF: SDK Configuration Editor` | 图形化配置 sdkconfig |
| `ESP-IDF: IDF Terminal` | 打开已配置环境的终端 |
| `ESP-IDF: Set Espressif Device Target` | 切换目标芯片 |
| `ESP-IDF: Full Clean` | 清除所有编译产物 |
| `ESP-IDF: Doctor` | 诊断环境 |

### 4. 底部状态栏
插件会在 VSCode 底部显示：
- **Target**: 当前目标芯片（esp32s3）
- **Port**: 串口号（COM3）
- **Build / Flash / Monitor**: 快捷按钮

---

## 五、调试配置

### 1. 配置 launch.json
在 `.vscode/launch.json` 中：

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "ESP32-S3 Debug",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/my_esp32s3_project.elf",
            "args": [],
            "stopAtEntry": "app_main",
            "cwd": "${workspaceFolder}",
            "environment": [],
            "MIMode": "gdb",
            "miDebuggerPath": "C:/Espressif/tools/xtensa-esp32s3-elf-gdb/14.2_20240403/xtensa-esp32s3-elf/bin/xtensa-esp32s3-elf-gdb.exe",
            "setupCommands": [
                {
                    "description": "Setup GDB",
                    "text": "-interpreter-exec console \"monitor reset halt\""
                }
            ],
            "launchCompleteCommand": "exec-and-continue",
            "customLaunchSetupCommands": [
                {
                    "text": "set $a1=0"
                }
            ]
        }
    ]
}
```

### 2. OpenOCD 配置
创建 `openocd.cfg` 或在插件中配置：

```
# openocd.cfg
interface usb_serial_jtag
set CPUTAPID 0x12003405
source [find target/esp32s3.cfg]
```

### 3. 启动调试
1. 连接 ESP32-S3 开发板
2. 点击 VSCode 底部的 `Debug` 按钮或按 `F5`
3. 在 `app_main` 入口处自动停止
4. 使用调试工具栏进行单步、查看变量等操作

### 4. 调试技巧
```
F10 / Step Over     # 单步跳过
F11 / Step Into      # 单步进入
Shift+F11 / Step Out # 单步跳出
F5 / Continue        # 继续运行
Ctrl+Shift+F5        # 重启调试
Shift+F5             # 停止调试
```

---

## 六、常用终端命令

### 1. ESP-IDF 终端
`F1` → `ESP-IDF: IDF Terminal` 打开已配置好环境的终端。

### 2. 手动激活环境（普通终端）
```cmd
REM Windows CMD
C:\Espressif\frameworks\esp-idf-v5.2\export.bat

REM Windows PowerShell
& "C:\Espressif\frameworks\esp-idf-v5.2\export.ps1"
```

### 3. 常用构建命令
```bash
# 编译
idf.py build

# 烧录
idf.py -p COM3 flash

# 监视
idf.py -p COM3 monitor

# 编译+烧录+监视
idf.py -p COM3 flash monitor

# 配置项目
idf.py menuconfig

# 清理
idf.py fullclean
```

---

## 七、项目级配置

### 1. .vscode/settings.json
```json
{
    "idf.buildPath": "${workspaceFolder}/build",
    "idf.sdkconfigPath": "${workspaceFolder}/sdkconfig",
    "idf.partitionTablePath": "${workspaceFolder}/partitions.csv",
    "idf.skipCheckSizeAndCommit": true
}
```

### 2. c_cpp_properties.json（智能提示）
```json
{
    "configurations": [
        {
            "name": "ESP32-S3",
            "includePath": [
                "${workspaceFolder}/**",
                "${env:IDF_PATH}/components/**"
            ],
            "defines": [
                "ESP32S3",
                "CONFIG_IDF_TARGET_ESP32S3"
            ],
            "compilerPath": "C:/Espressif/tools/xtensa-esp32s3-elf-gcc/14.2_20240403/xtensa-esp32s3-elf/bin/xtensa-esp32s3-elf-gcc.exe",
            "intelliSenseMode": "windows-gcc-x64"
        }
    ],
    "version": 4
}
```

### 3. clangd 支持（可选）
如果使用 clangd 替代微软 C/C++ 扩展：

```json
{
    "clangd.arguments": [
        "--compile-commands-dir=${workspaceFolder}/build"
    ]
}
```

---

## 八、常见问题解决

### 1. 插件找不到 ESP-IDF
**症状**: 插件显示红色错误
**解决**: 
- 确认 `idf.espIdfPath` 路径正确
- 运行 `ESP-IDF: Doctor` 诊断
- 重新加载窗口（`Ctrl+Shift+P` → `Developer: Reload Window`）

### 2. 编译时找不到头文件
**症状**: `xxx.h: No such file or directory`
**解决**:
- 检查 `main/CMakeLists.txt` 的 `REQUIRES` 是否包含对应组件
- 运行 `idf.py reconfigure` 重新配置
- 运行 `idf.py fullclean && idf.py build`

### 3. 烧录时无法连接
**症状**: 超时或连接失败
**解决**:
- 确认串口未被其他程序占用
- 检查驱动是否正确安装
- 尝试按住 BOOT 键再连接
- 更换 USB 数据线和端口

### 4. 编译速度慢
**解决**:
- 使用 SSD 存储工程
- 增加并行编译数：`idf.py build -j 8`
- 关闭杀毒软件实时扫描 build 目录
- 避免深层目录

### 5. managed_components hash 不一致
**症状**: `Component hash mismatch` 错误
**解决**:
- 删除 `managed_components/` 下对应组件目录
- 运行 `idf.py reconfigure`
- 或手动移动组件到 `components/` 目录

### 6. 调试时 OpenOCD 无法连接
**解决**:
- 确认开发板已连接
- 检查 JTAG/USB 接线
- 更新 OpenOCD 到最新版本
- 使用 `idf.py openocd` 启动官方 OpenOCD

### 7. menuconfig 终端乱码
**解决**:
- 使用 `ESP-IDF: IDF Terminal` 打开终端（已配置好编码）
- 或在 Windows 设置中启用 UTF-8 支持
