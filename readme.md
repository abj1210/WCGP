# WCGP--Windows wenCry Gui Program 适用于Windows的WenCry图形界面程序

- 生成时间: 2026.8.10
- 生成版本: 1.0.0

## 简介
WCGP（Windows wenCry Gui Program）是一个适用于Windows操作系统的图形界面程序，旨在提供用户友好的界面来管理和操作WenCry相关功能。该程序简化了复杂的命令行操作，使用户能够更轻松地使用WenCry。

## 编译

1. 编译内核:本项目依赖于wencry项目，在下载wencry项目后依照其指示将其编译成vs2026项目。  
1. 添加依赖:参照“内核库位置配置”小节，在WCGP项目中添加内核库的依赖，确保在编译WCGP之前，wencry项目已经成功编译。
1. 编译运行:在vs2026中编译并运行WCGP项目，确保所有依赖项正确配置。

**具体运行方法参见内核项目wencry的相关文档和注释**

### 内核库位置配置

GUI 工程通过 `WinCryGUI.props` 定位内核库(wencry)的头文件与 `.lib`,支持用户自定义路径:

| 属性 | 含义 | 默认值 |
|---|---|---|
| `WENCRY_ROOT` | wencry **源码根目录**(提供 `cry.h` 等头文件) | `$(ProjectDir)..\wencry`(本工程同级目录) |
| `WENCRY_BUILD` | wencry 的 **CMake 构建输出目录**(提供 `*.lib`) | `$(WENCRY_ROOT)\build_vs` |

配置方式(任选其一):

1. **修改 `WinCryGUI.props`**(推荐):VS 中打开该文件,改 `WENCRY_ROOT`/`WENCRY_BUILD` 两行即可。
2. **环境变量**:构建前设置 `WENCRY_ROOT` 和 `WENCRY_BUILD`(MSBuild 未定义属性时自动回退环境变量,且其优先级高于默认值)。
3. **命令行**:`msbuild WinCry GUI.vcxproj /p:WENCRY_ROOT=D:\path\wencry /p:WENCRY_BUILD=D:\path\wencry\build_vs /p:Configuration=Release /p:Platform=x64`。

注意:若整个 wencry 目录搬走,需在其新位置重新生成 CMake 构建目录后再配置指向:
```
cmake -S <wencry> -B <wencry>\build_vs -G "Visual Studio 17 2022" -A x64 -DBUILD_TEST=OFF
```

`build_release.cmd` 同样读取 `WENCRY_ROOT`/`WENCRY_BUILD`(默认取脚本同级目录的 wencry),未配置时无需任何改动。

---

## 问题与解决方案记录

### 一、GUI 代码问题（`WinCry GUIDlg.cpp/.h`）

1. **消息映射重复**
   - 现象：`BEGIN_MESSAGE_MAP` 中出现两条 `ON_WM_TIMER()`。
   - 解决：删除重复条目，仅保留一条。

2. **`std::cout.rdbuf(nullptr)` 导致后续 cout 崩溃**
   - 现象：工作线程把 `std::cout` 重定向到 `std::stringstream`，结束时用 `rdbuf(nullptr)` 而非恢复原缓冲区，之后任何 `std::cout` 输出都会崩溃。
   - 解决：整体移除 cout 重定向机制（引擎在 GUI 中走 `NullResPrint`，本就不输出）。

3. **工作线程直接操作 UI 控件（跨线程 MFC 调用）**
   - 现象：线程函数里直接调用 `SetWindowTextW` / `EnableWindow` 等，存在竞态。
   - 解决：改为 `PostMessage` 自定义消息（`WM_UI_SET_TEXT`/`WM_UI_ENABLE`/`WM_UI_START`/`WM_UI_DONE`），由主线程统一处理；控件输入通过 `CryParams` 结构在主线程一次性读取后传入线程。

4. **`runner` 指针跨线程读写竞态（UAF / 双释放）**
   - 现象：工作线程写 `dlg->runner`，主线程 `OnTimer` 读取、`OnUiDone` 释放。
   - 解决：worker 仅在消息同步点 `WM_UI_START` 之前写入 `runner`，此后由主线程经 `OnTimer`/`OnUiDone` 访问，并由 `CleanupWorker()` 统一 `delete` 后置空。

5. **`m_pWorkerThread` 悬垂访问（UAF）**
   - 现象：`AfxBeginThread` 返回的 `CWinThread` 默认 `m_bAutoDelete=TRUE`，线程退出即被框架删除；`OnCancel`/`OnDestroy` 仍用 `m_pWorkerThread->m_hThread` → 读已释放内存。
   - 解决：创建后设 `pThread->m_bAutoDelete = FALSE;`，由 `CleanupWorker()` 在 `WaitForSingleObject` 确认线程结束后统一 `delete`。

