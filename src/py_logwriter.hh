/**
 * @file py_logwriter.hxx
 * @author 刘雪彦 (liu_xue_yan@foxmail.com)
 * @brief
 * @version 0.1
 * @date 2024-05-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <pybind11/pybind11.h>

#include <pjsua2.hpp>

template <class LogWriterBase = pj::LogWriter>
class PyLogWriter : public LogWriterBase {
  public:
    using LogWriterBase::LogWriterBase; // Inherit constructors

    void write(const pj::LogEntry& entry) override {
        PYBIND11_OVERRIDE_PURE(void, LogWriterBase, write, entry);
    }
};
