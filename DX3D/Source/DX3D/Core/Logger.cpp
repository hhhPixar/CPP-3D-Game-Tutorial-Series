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
// 所属子系统：Core（核心基础设施）—— 日志系统实现
// 文件职责：实现 Logger 的构造、析构与 _log（实际输出）。
// 实现 _log：按级别过滤后用 std::clog 输出到标准错误流（带 [DX3D 级别] 前缀）。
// =============================================================================
#include <DX3D/Core/Logger.h>
#include <iostream>

// 构造函数：用初始化列表保存日志级别。m_logLevel 决定允许输出的最高详细度。
dx3d::Logger::Logger(LogLevel logLevel): m_logLevel(logLevel)
{
}

// 析构函数体为空：Logger 不持有需手动释放的资源。
dx3d::Logger::~Logger()
{
}

// _log：真正输出日志的函数。由模板 log() 在格式化后调用。
//   level：本条日志的级别；message：已格式化好的 C 字符串。
//   if (level > m_logLevel) return：级别数字越大越「详细」（Info=2>Warning=1>Error=0），
//      若本条级别比设定阈值更详细（数字更大），则直接返回不输出。
//      例如阈值设 Warning(1)，则 Info(2) 不输出，Warning(1) 与 Error(0) 输出。
void dx3d::Logger::_log(LogLevel level, const char* message)
{
	// lambda：把级别枚举转成可读字符串（日志前缀用）
	auto logLevelToString = [](LogLevel level) {
		switch (level)
		{
		case LogLevel::Info: return "Info";
		case LogLevel::Warning: return "Warning";
		case LogLevel::Error: return "Error";
		default: return "Unknown";
		}
	};

	// 级别过滤：太详细的直接跳过
	if (level > m_logLevel) return;
	// std::clog 是面向字符的「标准日志流」（默认写到 stderr）。
	// 输出形如：[DX3D Info]: 消息内容
	std::clog << "[DX3D " << logLevelToString(level) << "]: " << message << "\n";
}
