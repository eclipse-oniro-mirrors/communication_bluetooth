# ProcessErrCode 错误码转换缺陷分析与修复方案

## 1. 摘要

NAPI / Taihe 错误码统一处理函数 `ProcessErrCode` 存在判断顺序缺陷：当某个错误码同时是"接口声明的有效业务错误码"和"内部错误码映射表（`innerToBusinessErrCodeMap`）的键"时，该错误码会被静默转换为内部错误码对应的目标码，绕过接口白名单声明。

- **受影响案例**：`BT_ERR_BLE_SCAN_NO_RESOURCE`（2900009）在 `BleScanner.startScan` 接口被错误转换为 2900099（`BT_ERR_INTERNAL_ERROR`）抛出，与 d.ts 声明不一致。
- **修复方案**：将 `ProcessErrCode` 判断顺序调整为"白名单优先"（napi 与 taihe 两处同步修改），不删除内部映射表中的数据条目。

## 2. 背景

### 2.1 错误码处理链路

蓝牙组件对外错误码处理由三张表驱动（以 napi 侧为例，文件 `frameworks/js/napi/src/common/napi_bluetooth_error.cpp`）：

| 表 | 语义 |
| :--- | :--- |
| `napiErrMsgMap` | 业务错误码 → 用户可见错误消息 |
| `innerErrMsgMap` | 内部错误码 → 详细错误消息 |
| `innerToBusinessErrCodeMap` | 内部错误码 → 对外业务错误码 |

统一处理入口为 `ProcessErrCode(originalCode, validErrCodes)`，被以下路径调用：

- **sync**：`HandleSyncErrWithValidCodes`、`HandleSyncErrNumWithValidCodes`（经 `HandleSyncErrAdapter` / `HandleSyncErrNumAdapter` 分发）；
- **async**：`napi_async_work.cpp` 的 `Complete()` 与 `CallFunction()`；
- **taihe**（ArkTS 1.2 静态接口）：`frameworks/ets/taihe/common/taihe_bluetooth_error.cpp` 的同构实现。

`validErrCodes` 为各接口通过 `NAPI_BT_CONTEXT` 声明的"可对外抛出错误码白名单"，与 d.ts 的 `@throws` 声明对齐。

### 2.2 缺陷机制

修改前，`ProcessErrCode` 先判断"是否内部错误码"，命中则直接转换返回，完全不检查白名单：

```cpp
ErrInfo result = { originalCode, "" };
// 内码优先：命中 innerToBusinessErrCodeMap 就直接转换并 return
if (IsInnerErrorCode(originalCode)) {          // ← 不检查 validErrCodes
    ConvertInnerToBusinessErrCode(originalCode, result);
    return result;
}
// 白名单校验只在"非内码"时执行
```

该逻辑隐含一个未被任何机制保证的假设：**内部错误码集合与业务有效错误码集合互不相交**。一旦某错误码具备双重身份，白名单声明即被绕过，错误码被静默转换。

### 2.3 实际受影响案例

`BT_ERR_BLE_SCAN_NO_RESOURCE`（2900009）符合"双重身份"，证据链如下：

| 证据 | 位置 |
| :--- | :--- |
| 公开业务错误码（Customized error codes 段） | `interfaces/inner_api/include/bluetooth_errorcode.h`：`BT_ERR_BLE_SCAN_NO_RESOURCE = BT_ERR_BASE_SYSCAP + 9 = 2900009` |
| 接口白名单已声明 | `napi_bluetooth_ble_scanner.cpp` 的 `NapiBleScanner::StartScan` 中 `validErrCodes` 包含该码 |
| d.ts 对外承诺 | `@ohos.bluetooth.ble.d.ts` 的 `BleScanner.startScan`：`@throws { BusinessError } 2900009 - Fails to start scan as it is out of hardware resources.` |
| 业务消息已登记 | `napiErrMsgMap` 中 2900009 → "Fails to start scan as it is out of hardware resources." |
| **同时被登记为内部错误码** | `innerToBusinessErrCodeMap` 中 `{ BT_ERR_BLE_SCAN_NO_RESOURCE, BT_ERR_INTERNAL_ERROR }` ← 缺陷根因 |

