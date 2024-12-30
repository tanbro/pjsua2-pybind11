/**
 * @file py_pjsua2.cxx
 * @author 刘雪彦 (liu_xue_yan@foxmail.com)
 * @brief
 * @version 0.1
 * @date 2024-05-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <pjsua2.hpp>

#include "py_account.hh"
#include "py_call.hh"
#include "py_logwriter.hh"
#include "py_persistent_object.hh"

namespace py = pybind11;

////////////////////////////////////////////////////////////////////
// 容器不透明化
////////////////////////////////////////////////////////////////////
PYBIND11_MAKE_OPAQUE(pj::IntVector);
PYBIND11_MAKE_OPAQUE(pj::StringVector);
PYBIND11_MAKE_OPAQUE(pj::SipHeaderVector);
PYBIND11_MAKE_OPAQUE(pj::AuthCredInfoVector);
PYBIND11_MAKE_OPAQUE(pj::CallMediaInfoVector);
PYBIND11_MAKE_OPAQUE(pj::AudioMediaVector2);
PYBIND11_MAKE_OPAQUE(pj::AudioDevInfoVector2);

PYBIND11_MODULE(pjsua2, m) {
    ////////////////////////////////////////////////////////////////////
    // pj::Error 异常处理
    ////////////////////////////////////////////////////////////////////

    py::register_exception_translator([](std::exception_ptr p) {
        if (p) {
            try {
                std::rethrow_exception(p);
            } catch (const pj::Error& e) {
                /// pybind11::set_error was introduced in 2.12.0
#if PYBIND11_VERSION_HEX >= 0x020c0000
                py::set_error(PyExc_RuntimeError, e.info().c_str());
#else
                PyErr_SetString(PyExc_RuntimeError, e.info().c_str());
#endif
            }
        }
    });

    ////////////////////////////////////////////////////////////////////
    // 枚举类型
    ////////////////////////////////////////////////////////////////////

    py::enum_<pjsua_state>(m, "State")
        .value("NULL", pjsua_state::PJSUA_STATE_NULL)
        .value("CREATED", pjsua_state::PJSUA_STATE_CREATED)
        .value("INIT", pjsua_state::PJSUA_STATE_INIT)
        .value("STARTING", pjsua_state::PJSUA_STATE_STARTING)
        .value("RUNNING", pjsua_state::PJSUA_STATE_RUNNING)
        .value("CLOSING", pjsua_state::PJSUA_STATE_CLOSING);

    py::enum_<pjsua_call_media_status>(m, "CallMediaStatus")
        .value("NONE", pjsua_call_media_status::PJSUA_CALL_MEDIA_NONE)
        .value("ACTIVE", pjsua_call_media_status::PJSUA_CALL_MEDIA_ACTIVE)
        .value(
            "LOCAL_HOLD", pjsua_call_media_status::PJSUA_CALL_MEDIA_LOCAL_HOLD
        )
        .value(
            "REMOTE_HOLD", pjsua_call_media_status::PJSUA_CALL_MEDIA_REMOTE_HOLD
        )
        .value("ERROR", pjsua_call_media_status::PJSUA_CALL_MEDIA_ERROR);

    py::enum_<pjsua_dtmf_method>(m, "DtmfMethod")
        .value("RFC2833", pjsua_dtmf_method::PJSUA_DTMF_METHOD_RFC2833)
        .value("SIP_INFO", pjsua_dtmf_method::PJSUA_DTMF_METHOD_SIP_INFO);

    py::enum_<pjsip_transport_type_e>(m, "TransportType", py::arithmetic())
        .value(
            "UNSPECIFIED", pjsip_transport_type_e::PJSIP_TRANSPORT_UNSPECIFIED
        )
        .value("UDP", pjsip_transport_type_e::PJSIP_TRANSPORT_UDP)
        .value("TCP", pjsip_transport_type_e::PJSIP_TRANSPORT_TCP)
        .value("TLS", pjsip_transport_type_e::PJSIP_TRANSPORT_TLS)
        .value("DTLS", pjsip_transport_type_e::PJSIP_TRANSPORT_DTLS)
        .value("SCTP", pjsip_transport_type_e::PJSIP_TRANSPORT_SCTP)
        .value("LOOP", pjsip_transport_type_e::PJSIP_TRANSPORT_LOOP)
        .value("DGRAM", pjsip_transport_type_e::PJSIP_TRANSPORT_LOOP_DGRAM)
        .value("IPV6", pjsip_transport_type_e::PJSIP_TRANSPORT_IPV6)
        .value("UDP6", pjsip_transport_type_e::PJSIP_TRANSPORT_UDP6)
        .value("TCP6", pjsip_transport_type_e::PJSIP_TRANSPORT_TCP6)
        .value("TLS6", pjsip_transport_type_e::PJSIP_TRANSPORT_TLS6)
        .value("DTLS6", pjsip_transport_type_e::PJSIP_TRANSPORT_DTLS6);

    py::enum_<pjsip_status_code>(m, "StatusCode")
        .value("NULL", pjsip_status_code::PJSIP_SC_NULL)
        /** 1xx **/
        .value("TRYING", pjsip_status_code::PJSIP_SC_TRYING)
        .value("RINGING", pjsip_status_code::PJSIP_SC_RINGING)
        .value(
            "CALL_BEING_FORWARDED",
            pjsip_status_code::PJSIP_SC_CALL_BEING_FORWARDED
        )
        .value("QUEUED", pjsip_status_code::PJSIP_SC_QUEUED)
        .value("PROGRESS", pjsip_status_code::PJSIP_SC_PROGRESS)
        .value(
            "EARLY_DIALOG_TERMINATED",
            pjsip_status_code::PJSIP_SC_EARLY_DIALOG_TERMINATED
        )
        /** 2xx **/
        .value("OK", pjsip_status_code::PJSIP_SC_OK)
        .value("ACCEPTED", pjsip_status_code::PJSIP_SC_ACCEPTED)
        .value("NO_NOTIFICATION", pjsip_status_code::PJSIP_SC_NO_NOTIFICATION)
        /** 3xx **/
        .value("MULTIPLE_CHOICES", pjsip_status_code::PJSIP_SC_MULTIPLE_CHOICES)
        .value(
            "MOVED_PERMANENTLY", pjsip_status_code::PJSIP_SC_MOVED_PERMANENTLY
        )
        .value(
            "MOVED_TEMPORARILY", pjsip_status_code::PJSIP_SC_MOVED_TEMPORARILY
        )
        .value("USE_PROXY", pjsip_status_code::PJSIP_SC_USE_PROXY)
        .value(
            "ALTERNATIVE_SERVICE",
            pjsip_status_code::PJSIP_SC_ALTERNATIVE_SERVICE
        )
        /** 4xx **/
        .value("BAD_REQUEST", pjsip_status_code::PJSIP_SC_BAD_REQUEST)
        .value("UNAUTHORIZED", pjsip_status_code::PJSIP_SC_UNAUTHORIZED)
        .value("PAYMENT_REQUIRED", pjsip_status_code::PJSIP_SC_PAYMENT_REQUIRED)
        .value("FORBIDDEN", pjsip_status_code::PJSIP_SC_FORBIDDEN)
        .value("NOT_FOUND", pjsip_status_code::PJSIP_SC_NOT_FOUND)
        .value(
            "METHOD_NOT_ALLOWED", pjsip_status_code::PJSIP_SC_METHOD_NOT_ALLOWED
        )
        .value("NOT_ACCEPTABLE", pjsip_status_code::PJSIP_SC_NOT_ACCEPTABLE)
        .value(
            "PROXY_AUTHENTICATION_REQUIRED",
            pjsip_status_code::PJSIP_SC_PROXY_AUTHENTICATION_REQUIRED
        )
        .value("REQUEST_TIMEOUT", pjsip_status_code::PJSIP_SC_REQUEST_TIMEOUT)
        .value("CONFLICT", pjsip_status_code::PJSIP_SC_CONFLICT)
        .value("GONE", pjsip_status_code::PJSIP_SC_GONE)
        .value("LENGTH_REQUIRED", pjsip_status_code::PJSIP_SC_LENGTH_REQUIRED)
        .value(
            "CONDITIONAL_REQUEST_FAILED",
            pjsip_status_code::PJSIP_SC_CONDITIONAL_REQUEST_FAILED
        )
        .value(
            "REQUEST_ENTITY_TOO_LARGE",
            pjsip_status_code::PJSIP_SC_REQUEST_ENTITY_TOO_LARGE
        )
        .value(
            "REQUEST_URI_TOO_LONG",
            pjsip_status_code::PJSIP_SC_REQUEST_URI_TOO_LONG
        )
        .value(
            "UNSUPPORTED_MEDIA_TYPE",
            pjsip_status_code::PJSIP_SC_UNSUPPORTED_MEDIA_TYPE
        )
        .value(
            "UNSUPPORTED_URI_SCHEME",
            pjsip_status_code::PJSIP_SC_UNSUPPORTED_URI_SCHEME
        )
        .value(
            "UNKNOWN_RESOURCE_PRIORITY",
            pjsip_status_code::PJSIP_SC_UNKNOWN_RESOURCE_PRIORITY
        )
        .value("BAD_EXTENSION", pjsip_status_code::PJSIP_SC_BAD_EXTENSION)
        .value(
            "EXTENSION_REQUIRED", pjsip_status_code::PJSIP_SC_EXTENSION_REQUIRED
        )
        .value(
            "SESSION_TIMER_TOO_SMALL",
            pjsip_status_code::PJSIP_SC_SESSION_TIMER_TOO_SMALL
        )
        .value(
            "INTERVAL_TOO_BRIEF", pjsip_status_code::PJSIP_SC_INTERVAL_TOO_BRIEF
        )
        .value(
            "BAD_LOCATION_INFORMATION",
            pjsip_status_code::PJSIP_SC_BAD_LOCATION_INFORMATION
        )
        .value(
            "USE_IDENTITY_HEADER",
            pjsip_status_code::PJSIP_SC_USE_IDENTITY_HEADER
        )
        .value(
            "PROVIDE_REFERRER_HEADER",
            pjsip_status_code::PJSIP_SC_PROVIDE_REFERRER_HEADER
        )
        .value("FLOW_FAILED", pjsip_status_code::PJSIP_SC_FLOW_FAILED)
        .value(
            "ANONIMITY_DISALLOWED",
            pjsip_status_code::PJSIP_SC_ANONIMITY_DISALLOWED
        )
        .value(
            "BAD_IDENTITY_INFO", pjsip_status_code::PJSIP_SC_BAD_IDENTITY_INFO
        )
        .value(
            "UNSUPPORTED_CERTIFICATE",
            pjsip_status_code::PJSIP_SC_UNSUPPORTED_CERTIFICATE
        )
        .value(
            "INVALID_IDENTITY_HEADER",
            pjsip_status_code::PJSIP_SC_INVALID_IDENTITY_HEADER
        )
        .value(
            "FIRST_HOP_LACKS_OUTBOUND_SUPPORT",
            pjsip_status_code::PJSIP_SC_FIRST_HOP_LACKS_OUTBOUND_SUPPORT
        )
        .value(
            "MAX_BREADTH_EXCEEDED",
            pjsip_status_code::PJSIP_SC_MAX_BREADTH_EXCEEDED
        )
        .value("BAD_INFO_PACKAGE", pjsip_status_code::PJSIP_SC_BAD_INFO_PACKAGE)
        .value("CONSENT_NEEDED", pjsip_status_code::PJSIP_SC_CONSENT_NEEDED)
        .value(
            "TEMPORARILY_UNAVAILABLE",
            pjsip_status_code::PJSIP_SC_TEMPORARILY_UNAVAILABLE
        )
        .value(
            "CALL_TSX_DOES_NOT_EXIST",
            pjsip_status_code::PJSIP_SC_CALL_TSX_DOES_NOT_EXIST
        )
        .value("LOOP_DETECTED", pjsip_status_code::PJSIP_SC_LOOP_DETECTED)
        .value("TOO_MANY_HOPS", pjsip_status_code::PJSIP_SC_TOO_MANY_HOPS)
        .value(
            "ADDRESS_INCOMPLETE", pjsip_status_code::PJSIP_SC_ADDRESS_INCOMPLETE
        )
        .value("AMBIGUOUS", pjsip_status_code::PJSIP_AC_AMBIGUOUS)
        .value("BUSY_HERE", pjsip_status_code::PJSIP_SC_BUSY_HERE)
        .value(
            "REQUEST_TERMINATED", pjsip_status_code::PJSIP_SC_REQUEST_TERMINATED
        )
        .value(
            "NOT_ACCEPTABLE_HERE",
            pjsip_status_code::PJSIP_SC_NOT_ACCEPTABLE_HERE
        )
        .value("BAD_EVENT", pjsip_status_code::PJSIP_SC_BAD_EVENT)
        .value("REQUEST_UPDATED", pjsip_status_code::PJSIP_SC_REQUEST_UPDATED)
        .value("REQUEST_PENDING", pjsip_status_code::PJSIP_SC_REQUEST_PENDING)
        .value("UNDECIPHERABLE", pjsip_status_code::PJSIP_SC_UNDECIPHERABLE)
        .value(
            "SECURITY_AGREEMENT_NEEDED",
            pjsip_status_code::PJSIP_SC_SECURITY_AGREEMENT_NEEDED
        )
        /** 5xx **/
        .value(
            "INTERNAL_SERVER_ERROR",
            pjsip_status_code::PJSIP_SC_INTERNAL_SERVER_ERROR
        )
        .value("NOT_IMPLEMENTED", pjsip_status_code::PJSIP_SC_NOT_IMPLEMENTED)
        .value("BAD_GATEWAY", pjsip_status_code::PJSIP_SC_BAD_GATEWAY)
        .value(
            "SERVICE_UNAVAILABLE",
            pjsip_status_code::PJSIP_SC_SERVICE_UNAVAILABLE
        )
        .value("SERVER_TIMEOUT", pjsip_status_code::PJSIP_SC_SERVER_TIMEOUT)
        .value(
            "VERSION_NOT_SUPPORTED",
            pjsip_status_code::PJSIP_SC_VERSION_NOT_SUPPORTED
        )
        .value(
            "MESSAGE_TOO_LARGE", pjsip_status_code::PJSIP_SC_MESSAGE_TOO_LARGE
        )
        .value(
            "PUSH_NOTIFICATION_SERVICE_NOT_SUPPORTED",
            pjsip_status_code::PJSIP_SC_PUSH_NOTIFICATION_SERVICE_NOT_SUPPORTED
        )
        .value(
            "PRECONDITION_FAILURE",
            pjsip_status_code::PJSIP_SC_PRECONDITION_FAILURE
        )
        /** 6xx **/
        .value("BUSY_EVERYWHERE", pjsip_status_code::PJSIP_SC_BUSY_EVERYWHERE)
        .value("DECLINE", pjsip_status_code::PJSIP_SC_DECLINE)
        .value(
            "DOES_NOT_EXIST_ANYWHERE",
            pjsip_status_code::PJSIP_SC_DOES_NOT_EXIST_ANYWHERE
        )
        .value(
            "NOT_ACCEPTABLE_ANYWHERE",
            pjsip_status_code::PJSIP_SC_NOT_ACCEPTABLE_ANYWHERE
        )
        .value("UNWANTED", pjsip_status_code::PJSIP_SC_UNWANTED)
        .value("REJECTED", pjsip_status_code::PJSIP_SC_REJECTED)
        /** other **/
        .value("TSX_TIMEOUT", pjsip_status_code::PJSIP_SC_TSX_TIMEOUT)
        .value(
            "TSX_TRANSPORT_ERROR",
            pjsip_status_code::PJSIP_SC_TSX_TRANSPORT_ERROR
        );

    py::enum_<pjsip_role_e>(m, "Role")
        .value("UAC", pjsip_role_e::PJSIP_ROLE_UAC)
        .value("UAS", pjsip_role_e::PJSIP_ROLE_UAS);

    py::enum_<pjsip_inv_state>(m, "InvState")
        .value("NULL", pjsip_inv_state::PJSIP_INV_STATE_NULL)
        .value("CALLING", pjsip_inv_state::PJSIP_INV_STATE_CALLING)
        .value("INCOMING", pjsip_inv_state::PJSIP_INV_STATE_INCOMING)
        .value("EARLY", pjsip_inv_state::PJSIP_INV_STATE_EARLY)
        .value("CONNECTING", pjsip_inv_state::PJSIP_INV_STATE_CONNECTING)
        .value("CONFIRMED", pjsip_inv_state::PJSIP_INV_STATE_CONFIRMED)
        .value("DISCONNECTED", pjsip_inv_state::PJSIP_INV_STATE_DISCONNECTED);

    py::enum_<pjsip_redirect_op>(m, "RedirectOp")
        .value("REJECT", pjsip_redirect_op::PJSIP_REDIRECT_REJECT)
        .value("ACCEPT", pjsip_redirect_op::PJSIP_REDIRECT_ACCEPT)
        .value(
            "ACCEPT_REPLACE", pjsip_redirect_op::PJSIP_REDIRECT_ACCEPT_REPLACE
        )
        .value("PENDING", pjsip_redirect_op::PJSIP_REDIRECT_PENDING)
        .value("STOP", pjsip_redirect_op::PJSIP_REDIRECT_STOP);

    py::enum_<pjmedia_jb_discard_algo>(m, "MediaJbDiscardAlgo")
        .value("NONE", pjmedia_jb_discard_algo::PJMEDIA_JB_DISCARD_NONE)
        .value("STATIC", pjmedia_jb_discard_algo::PJMEDIA_JB_DISCARD_STATIC)
        .value(
            "PROGRESSIVE",
            pjmedia_jb_discard_algo::PJMEDIA_JB_DISCARD_PROGRESSIVE
        );

    py::enum_<pjmedia_type>(m, "MediaType")
        .value("NONE", pjmedia_type::PJMEDIA_TYPE_NONE)
        .value("AUDIO", pjmedia_type::PJMEDIA_TYPE_AUDIO)
        .value("VIDEO", pjmedia_type::PJMEDIA_TYPE_VIDEO)
        .value("APPLICATION", pjmedia_type::PJMEDIA_TYPE_APPLICATION)
        .value("UNKNOWN", pjmedia_type::PJMEDIA_TYPE_UNKNOWN);

    py::enum_<pjmedia_dir>(m, "MediaDir")
        .value("NONE", pjmedia_dir::PJMEDIA_DIR_NONE)
        .value("ENCODING", pjmedia_dir::PJMEDIA_DIR_ENCODING)
        .value("CAPTURE", pjmedia_dir::PJMEDIA_DIR_CAPTURE)
        .value("DECODING", pjmedia_dir::PJMEDIA_DIR_DECODING)
        .value("PLAYBACK", pjmedia_dir::PJMEDIA_DIR_PLAYBACK)
        .value("RENDER", pjmedia_dir::PJMEDIA_DIR_RENDER)
        .value("ENCODING_DECODING", pjmedia_dir::PJMEDIA_DIR_ENCODING_DECODING)
        .value("CAPTURE_PLAYBACK", pjmedia_dir::PJMEDIA_DIR_CAPTURE_PLAYBACK)
        .value("CAPTURE_RENDER", pjmedia_dir::PJMEDIA_DIR_CAPTURE_RENDER);

    py::enum_<pjsua_ipv6_use>(m, "Ipv6Use")
        .value("DISABLED", pjsua_ipv6_use::PJSUA_IPV6_DISABLED)
        .value("ENABLED", pjsua_ipv6_use::PJSUA_IPV6_ENABLED)
        .value(
            "ENABLED_NO_PREFERENCE",
            pjsua_ipv6_use::PJSUA_IPV6_ENABLED_NO_PREFERENCE
        )
        .value(
            "ENABLED_PREFER_IPV4",
            pjsua_ipv6_use::PJSUA_IPV6_ENABLED_PREFER_IPV4
        )
        .value(
            "ENABLED_PREFER_IPV6",
            pjsua_ipv6_use::PJSUA_IPV6_ENABLED_PREFER_IPV6
        )
        .value(
            "ENABLED_USE_IPV6_ONLY",
            pjsua_ipv6_use::PJSUA_IPV6_ENABLED_USE_IPV6_ONLY
        );

    py::enum_<pjsip_event_id_e>(m, "EventId")
        .value("UNKNOWN", pjsip_event_id_e::PJSIP_EVENT_UNKNOWN)
        .value("TIMER", pjsip_event_id_e::PJSIP_EVENT_TIMER)
        .value("TX_MSG", pjsip_event_id_e::PJSIP_EVENT_TX_MSG)
        .value("RX_MSG", pjsip_event_id_e::PJSIP_EVENT_RX_MSG)
        .value("TRANSPORT_ERROR", pjsip_event_id_e::PJSIP_EVENT_TRANSPORT_ERROR)
        .value("TSX_STATE", pjsip_event_id_e::PJSIP_EVENT_TSX_STATE)
        .value("USER", pjsip_event_id_e::PJSIP_EVENT_USER);

    py::enum_<pjsip_tsx_state_e>(m, "TsxState")
        .value("NULL", pjsip_tsx_state_e::PJSIP_TSX_STATE_NULL)
        .value("CALLING", pjsip_tsx_state_e::PJSIP_TSX_STATE_CALLING)
        .value("TRYING", pjsip_tsx_state_e::PJSIP_TSX_STATE_TRYING)
        .value("PROCEEDING", pjsip_tsx_state_e::PJSIP_TSX_STATE_PROCEEDING)
        .value("COMPLETED", pjsip_tsx_state_e::PJSIP_TSX_STATE_COMPLETED)
        .value("CONFIRMED", pjsip_tsx_state_e::PJSIP_TSX_STATE_CONFIRMED)
        .value("TERMINATED", pjsip_tsx_state_e::PJSIP_TSX_STATE_TERMINATED)
        .value("DESTROYED", pjsip_tsx_state_e::PJSIP_TSX_STATE_DESTROYED)
        .value("MAX", pjsip_tsx_state_e::PJSIP_TSX_STATE_MAX);

    py::enum_<pjmedia_aud_dev_cap>(m, "AudDevCap", py::arithmetic())
        .value(
            "EXT_FORMAT", pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_EXT_FORMAT
        )
        .value(
            "INPUT_LATENCY",
            pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY
        )
        .value(
            "OUTPUT_LATENCY",
            pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY
        )
        .value(
            "INPUT_VOLUME_SETTING",
            pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_INPUT_VOLUME_SETTING
        )
        .value(
            "OUTPUT_VOLUME_SETTING",
            pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING
        )
        .value(
            "INPUT_SIGNAL_METER",
            pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_INPUT_SIGNAL_METER
        )
        .value(
            "OUTPUT_SIGNAL_METER",
            pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_OUTPUT_SIGNAL_METER
        )
        .value(
            "INPUT_ROUTE", pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_INPUT_ROUTE
        )
        .value(
            "INPUT_SOURCE",
            pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_INPUT_SOURCE
        )
        .value(
            "OUTPUT_ROUTE",
            pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_OUTPUT_ROUTE
        )
        .value("EC", pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_EC)
        .value("EC_TAIL", pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_EC_TAIL)
        .value("VAD", pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_VAD)
        .value("CNG", pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_CNG)
        .value("PLC", pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_PLC)
        .value("MAX", pjmedia_aud_dev_cap::PJMEDIA_AUD_DEV_CAP_MAX);

    py::enum_<pjmedia_aud_dev_route>(m, "MediaAudioDevRoute", py::arithmetic())
        .value("DEFAULT", pjmedia_aud_dev_route::PJMEDIA_AUD_DEV_ROUTE_DEFAULT)
        .value(
            "LOUDSPEAKER",
            pjmedia_aud_dev_route::PJMEDIA_AUD_DEV_ROUTE_LOUDSPEAKER
        )
        .value(
            "EARPIECE", pjmedia_aud_dev_route::PJMEDIA_AUD_DEV_ROUTE_EARPIECE
        )
        .value(
            "BLUETOOTH", pjmedia_aud_dev_route::PJMEDIA_AUD_DEV_ROUTE_BLUETOOTH
        )
        .value("CUSTOM", pjmedia_aud_dev_route::PJMEDIA_AUD_DEV_ROUTE_CUSTOM);

    /////////////

    py::bind_vector<std::vector<int>>(m, "IntVector");
    py::bind_vector<std::vector<std::string>>(m, "StringVector");

    ////////////////////////////////////////////////////////////////////
    // 杂项类型
    ////////////////////////////////////////////////////////////////////

    py::class_<pj::SipHeader>(m, "SipHeader")
        .def(py::init())
        .def_readwrite("hName", &pj::SipHeader::hName)
        .def_readwrite("hValue", &pj::SipHeader::hValue);

    py::bind_vector<std::vector<pj::SipHeader>>(m, "SipHeaderVector");

    py::class_<pj::LogEntry>(m, "LogEntry")
        .def(py::init())
        .def_readwrite("level", &pj::LogEntry::level)
        .def_readwrite("msg", &pj::LogEntry::msg)
        .def_readwrite("threadId", &pj::LogEntry::threadId)
        .def_readwrite("threadName", &pj::LogEntry::threadName);

    py::class_<pj::CallSetting>(m, "CallSetting")
        .def(py::init())
        .def_readwrite("flag", &pj::CallSetting::flag)
        .def_readwrite("reqKeyframeMethod", &pj::CallSetting::reqKeyframeMethod)
        .def_readwrite("audioCount", &pj::CallSetting::audioCount)
        .def_readwrite("videoCount", &pj::CallSetting::videoCount)
        .def_readwrite("mediaDir", &pj::CallSetting::mediaDir);

    py::class_<pj::TimeVal>(m, "TimeVal")
        .def(py::init())
        .def_readwrite("sec", &pj::TimeVal::sec)
        .def_readwrite("msec", &pj::TimeVal::msec);

    py::class_<pj::CallSendDtmfParam>(m, "CallSendDtmfParam")
        .def(py::init())
        .def_readwrite("method", &pj::CallSendDtmfParam::method)
        .def_readwrite("duration", &pj::CallSendDtmfParam::duration)
        .def_readwrite("digits", &pj::CallSendDtmfParam::digits);

    py::class_<pj::SdpSession>(m, "SdpSession")
        .def(py::init())
        .def_readwrite("wholeSdp", &pj::SdpSession::wholeSdp);

    py::class_<pj::SipTxOption>(m, "SipTxOption")
        .def(py::init())
        .def_readwrite("targetUri", &pj::SipTxOption::targetUri)
        .def_readwrite("localUri", &pj::SipTxOption::localUri)
        .def_readonly("headers", &pj::SipTxOption::headers)
        .def_readwrite("contentType", &pj::SipTxOption::contentType)
        .def_readwrite("msgBody", &pj::SipTxOption::msgBody)
        // /// TODO: wrap SipMediaType
        // .def_readwrite("multipartContentType",
        // &pj::SipTxOption::multipartContentType)
        // /// TODO: SipMultipartPartVector
        // .def_readwrite("multipartParts", &pj::SipTxOption::multipartParts)
        .def_property_readonly("empty", &pj::SipTxOption::isEmpty);

    py::class_<pj::MediaFormatAudio>(m, "MediaFormatAudio")
        .def(py::init())
        .def_readwrite("clockRate", &pj::MediaFormatAudio::clockRate)
        .def_readwrite("channelCount", &pj::MediaFormatAudio::channelCount)
        .def_readwrite("frameTimeUsec", &pj::MediaFormatAudio::frameTimeUsec)
        .def_readwrite("bitsPerSample", &pj::MediaFormatAudio::bitsPerSample)
        .def_readwrite("avgBps", &pj::MediaFormatAudio::avgBps)
        .def_readwrite("maxBps", &pj::MediaFormatAudio::maxBps);

    py::class_<pj::SipTxData>(m, "SipTxData")
        .def(py::init())
        // string              info;
        .def_readwrite("info", &pj::SipTxData::info)
        // string              wholeMsg;
        .def_readwrite("wholeMsg", &pj::SipTxData::wholeMsg)
        /// string// SocketAddress       dstAddress;
        .def_readwrite("dstAddress", &pj::SipTxData::dstAddress);

    py::class_<pj::SipRxData>(m, "SipRxData")
        .def(py::init())
        // string              info;
        .def_readwrite("info", &pj::SipRxData::info)
        // string              wholeMsg;
        .def_readwrite("wholeMsg", &pj::SipRxData::wholeMsg)
        /// string// SocketAddress       srcAddress;
        .def_readwrite("srcAddress", &pj::SipRxData::srcAddress);

    py::class_<pj::TsxStateEventSrc>(m, "TsxStateEventSrc")
        .def(py::init())
        // SipRxData       rdata;          /**< The incoming
        /// message.      */
        .def_readonly("rdata", &pj::TsxStateEventSrc::rdata)
        // SipTxData       tdata;          /**< The outgoing
        /// message.      */
        .def_readonly("tdata", &pj::TsxStateEventSrc::tdata)
        /// NOTE: void* TimerEntry      timer;          /**< The timer. */
        // .def_readonly("timer", &pj::TsxStateEventSrc::timer)
        // pj_status_t     status;         /**< Transport error status.    */
        .def_readonly("status", &pj::TsxStateEventSrc::status)
        /// NOTE: void* GenericData     data;           /**< Generic data. */
        // .def_readonly("data", &pj::TsxStateEventSrc::data)
        ;

    py::class_<pj::SipTransaction>(m, "SipTransaction")
        .def(py::init())
        // pjsip_role_e        role;           /**< Role (UAS or UAC)      */
        .def_readwrite("role", &pj::SipTransaction::role)
        // string              method;         /**< The method.            */
        .def_readwrite("method", &pj::SipTransaction::method)
        // int                 statusCode;     /**< Last status code seen. */
        .def_readwrite("statusCode", &pj::SipTransaction::statusCode)
        // string              statusText;     /**< Last reason phrase.    */
        .def_readwrite("statusText", &pj::SipTransaction::statusText)
        // pjsip_tsx_state_e   state;          /**< State.                 */
        .def_readwrite("state", &pj::SipTransaction::state)
        // SipTxData           lastTx;         /**< Msg kept for retrans.  */
        .def_readonly("lastTx", &pj::SipTransaction::lastTx);

    py::class_<pj::TsxStateEvent>(m, "TsxStateEvent")
        .def(py::init())
        .def_readonly("src", &pj::TsxStateEvent::src)
        .def_readonly("tsx", &pj::TsxStateEvent::tsx)
        .def_readonly("prevState", &pj::TsxStateEvent::prevState)
        .def_readonly("type", &pj::TsxStateEvent::type);

    py::class_<pj::TimerEvent>(m, "TimerEvent")
        /// NOTE: void*
        // .def_readonly("entry", &pj::TimerEvent::entry)
        ;

    py::class_<pj::TxMsgEvent>(m, "TxMsgEvent")
        .def_readonly("tdata", &pj::TxMsgEvent::tdata);

    py::class_<pj::TxErrorEvent>(m, "TxErrorEvent")
        .def_readonly("tdata", &pj::TxErrorEvent::tdata)
        .def_readonly("tsx", &pj::TxErrorEvent::tsx);

    py::class_<pj::RxMsgEvent>(m, "RxMsgEvent")
        .def_readonly("rdata", &pj::RxMsgEvent::rdata);

    py::class_<pj::UserEvent>(m, "UserEvent");

    py::class_<pj::SipEventBody>(m, "SipEventBody")
        .def_readonly("timer", &pj::SipEventBody::timer)
        .def_readonly("tsxState", &pj::SipEventBody::tsxState)
        .def_readonly("txMsg", &pj::SipEventBody::txMsg)
        .def_readonly("txError", &pj::SipEventBody::txError)
        .def_readonly("rxMsg", &pj::SipEventBody::rxMsg)
        .def_readonly("user", &pj::SipEventBody::user);

    py::class_<pj::SipEvent>(m, "SipEvent")
        .def(py::init())
        .def_readwrite("type", &pj::SipEvent::type)
        .def_readwrite("body", &pj::SipEvent::body);

    ////////////////////////////////////////////////////////////////////
    // Info 类型
    ////////////////////////////////////////////////////////////////////

    py::class_<pj::AuthCredInfo>(m, "AuthCredInfo")
        .def(py::init())
        .def(
            py::init<
                const std::string&, const std::string&, const std::string&,
                const int, const std::string>(),
            py::arg("scheme"), py::arg("realm"), py::arg("user_name"),
            py::arg("data_type"), py::arg("data")
        )
        .def_readwrite("scheme", &pj::AuthCredInfo::scheme)
        .def_readwrite("realm", &pj::AuthCredInfo::realm)
        .def_readwrite("username", &pj::AuthCredInfo::username)
        .def_readwrite("dataType", &pj::AuthCredInfo::dataType)
        .def_readwrite("data", &pj::AuthCredInfo::data)
        .def_readwrite("akaK", &pj::AuthCredInfo::akaK)
        .def_readwrite("akaOp", &pj::AuthCredInfo::akaOp)
        .def_readwrite("akaAmf", &pj::AuthCredInfo::akaAmf);

    py::bind_vector<std::vector<pj::AuthCredInfo>>(m, "AuthCredInfoVector");

    py::class_<pj::CallMediaInfo>(m, "CallMediaInfo")
        .def(py::init())
        .def_readwrite("index", &pj::CallMediaInfo::index)
        .def_readwrite("type", &pj::CallMediaInfo::type)
        .def_readwrite("dir", &pj::CallMediaInfo::dir)
        .def_readwrite("status", &pj::CallMediaInfo::status)
        .def_readwrite("audioConfSlot", &pj::CallMediaInfo::audioConfSlot);

    py::bind_vector<std::vector<pj::CallMediaInfo>>(m, "CallMediaInfoVector");

    py::class_<pj::ConfPortInfo>(m, "ConfPortInfo")
        .def(py::init())
        .def_readwrite("portId", &pj::ConfPortInfo::portId)
        .def_readwrite("name", &pj::ConfPortInfo::name)
        .def_readonly("format", &pj::ConfPortInfo::format)
        .def_readwrite("txLevelAdj", &pj::ConfPortInfo::txLevelAdj)
        .def_readwrite("rxLevelAdj", &pj::ConfPortInfo::rxLevelAdj)
        .def_readonly("listeners", &pj::ConfPortInfo::listeners);

    py::class_<pj::Media>(m, "Media")
        .def_property_readonly("type", &pj::Media::getType);

    py::class_<pj::AudioMedia, pj::Media>(m, "AudioMedia")
        .def(py::init())
        .def_property_readonly("portInfo", &pj::AudioMedia::getPortInfo)
        .def_property_readonly("portId", &pj::AudioMedia::getPortId)
        .def_static(
            "getPortInfoFromId", &pj::AudioMedia::getPortInfoFromId,
            py::arg("port_id")
        )
        .def("startTransmit", &pj::AudioMedia::startTransmit, py::arg("sink"))
        .def(
            "startTransmit2", &pj::AudioMedia::startTransmit2, py::arg("sink"),
            py::arg("param")
        )
        .def("stopTransmit", &pj::AudioMedia::stopTransmit, py::arg("sink"))
        .def("adjustRxLevel", &pj::AudioMedia::adjustRxLevel, py::arg("level"))
        .def("adjustTxLevel", &pj::AudioMedia::adjustTxLevel, py::arg("level"))
        .def_property_readonly("rxLevel", &pj::AudioMedia::getRxLevel)
        .def_property_readonly("txLevel", &pj::AudioMedia::getTxLevel)
        .def_static(
            "typecastFromMedia", &pj::AudioMedia::typecastFromMedia,
            py::arg("media")
        );

    py::bind_vector<std::vector<pj::AudioMedia>>(m, "AudioMediaVector2");

    py::class_<pj::StreamInfo>(m, "StreamInfo")
        .def(py::init())
        .def_readwrite("type", &pj::StreamInfo::type)
        /// TODO: more ...
        ;

    py::class_<pj::AccountInfo>(m, "AccountInfo")
        .def(py::init())
        .def_readwrite("id", &pj::AccountInfo::id)
        .def_readwrite("isDefault", &pj::AccountInfo::isDefault)
        .def_readwrite("uri", &pj::AccountInfo::uri)
        .def_readwrite("regIsConfigured", &pj::AccountInfo::regIsConfigured)
        .def_readwrite("regIsActive", &pj::AccountInfo::regIsActive)
        .def_readwrite("regExpiresSec", &pj::AccountInfo::regExpiresSec)
        .def_readwrite("regStatus", &pj::AccountInfo::regStatus)
        .def_readwrite("regStatusText", &pj::AccountInfo::regStatusText)
        .def_readwrite("regLastErr", &pj::AccountInfo::regLastErr)
        .def_readwrite("onlineStatus", &pj::AccountInfo::onlineStatus)
        .def_readwrite("onlineStatusText", &pj::AccountInfo::onlineStatusText);

    py::class_<pj::CallInfo>(m, "CallInfo")
        .def(py::init())
        .def_readwrite("id", &pj::CallInfo::id)
        .def_readwrite("role", &pj::CallInfo::role)
        .def_readwrite("accId", &pj::CallInfo::accId)
        .def_readwrite("localUri", &pj::CallInfo::localUri)
        .def_readwrite("localContact", &pj::CallInfo::localContact)
        .def_readwrite("remoteUri", &pj::CallInfo::remoteUri)
        .def_readwrite("remoteContact", &pj::CallInfo::remoteContact)
        .def_readwrite("callIdString", &pj::CallInfo::callIdString)
        .def_readonly("setting", &pj::CallInfo::setting)
        .def_readwrite("state", &pj::CallInfo::state)
        .def_readwrite("stateText", &pj::CallInfo::stateText)
        .def_readwrite("lastStatusCode", &pj::CallInfo::lastStatusCode)
        .def_readwrite("lastReason", &pj::CallInfo::lastReason)
        .def_readonly("media", &pj::CallInfo::media)
        .def_readonly("provMedia", &pj::CallInfo::provMedia)
        .def_readonly("connectDuration", &pj::CallInfo::connectDuration)
        .def_readonly("totalDuration", &pj::CallInfo::totalDuration)
        .def_readwrite("remOfferer", &pj::CallInfo::remOfferer)
        .def_readwrite("remAudioCount", &pj::CallInfo::remAudioCount)
        .def_readwrite("remVideoCount", &pj::CallInfo::remVideoCount);

    py::class_<pj::AudioDevInfo>(m, "AudioDevInfo")
        .def_readwrite("id", &pj::AudioDevInfo::id)
        .def_readwrite("name", &pj::AudioDevInfo::name)
        .def_readwrite("id", &pj::AudioDevInfo::id)
        .def_readwrite("inputCount", &pj::AudioDevInfo::inputCount)
        .def_readwrite("outputCount", &pj::AudioDevInfo::outputCount)
        .def_readwrite(
            "defaultSamplesPerSec", &pj::AudioDevInfo::defaultSamplesPerSec
        )
        .def_readwrite("driver", &pj::AudioDevInfo::driver)
        .def_readwrite("caps", &pj::AudioDevInfo::caps)
        .def_readwrite("routes", &pj::AudioDevInfo::routes)
        /// more ...
        ;

    py::class_<pj::TransportInfo>(m, "TransportInfo")
        // TransportId             id;
        .def_readwrite("id", &pj::TransportInfo::id)
        // pjsip_transport_type_e  type;
        .def_readwrite("type", &pj::TransportInfo::type)
        // string                  typeName;
        .def_readwrite("typeName", &pj::TransportInfo::typeName)
        // string                  info;
        .def_readwrite("info", &pj::TransportInfo::info)
        // unsigned                flags;
        .def_readwrite("flags", &pj::TransportInfo::flags)
        // SocketAddress           localAddress;
        .def_readwrite("localAddress", &pj::TransportInfo::localAddress)
        // SocketAddress           localName;
        .def_readwrite("localName", &pj::TransportInfo::localName)
        // unsigned                usageCount;
        .def_readwrite("usageCount", &pj::TransportInfo::usageCount);

    py::bind_vector<std::vector<pj::AudioDevInfo>>(m, "AudioDevInfoVector2");

    ////////////////////////////////////////////////////////////////////
    // 参数类型
    ////////////////////////////////////////////////////////////////////

    py::class_<pj::CallOpParam>(m, "CallOpParam")
        .def(py::init<bool>(), py::arg("useDefaultCallSetting") = false)
        .def_readonly("opt", &pj::CallOpParam::opt)
        .def_readwrite("statusCode", &pj::CallOpParam::statusCode)
        .def_readwrite("reason", &pj::CallOpParam::reason)
        .def_readwrite("options", &pj::CallOpParam::options)
        .def_readonly("txOption", &pj::CallOpParam::txOption)
        .def_readonly("sdp", &pj::CallOpParam::sdp);

    py::class_<pj::OnRegStartedParam>(m, "OnRegStartedParam")
        .def(py::init())
        .def_readwrite("renew", &pj::OnRegStartedParam::renew);

    py::class_<pj::OnRegStateParam>(m, "OnRegStateParam")
        .def(py::init())
        .def_readwrite("status", &pj::OnRegStateParam::status)
        .def_readwrite("code", &pj::OnRegStateParam::code)
        .def_readwrite("reason", &pj::OnRegStateParam::reason)
        .def_readonly("rdata", &pj::OnRegStateParam::rdata)
        .def_readwrite("expiration", &pj::OnRegStateParam::expiration);

    py::class_<pj::OnCallStateParam>(m, "OnCallStateParam")
        .def(py::init())
        .def_readonly("e", &pj::OnCallStateParam::e);

    py::class_<pj::OnIncomingCallParam>(m, "OnIncomingCallParam")
        .def(py::init())
        .def_readwrite("callId", &pj::OnIncomingCallParam::callId)
        .def_readonly("rdata", &pj::OnIncomingCallParam::rdata);

    py::class_<pj::OnCallTsxStateParam>(m, "OnCallTsxStateParam")
        .def(py::init())
        .def_readonly("e", &pj::OnCallTsxStateParam::e);

    py::class_<pj::OnCallMediaStateParam>(m, "OnCallMediaStateParam")
        .def(py::init());

    py::class_<pj::OnCallSdpCreatedParam>(m, "OnCallSdpCreatedParam")
        .def(py::init())
        .def_readonly("sdp", &pj::OnCallSdpCreatedParam::sdp)
        .def_readonly("remSdp", &pj::OnCallSdpCreatedParam::remSdp);

    py::class_<pj::OnStreamPreCreateParam>(m, "OnStreamPreCreateParam")
        .def(py::init())
        .def_readwrite("streamIdx", &pj::OnStreamPreCreateParam::streamIdx)
        .def_readonly("streamInfo", &pj::OnStreamPreCreateParam::streamInfo);

    py::class_<pj::OnStreamCreatedParam>(m, "OnStreamCreatedParam")
        .def(py::init())
        /// NOTE: MediaStream `void *` !
        // .def_readwrite("stream", &pj::OnStreamCreatedParam::stream)
        .def_readwrite("streamIdx", &pj::OnStreamCreatedParam::streamIdx)
        .def_readwrite("destroyPort", &pj::OnStreamCreatedParam::destroyPort)
        /// NOTE: MediaPort `void *` !
        // .def_readwrite("pPort", &pj::OnStreamCreatedParam::pPort)
        ;

    py::class_<pj::OnStreamDestroyedParam>(m, "OnStreamDestroyedParam")
        .def(py::init())
        /// NOTE: MediaStream void *
        // .def_readwrite("stream", &pj::OnStreamDestroyedParam::stream)
        .def_readwrite("streamIdx", &pj::OnStreamDestroyedParam::streamIdx);

    py::class_<pj::OnDtmfDigitParam>(m, "OnDtmfDigitParam")
        .def(py::init())
        .def_readwrite("method", &pj::OnDtmfDigitParam::method)
        .def_readwrite("digit", &pj::OnDtmfDigitParam::digit)
        .def_readwrite("duration", &pj::OnDtmfDigitParam::duration);

    py::class_<pj::OnDtmfEventParam>(m, "OnDtmfEventParam")
        .def(py::init())
        .def_readwrite("method", &pj::OnDtmfEventParam::method)
        .def_readwrite("timestamp", &pj::OnDtmfEventParam::timestamp)
        .def_readwrite("digit", &pj::OnDtmfEventParam::digit)
        .def_readwrite("duration", &pj::OnDtmfEventParam::duration)
        .def_readwrite("flags", &pj::OnDtmfEventParam::flags);

    py::class_<pj::OnCallTransferRequestParam>(m, "onCallTransferRequestParam")
        .def(py::init())
        .def_readwrite("dstUri", &pj::OnCallTransferRequestParam::dstUri)
        .def_readwrite(
            "statusCode", &pj::OnCallTransferRequestParam::statusCode
        )
        .def_readwrite("opt", &pj::OnCallTransferRequestParam::opt)
        .def_readwrite("newCall", &pj::OnCallTransferRequestParam::newCall);

    py::class_<pj::OnCallTransferStatusParam>(m, "OnCallTransferStatusParam")
        .def(py::init())
        .def_readwrite("statusCode", &pj::OnCallTransferStatusParam::statusCode)
        .def_readwrite("reason", &pj::OnCallTransferStatusParam::reason)
        .def_readwrite(
            "finalNotify", &pj::OnCallTransferStatusParam::finalNotify
        )
        .def_readwrite("cont", &pj::OnCallTransferStatusParam::cont);

    py::class_<pj::OnCallReplaceRequestParam>(m, "OnCallReplaceRequestParam")
        .def(py::init())
        .def_readwrite("rdata", &pj::OnCallReplaceRequestParam::rdata)
        .def_readwrite("statusCode", &pj::OnCallReplaceRequestParam::statusCode)
        .def_readwrite("reason", &pj::OnCallReplaceRequestParam::reason)
        .def_readwrite("opt", &pj::OnCallReplaceRequestParam::opt)
        .def_readwrite("newCall", &pj::OnCallReplaceRequestParam::newCall);

    py::class_<pj::OnCallReplacedParam>(m, "OnCallReplacedParam")
        .def(py::init())
        .def_readwrite("newCallId", &pj::OnCallReplacedParam::newCallId)
        .def_readwrite("newCall", &pj::OnCallReplacedParam::newCall);

    py::class_<pj::OnCallRxOfferParam>(m, "OnCallRxOfferParam")
        .def(py::init())
        .def_readwrite("offer", &pj::OnCallRxOfferParam::offer)
        .def_readwrite("statusCode", &pj::OnCallRxOfferParam::statusCode)
        .def_readwrite("opt", &pj::OnCallRxOfferParam::opt);

    py::class_<pj::OnCallTxOfferParam>(m, "onCallTxOfferParam")
        .def(py::init())
        .def_readwrite("opt", &pj::OnCallTxOfferParam::opt);

    py::class_<pj::AudioMediaTransmitParam>(m, "AudioMediaTransmitParam")
        .def(py::init())
        .def_readwrite("level", &pj::AudioMediaTransmitParam::level);

    ////////////////////////////////////////////////////////////////////
    // Config 类型
    ////////////////////////////////////////////////////////////////////

    py::class_<pj::PersistentObject, PyPersistentObject<>>(
        m, "PersistentObject"
    )
        .def(py::init());

    /**
     * @brief TransportConfig
     *
     */
    py::class_<pj::TransportConfig, pj::PersistentObject>(m, "TransportConfig")
        .def(py::init())
        .def_readwrite("port", &pj::TransportConfig::port)
        .def_readwrite("portRange", &pj::TransportConfig::portRange)
        .def_readwrite("randomizePort", &pj::TransportConfig::randomizePort)
        .def_readwrite("publicAddress", &pj::TransportConfig::publicAddress)
        .def_readwrite("boundAddress", &pj::TransportConfig::boundAddress);

    py::class_<pj::LogWriter, PyLogWriter<>>(m, "LogWriter")
        .def(py::init())
        .def("write", &pj::LogWriter::write, py::arg("entry"));

    /**
     * @brief UaConfig
     *
     */
    py::class_<pj::UaConfig, pj::PersistentObject>(m, "UaConfig")
        .def(py::init())
        .def_readwrite("maxCalls", &pj::UaConfig::maxCalls)
        .def_readwrite("threadCnt", &pj::UaConfig::threadCnt)
        .def_readwrite("mainThreadOnly", &pj::UaConfig::mainThreadOnly)
        .def_readwrite("nameserver", &pj::UaConfig::nameserver);

    py::class_<pj::LogConfig, pj::PersistentObject>(m, "LogConfig")
        .def(py::init())
        .def_readwrite("msgLogging", &pj::LogConfig::msgLogging)
        .def_readwrite("level", &pj::LogConfig::level)
        .def_readwrite("consoleLevel", &pj::LogConfig::consoleLevel)
        .def_readwrite("decor", &pj::LogConfig::decor)
        .def_readwrite("filename", &pj::LogConfig::filename)
        .def_readwrite("fileFlags", &pj::LogConfig::fileFlags)
        .def_readwrite("writer", &pj::LogConfig::writer);

    py::class_<pj::MediaConfig, pj::PersistentObject>(m, "MediaConfig")
        .def(py::init())
        .def_readwrite("clockRate", &pj::MediaConfig::clockRate)
        .def_readwrite("sndClockRate", &pj::MediaConfig::sndClockRate)
        .def_readwrite("channelCount", &pj::MediaConfig::channelCount)
        .def_readwrite("audioFramePtime", &pj::MediaConfig::audioFramePtime)
        .def_readwrite("maxMediaPorts", &pj::MediaConfig::maxMediaPorts)
        .def_readwrite("hasIoqueue", &pj::MediaConfig::hasIoqueue)
        .def_readwrite("threadCnt", &pj::MediaConfig::threadCnt)
        .def_readwrite("quality", &pj::MediaConfig::quality)
        .def_readwrite("ptime", &pj::MediaConfig::ptime)
        .def_readwrite("noVad", &pj::MediaConfig::noVad)
        .def_readwrite("ilbcMode", &pj::MediaConfig::ilbcMode)
        .def_readwrite("txDropPct", &pj::MediaConfig::txDropPct)
        .def_readwrite("rxDropPct", &pj::MediaConfig::rxDropPct)
        .def_readwrite("ecOptions", &pj::MediaConfig::ecOptions)
        .def_readwrite("ecTailLen", &pj::MediaConfig::ecTailLen)
        .def_readwrite("sndRecLatency", &pj::MediaConfig::sndRecLatency)
        .def_readwrite("sndPlayLatency", &pj::MediaConfig::sndPlayLatency)
        .def_readwrite("jbInit", &pj::MediaConfig::jbInit)
        .def_readwrite("jbMinPre", &pj::MediaConfig::jbMinPre)
        .def_readwrite("jbMaxPre", &pj::MediaConfig::jbMaxPre)
        .def_readwrite("jbMax", &pj::MediaConfig::jbMax)
        .def_readwrite("jbDiscardAlgo", &pj::MediaConfig::jbDiscardAlgo)
        .def_readwrite("sndAutoCloseTime", &pj::MediaConfig::sndAutoCloseTime)
        .def_readwrite(
            "vidPreviewEnableNative", &pj::MediaConfig::vidPreviewEnableNative
        );

    /**
     * @brief EpConfig
     *
     */
    py::class_<pj::EpConfig, pj::PersistentObject>(m, "EpConfig")
        .def(py::init())
        .def_readonly("uaConfig", &pj::EpConfig::uaConfig)
        .def_readonly("logConfig", &pj::EpConfig::logConfig)
        .def_readonly("medConfig", &pj::EpConfig::medConfig);

    py::class_<pj::AccountSipConfig, pj::PersistentObject>(
        m, "AccountSipConfig"
    )
        .def(py::init())
        .def_readwrite("authCreds", &pj::AccountSipConfig::authCreds)
        .def_readwrite("proxies", &pj::AccountSipConfig::proxies)
        .def_readwrite("contactForced", &pj::AccountSipConfig::contactForced)
        .def_readwrite("contactParams", &pj::AccountSipConfig::contactParams)
        .def_readwrite(
            "contactUriParams", &pj::AccountSipConfig::contactUriParams
        )
        .def_readwrite(
            "authInitialEmpty", &pj::AccountSipConfig::authInitialEmpty
        )
        .def_readwrite(
            "authInitialAlgorithm", &pj::AccountSipConfig::authInitialAlgorithm
        )
        .def_readwrite("transportId", &pj::AccountSipConfig::transportId)
        .def_readwrite("ipv6Use", &pj::AccountSipConfig::ipv6Use);

    py::class_<pj::AccountRegConfig, pj::PersistentObject>(
        m, "AccountRegConfig"
    )
        .def(py::init())
        .def_readwrite("registrarUri", &pj::AccountRegConfig::registrarUri)
        .def_readwrite("registerOnAdd", &pj::AccountRegConfig::registerOnAdd)
        .def_readwrite("contactParams", &pj::AccountRegConfig::contactParams)
        .def_readwrite(
            "contactUriParams", &pj::AccountRegConfig::contactUriParams
        )
        .def_readwrite("timeoutSec", &pj::AccountRegConfig::timeoutSec)
        .def_readwrite(
            "retryIntervalSec", &pj::AccountRegConfig::retryIntervalSec
        )
        .def_readwrite(
            "firstRetryIntervalSec",
            &pj::AccountRegConfig::firstRetryIntervalSec
        )
        .def_readwrite(
            "randomRetryIntervalSec",
            &pj::AccountRegConfig::randomRetryIntervalSec
        )
        .def_readwrite(
            "delayBeforeRefreshSec",
            &pj::AccountRegConfig::delayBeforeRefreshSec
        )
        .def_readwrite(
            "dropCallsOnFail", &pj::AccountRegConfig::dropCallsOnFail
        )
        .def_readwrite("unregWaitMsec", &pj::AccountRegConfig::unregWaitMsec)
        .def_readwrite("proxyUse", &pj::AccountRegConfig::proxyUse);

    py::class_<pj::AccountConfig>(m, "AccountConfig")
        .def(py::init())
        .def_readwrite("priority", &pj::AccountConfig::priority)
        .def_readwrite("idUri", &pj::AccountConfig::idUri)
        .def_readonly("regConfig", &pj::AccountConfig::regConfig)
        .def_readonly("sipConfig", &pj::AccountConfig::sipConfig);

    ////////////////////////////////////////////////////////////////////
    // 主要类型
    ////////////////////////////////////////////////////////////////////

    py::class_<
        pj::AudDevManager, std::unique_ptr<pj::AudDevManager, py::nodelete>>(
        m, "AudDevManager"
    )
        .def_property(
            "captureDev", &pj::AudDevManager::getCaptureDev,
            &pj::AudDevManager::setCaptureDev
        )
        .def_property_readonly(
            "captureDevMedia", &pj::AudDevManager::getCaptureDevMedia
        )
        .def_property(
            "playbackDev", &pj::AudDevManager::getPlaybackDev,
            &pj::AudDevManager::setPlaybackDev
        )
        .def_property_readonly(
            "playbackDevMedia", &pj::AudDevManager::getPlaybackDevMedia
        )
        .def_property_readonly("enumDev2", &pj::AudDevManager::enumDev2)
        .def("setNullDev", &pj::AudDevManager::setNullDev)
        .def_property_readonly("sndIsActive", &pj::AudDevManager::sndIsActive)
        .def("refreshDevs", &pj::AudDevManager::refreshDevs)
        .def_property_readonly("devCount", &pj::AudDevManager::getDevCount)
        .def("getDevInfo", &pj::AudDevManager::getDevInfo, py::arg("id"))
        .def(
            "lookupDev", &pj::AudDevManager::lookupDev, py::arg("drv_name"),
            py::arg("dev_name")
        )
        .def("capName", &pj::AudDevManager::capName, py::arg("cap"))
        /// more ...
        .def_property(
            "inputLatency", &pj::AudDevManager::getInputLatency,
            &pj::AudDevManager::setInputLatency
        )
        .def_property(
            "outputLatency", &pj::AudDevManager::getOutputLatency,
            &pj::AudDevManager::setOutputLatency
        )
        .def_property(
            "inputVolume", &pj::AudDevManager::getInputVolume,
            &pj::AudDevManager::setInputVolume
        )
        .def_property(
            "outputVolume", &pj::AudDevManager::getOutputVolume,
            &pj::AudDevManager::setOutputVolume
        )
        .def_property_readonly(
            "inputSignal", &pj::AudDevManager::getInputSignal
        )
        .def_property_readonly(
            "outputSignal", &pj::AudDevManager::getOutputSignal
        )
        .def_property(
            "inputRoute", &pj::AudDevManager::getInputRoute,
            &pj::AudDevManager::setInputRoute
        )
        .def_property(
            "outputRoute", &pj::AudDevManager::getOutputRoute,
            &pj::AudDevManager::setOutputRoute
        )
        .def_property(
            "vad", &pj::AudDevManager::getVad, &pj::AudDevManager::setVad
        )
        .def_property(
            "cng", &pj::AudDevManager::getCng, &pj::AudDevManager::setCng
        )
        .def_property(
            "plc", &pj::AudDevManager::getPlc, &pj::AudDevManager::setPlc
        )
        /// more ...
        ;

    py::class_<pj::Version>(m, "Version")
        .def_readwrite("major", &pj::Version::major)
        .def_readwrite("rev", &pj::Version::rev)
        .def_readwrite("suffix", &pj::Version::suffix)
        .def_readwrite("full", &pj::Version::full)
        .def_readwrite("numeric", &pj::Version::numeric);

    /**
     * @brief Endpoint
     *
     */
    py::class_<pj::Endpoint>(m, "Endpoint")
        .def(py::init())
        .def("libCreate", &pj::Endpoint::libCreate)
        .def("libInit", &pj::Endpoint::libInit, py::arg("prmEpConfig"))
        .def("libStart", &pj::Endpoint::libStart)
        .def("libDestroy", &pj::Endpoint::libDestroy, py::arg("prmFlags") = 0)
        .def_property_readonly("version", &pj::Endpoint::libVersion)
        .def_property_readonly("state", &pj::Endpoint::libGetState)
        .def_static("instance", &pj::Endpoint::instance)
        .def(
            "transportCreate", &pj::Endpoint::transportCreate, py::arg("type_"),
            py::arg("cfg")
        )
        .def_property_readonly("transportEnum", &pj::Endpoint::transportEnum)
        .def("transportGetInfo", &pj::Endpoint::transportGetInfo, py::arg("id"))
        .def(
            "transportSetEnable", &pj::Endpoint::transportSetEnable,
            py::arg("id"), py::arg("enabled")
        )
        .def("transportClose", &pj::Endpoint::transportClose, py::arg("id"))
        /// NOTE: typedef void* pj::TransportHandle
        // .def(
        //     "transportShutdown", &pj::Endpoint::transportShutdown,
        //     py::arg("tp")
        // )
        .def("hangupAllCalls", &pj::Endpoint::hangupAllCalls)
        .def("mediaAdd", &pj::Endpoint::mediaAdd, py::arg("media"))
        .def("mediaRemove", &pj::Endpoint::mediaRemove, py::arg("media"))
        .def("mediaExists", &pj::Endpoint::mediaExists, py::arg("media"))
        .def_property_readonly("mediaMaxPorts", &pj::Endpoint::mediaMaxPorts)
        .def_property_readonly(
            "mediaActivePorts", &pj::Endpoint::mediaActivePorts
        )
        .def_property_readonly(
            "mediaEnumPorts2", &pj::Endpoint::mediaEnumPorts2
        )
        .def_property_readonly(
            "audDevManager", &pj::Endpoint::audDevManager,
            py::return_value_policy::reference
        )
        // more ...
        ;

    py::class_<pj::Account, PyAccount<>>(m, "Account")
        .def(py::init())
        .def(
            "create", &pj::Account::create, py::arg("cfg"),
            py::arg("make_default") = false
        )
        .def("shutdown", &pj::Account::shutdown)
        .def("modify", &pj::Account::modify, py::arg("cfg"))
        .def_property_readonly("valid", &pj::Account::isValid)
        .def_property(
            "default", &pj::Account::isDefault, &pj::Account::setDefault
        )
        .def_property_readonly("id", &pj::Account::getId)
        .def_static("lookup", &pj::Account::lookup, py::arg("acc_id"))
        .def_property_readonly("info", &pj::Account::getInfo)
        .def("setRegistration", &pj::Account::setRegistration, py::arg("renew"))
        .def("setTransport", &pj::Account::setTransport, py::arg("tp_id"))
        .def("onIncomingCall", &pj::Account::onIncomingCall, py::arg("prm"))
        .def("onRegStarted", &pj::Account::onRegStarted, py::arg("prm"))
        .def("onRegState", &pj::Account::onRegState, py::arg("prm"));

    py::class_<pj::Call, PyCall<>>(m, "Call")
        .def(
            py::init<pj::Account&, int>(), py::arg("acc"),
            py::arg("call_id") = static_cast<int>(PJSUA_INVALID_ID)
        )
        .def_property_readonly("info", &pj::Call::getInfo)
        .def_property_readonly("active", &pj::Call::isActive)
        .def_property_readonly("id", &pj::Call::getId)
        .def_static("lookup", &pj::Call::lookup, py::arg("id_"))
        .def_property_readonly("hasMedia", &pj::Call::hasMedia)
        .def("getMedia", &pj::Call::getMedia, py::arg("med_idx"))
        .def("getAudioMedia", &pj::Call::getAudioMedia, py::arg("med_idx"))
        .def(
            "makeCall", &pj::Call::makeCall, py::arg("dst_uri"), py::arg("prm")
        )
        .def("answer", &pj::Call::answer, py::arg("prm"))
        .def("hangup", &pj::Call::hangup, py::arg("prm"))
        .def("setHold", &pj::Call::setHold, py::arg("prm"))
        .def("reinvite", &pj::Call::reinvite, py::arg("prm"))
        .def("update", &pj::Call::update, py::arg("prm"))
        .def("xfer", &pj::Call::xfer, py::arg("dest"), py::arg("prm"))
        .def(
            "xferReplaces", &pj::Call::xferReplaces, py::arg("dest_call"),
            py::arg("prm")
        )
        .def("processRedirect", &pj::Call::processRedirect, py::arg("cmd"))
        .def("dialDtmf", &pj::Call::dialDtmf, py::arg("digits"))
        .def("sendDtmf", &pj::Call::sendDtmf, py::arg("param"))

        .def("onCallState", &pj::Call::onCallState, py::arg("prm"))
        .def("onCallTsxState", &pj::Call::onCallTsxState, py::arg("prm"))
        .def("onCallMediaState", &pj::Call::onCallMediaState, py::arg("prm"))
        .def("onCallSdpCreated", &pj::Call::onCallSdpCreated, py::arg("prm"))
        .def("onStreamPreCreate", &pj::Call::onStreamPreCreate, py::arg("prm"))
        .def("onStreamCreated", &pj::Call::onStreamCreated, py::arg("prm"))
        .def("onStreamDestroyed", &pj::Call::onStreamDestroyed, py::arg("prm"))
        .def("onDtmfDigit", &pj::Call::onDtmfDigit, py::arg("prm"))
        .def("onDtmfEvent", &pj::Call::onDtmfEvent, py::arg("prm"));
}
