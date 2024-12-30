/**
 * @file py_call.hxx
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

template <class CallBase = pj::Call> class PyCall : public CallBase {
  public:
    using CallBase::CallBase; // Inherit constructors

    void onCallState(pj::OnCallStateParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onCallState, prm);
    }

    void onCallTsxState(pj::OnCallTsxStateParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onCallTsxState, prm);
    }

    void onCallMediaState(pj::OnCallMediaStateParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onCallMediaState, prm);
    }

    void onCallSdpCreated(pj::OnCallSdpCreatedParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onCallSdpCreated, prm);
    }

    void onStreamPreCreate(pj::OnStreamPreCreateParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onStreamPreCreate, prm);
    }

    void onStreamCreated(pj::OnStreamCreatedParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onStreamCreated, prm);
    }

    void onStreamDestroyed(pj::OnStreamDestroyedParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onStreamDestroyed, prm);
    }

    void onDtmfDigit(pj::OnDtmfDigitParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onDtmfDigit, prm);
    }

    void onDtmfEvent(pj::OnDtmfEventParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onDtmfEvent, prm);
    }

    void onCallTransferRequest(pj::OnCallTransferRequestParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onCallTransferRequest, prm);
    }

    void onCallTransferStatus(pj::OnCallTransferStatusParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onCallTransferStatus, prm);
    }

    void onCallReplaceRequest(pj::OnCallReplaceRequestParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onCallReplaceRequest, prm);
    }

    void onCallReplaced(pj::OnCallReplacedParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onCallReplaced, prm);
    }

    void onCallRxOffer(pj::OnCallRxOfferParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onCallRxOffer, prm);
    }

    void onCallTxOffer(pj::OnCallTxOfferParam& prm) override {
        PYBIND11_OVERRIDE(void, CallBase, onCallTxOffer, prm);
    }
};