**运行时后果**：BLE 扫描因硬件资源不足启动失败时，底层返回 2900009，经 `ProcessErrCode` 被转换为 **2900099（`BT_ERR_INTERNAL_ERROR`）** 抛出，与接口 d.ts 声明不一致；按 2900009 做业务分支判断的开发者将无法命中。

> 交叉验证（脚本校验 + 人工核对）：`innerToBusinessErrCodeMap` 与业务消息表的键交集仅 `BT_ERR_BLE_SCAN_NO_RESOURCE` 一个，该表其余键均为 `BT_ERR_INNER_BASE(-1000)` 段的负数码。taihe 侧存在完全相同的问题（同样把 2900009 登记为内码键）。

## 3. 修复方案

### 3.1 方案对比与决策

| 方案 | 内容 | 优点 | 缺点 |
| :--- | :--- | :--- | :--- |
| 修改 1（数据清理） | 从 `innerToBusinessErrCodeMap` / `innerErrMsgMap` 删除 2900009 条目 | 消除语义脏数据 | 单独执行时未改变"判断顺序先于白名单"的逻辑缺陷；白名单不含 2900009 的路径将丢失详细消息 |
| **修改 2（逻辑调整）** | `ProcessErrCode` 改为白名单优先 | 从机制上保证白名单最高优先级，对未来同类混入天然免疫；修复目标达成 | `innerToBusinessErrCodeMap` 中数据条目保留（语义待后续清理） |
| 修改 1 + 2 | 两者都做 | 最彻底 | 改动面更大 |

**本方案决策：只做修改 2**，理由：

1. 修改 2 在运行时行为层面已完整修复唯一受影响场景（`BleScanner.startScan` 白名单已含 2900009，白名单优先后会原样抛出）；
2. 白名单优先使 `validErrCodes` 获得最终决定权，符合"白名单 = 接口对开发者的对外承诺"的语义；
3. 保留映射表数据可让"白名单未声明的 2900009"路径继续走内码转换（保留详细错误消息），兜底行为不劣化。

### 3.2 修改内容

统一将 `ProcessErrCode` 调整为三段式判断，顺序为：**白名单 → 内码转换 → 兜底**：

```cpp
ErrInfo ProcessErrCode(int32_t originalCode, const std::vector<int32_t> &validErrCodes)
{
    ErrInfo result = { originalCode, "" };
    // The error code declared by the API (validErrCodes) takes precedence and is thrown as is,
    // even if it is also registered in innerToBusinessErrCodeMap.
    bool isValidCode = false;
    for (const auto &code: validErrCodes) {
        if (code == result.errCode) {
            isValidCode = true;
            break;
        }
    }
    if (isValidCode) {
        auto detailIter = napiErrMsgMap.find(result.errCode);   // taihe 侧为 taiheErrMsgMap
        if (detailIter != napiErrMsgMap.end()) {
            result.errMsg = detailIter->second;
        }
        return result;
    }
    // inner code: originalCode -> business errCode + specific errMsg
    if (IsInnerErrorCode(originalCode)) {
        ConvertInnerToBusinessErrCode(originalCode, result);
        return result;
    }
    // invalid code: BT_ERR_INTERNAL_ERROR + specific errMsg
    result.errCode = BtErrCode::BT_ERR_INTERNAL_ERROR;
    result.errMsg = "Operation failed";
    return result;
}
```

修改涉及两个文件（必须同步，否则 ArkTS 1.2 与 JS 行为不一致）：

1. `frameworks/js/napi/src/common/napi_bluetooth_error.cpp` —— `ProcessErrCode`；
2. `frameworks/ets/taihe/common/taihe_bluetooth_error.cpp` —— `ProcessErrCode`。

实现要点：

- 原 `else` 分支的**消息填充逻辑必须保留**并移入白名单命中分支的提前 return 前：`HandleSyncErrWithValidCodes` 仅在 `errMsg` 非空时调用 `napi_throw_error`，若消息填充丢失会导致"白名单命中但静默不抛错"；
- 函数签名与全部调用方（sync 2 处、async 2 处）保持不变，仅调整内部判断顺序。

### 3.3 行为矩阵（修改前后对比）

在保留 `innerToBusinessErrCodeMap` 中 2900009 条目的前提下：