### 二、内核引擎问题（`wencry/kernel`）

6. **解密时信任文件头线程数 → 堆/栈越界（运行时堆损坏的根源）**
   - 现象：`.wenc` 文件 offset 47 的线程数 `num` 无上限校验。解密时 `num>16` 会使：
     - `prepare_IV` 的 320 字节 IV 缓冲被 `getIV` 越界读/写；
     - `run_multicry` 越界写 `std::thread threads[THREAD_MAX]`（16）成员数组，最终写穿 `runcrypt` 对象；
     - `set_buffergroup(num)` 分配 `num×16MB`，`num` 很大时直接 `bad_alloc`。
   - 报错特征：`HEAP CORRUPTION DETECTED: after Normal block ... Memory allocated at WinCry GUIDlg.cpp(new runcrypt)`。
   - 解决：`FileHeader::checkType()` 将 `num` 限幅到 `[1, multicry_master::THREAD_MAX]`；`execute_decrypt` 再次钳制。

7. **`base64_to_hex` 解码越界写密钥缓冲**
   - 现象：旧 `is_valid_b64` 的判断 `(len/4)*3-2==16` 对任意 24 字符串恒成立，放行无填充 base64；`base64_to_hex` 解码出 18 字节写进 16 字节的 `key[16]` → 栈越界。
   - 解决：`is_valid_b64` 严格校验解码长度恰为 16 字节（`(len/4)*3 - tail == 16`）；GUI 密钥缓冲放大为 `key[24]` 作纵深防御。

8. **畸形文件头的模式/线程数导致空指针或越界**
   - 现象：文件头 `ctype>4`/`htype>2` 会使 `prepare_AES` 的 `createCryMaster` 返回 NULL → 空指针解引用。
   - 解决：内核 `verify()` 校验 `ctype<=4 && htype<=2`；新增 `wencry_check_header(FILE*)` 统一校验魔数、模式、线程数（0=合法，1/2/3/4 分别对应魔数/模式/线程数/文件过短错误），GUI 解密/验证前调用。

9. **ABI 尺寸不匹配 → `new runcrypt` 构造即写穿**
   - 现象：GUI 直接 `new runcrypt` 依赖自身的 `sizeof(runcrypt)`；开发期 `fheader.h` 的 `hmac` 布局发生过改动，若 GUI 编译与链接的内核库头文件版本不一致，库的构造函数按库布局写入会越过 GUI 分配的较小块（表现为 `construct-after` 堆检查即损坏）。
   - 解决：新增工厂函数 `runcrypt_create(FILE*, FILE*, u8_t*)` / `runcrypt_destroy(runcrypt*)`，对象的分配/释放移入内核库，GUI 只持有指针，彻底消除该类 ABI 尺寸问题。

### 三、构建/链接问题

10. **CMake 工程引用配置传播错误 → 链接总找 Debug 库**
    - 现象：GUI 用 `ProjectReference` 引用 CMake 生成的库工程时，MSBuild 对 CMake 工程默认取第一个配置（`Debug|x64`），导致 **Release 构建也链接 `...\aes\Debug\Aes.lib`**，报 `LNK1181 无法打开输入文件`。
    - 解决：**移除 GUI 的全部 `ProjectReference`**，改为在 `<Link>` 中显式链接 6 个库（`Wenkernel.lib;Multiaes.lib;Hash.lib;Aes.lib;CMDvals.lib;Base64.lib`），路径用 `$(Configuration)` 自动跟随配置；库由 `cmake --build` 单独构建。
    - 补充：include 路径移除旧残留 `..\wincry`（若存在会遮蔽正确的 `cry.h` 造成 ABI 错配）。

11. **库的 Debug/Release 配置未编译导致链接失败**
    - 现象：库工程只编译了某一配置（如只编了 Release），GUI 用另一配置（Debug）构建时找不到对应 `.lib`。
    - 解决：先按需编译库，再编 GUI：
      ```
      cmake --build D:\code\wencry\build_vs --config Release --target Wenkernel CMDvals   # 或 Debug
      ```

12. **一键构建**
    - 提供 `build_release.cmd`（在 VS 开发者命令提示符中运行）：
      - `build_release.cmd` → 构建 Release|x64
      - `build_release.cmd Debug` → 构建 Debug|x64（需先有 Debug 库）

### 四、已知未修复的潜在问题
- `OnInitDialog` 中 `ver.SetFont(&mf)` 使用**局部 `CFont`**，函数返回后 GDI 字体句柄失效（建议改为成员或由父窗口持有）。