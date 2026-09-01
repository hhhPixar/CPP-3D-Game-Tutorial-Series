/*MIT License

C++ 3D Game Tutorial Series (https://github.com/PardCode/CPP-3D-Game-Tutorial-Series)

Copyright (c) 2019-2026, PardCode

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

// =============================================================================
// 所属子系统：Core（核心基础设施）—— 日志系统
// 文件职责：定义 Logger 类与一组日志宏（DX3DLog*/DX3DLogThrow*）。
//
// 关键概念讲解（初学者重点理解）：
//   1) std::format（C++20）：类似 Python 的 str.format / fmt 库，用占位符 {} 的格式串
//      生成字符串，比 C 的 printf 类型安全（编译期检查），比 cout 流式更简洁。
//      例如 std::format("x={}, y={}", 1, 2.5) 得到 "x=1, y=2.5"。
//   2) 日志级别（LogLevel）：Error > Warning > Info，级别越高越重要。
//      Logger 只打印 level <= m_logLevel 的日志（见 _log 里 if (level > m_logLevel) return）。
//      即若设为 Warning，则 Error 与 Warning 都打，Info 不打。默认 Error 只打错误。
//   3) 日志宏 DX3DLog*：把「调用 logger.log + 填级别」简化成一行，避免每次手写参数。
//      用 __VA_OPT__ 处理「有/无格式参数」两种情况（C++20 新语法）。
//   4) DX3DLogThrow*：先记日志、再抛异常，用于「出错时既要记录又要中断」的场景。
//
// 架构位置：被 Base 持有，几乎全引擎可用。DX3DLog* 宏在 Base 派生类中调用
//   （它们内部调用 getLogger()，所以调用点必须能访问 getLogger()，即在 Base 子类里）。
// 协作对象：Base（持有 Logger）、各业务类（通过宏打日志）。
// =============================================================================
#pragma once
#include <DX3D/Core/Core.h>
#include <format>

namespace dx3d
{
	// Logger：日志输出器。final 表示不可再被继承。
	// 用 dx3d_disable_copy_and_move 禁用拷贝/移动，保证全引擎单一日志器实例。
	class Logger final
	{
		dx3d_disable_copy_and_move(Logger)
	public:
		// 日志级别枚举。值越大越「详细」（Info=2 最详细，Error=0 最重要）。
		// _log 里的比较：level > m_logLevel 才跳过，所以设 Error 时只显示 Error。
		enum class LogLevel
		{
			Error = 0,
			Warning,
			Info
		};

		// 构造函数：explicit 防隐式转换，需显式传入日志级别，默认 Error。
		explicit Logger(LogLevel logLevel = LogLevel::Error);
		// 析构函数：目前无资源需手动释放。
		~Logger();

		// 模板成员函数 log：用 std::format 格式化后交给 _log 输出。
		// typename... Args 是可变参数模板（任意个数/类型的格式参数）。
		// std::format_string<Args...> 是「编译期类型安全的格式串」，
		//   构造时若占位符与参数数量/类型不匹配会在编译期报错——比 printf 安全。
		// Args&&... args 是万能引用（转发引用），配合 std::forward 完美转发，
		//   保留左值/右值属性，避免多余的拷贝。
		// 调用示例：logger.log(LogLevel::Info, "x={}, y={}", x, y);
		template<typename... Args>
		void log(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
		{
			auto str = std::format(fmt, std::forward<Args>(args)...);
			_log(level,
				str.c_str()
			);
		}
		// 私有非模板函数：真正输出日志（实现在 Logger.cpp）。
		// 模板 log 负责格式化，_log 负责按级别过滤并打印，职责分离。
	private:
		void _log(LogLevel level, const char* message);
		// 当前日志器允许打印的最高级别。高于此级别（更详细）的日志被丢弃。
	private:
		LogLevel m_logLevel = LogLevel::Error;
	};
}

// ============================================================================
// 日志宏区（使用预处理器宏来简化调用，需在 Base 派生类的成员函数内使用）
// ============================================================================

// DX3DLog：基础日志宏。
//   logger.log((type), {message} __VA_OPT__(,) __VA_ARGS__);
// 解析要点：
//   - {message} 构造一个 std::format_string（编译期检查的格式串）
//   - __VA_OPT__(,) 是 C++20 新语法：当「...」可变参非空时展开成逗号「,」，
//     为空时展开成什么都没有。这样就兼容了「只有消息文本」和「消息+参数」两种用法：
//       有参数：DX3DLog(logger, Info, "x={}", 1) -> logger.log(Info, {"x={}"}, 1)
//       无参数：DX3DLog(logger, Info, "启动完成") -> logger.log(Info, {"启动完成"})
#define DX3DLog(logger, type, message,...)\
	logger.log((type), {message} __VA_OPT__(,) __VA_ARGS__);

// DX3DLogThrow：记录日志后抛出异常。
//   用 { } 包裹成语句块，先调用 DX3DLog 打日志，再 throw 异常。
//   exception 是异常类型（如 std::runtime_error）。
//   用途：D3D11/Win32 调用失败时，既要把错误记录下来，又要中断执行路径。
#define DX3DLogThrow(logger, exception, type, message, ...)\
{\
DX3DLog(logger,type,message, __VA_ARGS__);\
throw exception(message);\
}

// 下面四个宏是便捷封装：自动填入 getLogger() 与级别，调用者只需写消息与参数。
// 前提：调用处所在对象必须继承 Base（这样才有 getLogger()）。

// DX3DLogInfo：打印 Info 级别日志（最详细，调试用）
#define DX3DLogInfo(message,...)\
	DX3DLog(getLogger(), Logger::LogLevel::Info, message, __VA_ARGS__)

// DX3DLogWarning：打印 Warning 级别日志
#define DX3DLogWarning(message,...)\
	DX3DLog(getLogger(), Logger::LogLevel::Warning, message, __VA_ARGS__)

// DX3DLogError：打印 Error 级别日志（默认就会输出）
#define DX3DLogError(message,...)\
	DX3DLog(getLogger(), Logger::LogLevel::Error, message, __VA_ARGS__)

// DX3DLogThrowError：记录 Error 后抛出 std::runtime_error（通用运行期错误）
#define DX3DLogThrowError(message,...)\
	DX3DLogThrow(getLogger(), std::runtime_error, Logger::LogLevel::Error, message, __VA_ARGS__)

// DX3DLogThrowInvalidArg：记录 Error 后抛出 std::invalid_argument（参数不合法时用）
#define DX3DLogThrowInvalidArg(message,...)\
	DX3DLogThrow(getLogger(), std::invalid_argument, Logger::LogLevel::Error, message, __VA_ARGS__)