| 输入场景 | 修改前 | 修改后 | 说明 |
| :--- | :--- | :--- | :--- |
| 白名单含 2900009 的接口（`BleScanner.startScan`）收到 2900009 | 2900099 + 详细消息 ❌ | **2900009 + 业务消息** ✅ | 修复目标 |
| 白名单不含 2900009 的路径收到 2900009（如旧接口 `startBLEScan` 透传 `ret`） | 2900099 + 详细消息 | 2900099 + 详细消息（不变） | 未声明即兜底，属正常设计 |
| 纯内部错误码（`-1000` 段） | 转换 + 详细消息 | 不变 | 白名单不含内码 |
| 普通业务错误码（白名单含 / 不含） | 原样 / 兜底 2900099 | 不变 | — |
| `BT_NO_ERROR` | 调用方提前 return | 不变 | — |

结论：除"双重身份码 + 白名单命中"场景外，其余输入组合行为与修改前完全一致，无回归。

## 4. 验证方式

- **编译验证**：从 OpenHarmony 源码根目录执行 `hb build bluetooth -i`（或 CI 流水线）；
- **单元测试建议**（可锁定修复、防回归）：
  1. `ProcessErrCode(2900009, {2900009, ...})` 应返回 `{2900009, "Fails to start scan as it is out of hardware resources."}`；
  2. 内码转换不受影响：`ProcessErrCode(<任一内码，如 BT_ERR_ADDRESS_NOT_EXIST>, {})` 应返回对应业务码 + 详细消息；
  3. 兜底不变：`ProcessErrCode(<不在白名单的业务码>, {...})` 应返回 `{2900099, "Operation failed"}`；
  4. （可选）断言 `innerToBusinessErrCodeMap` 的键全部小于 0，防止未来再次混入业务错误码。

## 5. 遗留说明

1. **数据条目保留**：`innerToBusinessErrCodeMap` 中的 2900009 条目本次未清理，语义上仍属脏数据。若后续执行数据清理（修改 1），需注意"白名单不含 2900009 的路径"错误消息将从详细消息退化为通用 "Operation failed"。
2. **兜底路径仍存在**：白名单不含 2900009 的路径（如旧接口 `startBLEScan` 将底层 `ret` 透传给 `NAPI_BT_ASSERT_ERR_RETURN_VERIFY`）仍会被兜底转换为 2900099，属"未声明即兜底"的正常设计。
3. **防复制传播提示**：该缺陷模式曾被复制到 taihe 侧一次（本次已同步修复）。后续如需复制 `ProcessErrCode` 逻辑到新代码，务必保留"白名单优先"的判断顺序。
4. **附带发现**（不在本次修改范围内，建议后续单独排查）：
   - napi 侧 `BT_ERR_GATT_CONNECT_STATE_ERROR`（-1021）在 `innerErrMsgMap` 中有消息但不在 `innerToBusinessErrCodeMap` 中，为死条目；taihe 侧已将其纳入映射，两侧行为不一致；
   - `BT_ERR_NULL_HFP_STATE_MACHINE` 的错误消息 napi 侧（"Hfp state machine is not null."）与 taihe 侧（"Hfp state machine is null."）不一致，按枚举语义 napi 侧疑似笔误；
   - opp 白名单中的部分错误码（如 `BT_ERR_FILE_TYPE_NOT_SUPPORT`、`BT_ERR_TRANSFER_BUSY`、`BT_ERR_FILE_NOT_ACCESSIBLE`、`BT_ERR_TRANSFER_INFORMATION_EMPTY`）未在 `napiErrMsgMap` 登记消息，白名单命中时 `errMsg` 为空（async 路径回调消息为空、sync 路径存在静默不抛风险）。

## 6. 涉及文件清单

| 文件 | 修改内容 |
| :--- | :--- |
| `frameworks/js/napi/src/common/napi_bluetooth_error.cpp` | `ProcessErrCode` 判断顺序调整为白名单优先 |
| `frameworks/ets/taihe/common/taihe_bluetooth_error.cpp` | 同步调整 `ProcessErrCode` 判断顺序 |
| `frameworks/js/napi/src/ble/napi_bluetooth_ble_scanner.cpp` | 未修改（受影响接口，`StartScan` 白名单已含 2900009） |
| `interfaces/inner_api/include/bluetooth_errorcode.h` | 未修改（错误码定义） |
