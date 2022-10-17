//
// Created by PinkySmile on 15/01/2019.
//

#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include "Logger.hpp"

namespace SpiralOfFate
{
#ifndef __ANDROID__
	Logger::Logger(const std::string &filepath) noexcept :
		file(std::ofstream(filepath, std::ios::out))
	{
	}

	Logger::~Logger() noexcept
	{
		this->file.close();
	}

	void Logger::msg(const std::string &content, const std::string &prepend, bool toStdout) noexcept
	{
		this->mutex.lock();
		std::stringstream logged_str;
		time_t timestamp = std::time(nullptr);
		struct tm *tm = std::localtime(&timestamp);

		logged_str << std::put_time(tm, "[%d-%m-%Y][%H:%M:%S]") << prepend << ": " << content << std::endl;
		this->file << logged_str.str();
		if (toStdout)
			std::cout << logged_str.str();
		this->mutex.unlock();
	}

	void Logger::verbose(const std::string &content) noexcept
	{
#ifdef _DEBUG
		this->msg(content, "[VERB]", false);
#endif
	}

	void Logger::debug(const std::string &content) noexcept
	{
#ifdef _DEBUG
		this->msg(content, "[DBUG]");
#else
		this->msg(content, "[DBUG]", false);
#endif
	}

	void Logger::warn(const std::string &content) noexcept
	{
		this->msg(content, "[WARN]");
	}

	void Logger::info(const std::string &content) noexcept
	{
		this->msg(content, "[INFO]");
	}

	void Logger::error(const std::string &content) noexcept
	{
		this->msg(content, "[ERROR]");
	}

	void Logger::fatal(const std::string &content) noexcept
	{
		this->msg(content, "[FATAL]");
	}

#else
	Logger::Logger(const std::string &) noexcept
	{
	}

	Logger::~Logger() noexcept
	{
	}

	void Logger::msg(const std::string &content, int prio) noexcept
	{
		__android_log_write(prio, "SoFGV", content.c_str());
	}

	void Logger::verbose(const std::string &content) noexcept
	{
		this->msg(content, ANDROID_LOG_VERBOSE);
	}

	void Logger::debug(const std::string &content) noexcept
	{
		this->msg(content, ANDROID_LOG_DEBUG);
	}

	void Logger::warn(const std::string &content) noexcept
	{
		this->msg(content, ANDROID_LOG_WARN);
	}

	void Logger::info(const std::string &content) noexcept
	{
		this->msg(content, ANDROID_LOG_INFO);
	}

	void Logger::error(const std::string &content) noexcept
	{
		this->msg(content, ANDROID_LOG_ERROR);
	}

	void Logger::fatal(const std::string &content) noexcept
	{
		this->msg(content, ANDROID_LOG_FATAL);
	}
#endif
}
