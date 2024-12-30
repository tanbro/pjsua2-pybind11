/**
 * @file py_persistent_object.hxx
 * @author 刘雪彦 (liu_xue_yan@foxmail.com)
 * @brief
 * @version 0.1
 * @date 2024-05-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <pjsua2.hpp>
#include <pybind11/pybind11.h>

template <class PersistentObjectBase = pj::PersistentObject>
class PyPersistentObject : public PersistentObjectBase {
  public:
    using PersistentObjectBase::PersistentObjectBase; // Inherit constructors

    void readObject(const pj::ContainerNode& node) override {
        PYBIND11_OVERRIDE_PURE(void, PersistentObjectBase, node);
    }

    void writeObject(pj::ContainerNode& node) const override {
        PYBIND11_OVERRIDE_PURE(void, PersistentObjectBase, node);
    }
};
