/**
 * @file py_account.hxx
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

template <class AccountBase = pj::Account>
class PyAccount : public AccountBase {
public:
    using AccountBase::AccountBase; // Inherit constructors

    void onIncomingCall(pj::OnIncomingCallParam& prm) override {
        PYBIND11_OVERRIDE(void, AccountBase, onIncomingCall, prm);
    }

    void onRegStarted(pj::OnRegStartedParam& prm) override {
        PYBIND11_OVERRIDE(void, AccountBase, onRegStarted, prm);
    }

    void onRegState(pj::OnRegStateParam& prm) override {
        PYBIND11_OVERRIDE(void, AccountBase, onRegState, prm);
    }
};
