#include "python_strategy.hpp"

#include "historicalData.hpp"

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include <iostream>
#include <utility>

namespace py = pybind11;

struct PythonUserStrategy::Impl {
    py::object user_instance;
};

static void log_python_error() {
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
}

PythonUserStrategy::PythonUserStrategy(const std::string& source_code) : impl_(std::make_unique<Impl>()) {
    py::gil_scoped_acquire gil;
    py::module_::import("netgrain");

    py::dict global;
    py::dict local;
    try {
        py::exec(source_code, global, local);
    } catch (py::error_already_set&) {
        log_python_error();
        throw std::runtime_error("Python error while loading strategy code");
    }

    if (!local.contains("Strategy")) {
        throw std::runtime_error("strategy code must define class Strategy");
    }

    try {
        py::object Strat = local["Strategy"];
        impl_->user_instance = Strat();
    } catch (py::error_already_set&) {
        log_python_error();
        throw std::runtime_error("Python error while instantiating Strategy");
    }
}

PythonUserStrategy::~PythonUserStrategy() {
    if (!impl_) {
        return;
    }
    py::gil_scoped_acquire gil;
    impl_->user_instance = py::object();
    impl_.reset();
}

PythonUserStrategy::PythonUserStrategy(PythonUserStrategy&&) noexcept = default;
PythonUserStrategy& PythonUserStrategy::operator=(PythonUserStrategy&&) noexcept = default;

void PythonUserStrategy::on_bar(const std::unordered_map<std::string, MarketDataRow>& bars, Broker& broker) {
    py::gil_scoped_acquire gil;
    py::dict market;
    for (const auto& [ticker, row] : bars) {
        py::dict rowd;
        rowd["date"] = row.date;
        rowd["open"] = static_cast<double>(row.open) / static_cast<double>(PRICE_SCALE_FACTOR);
        rowd["high"] = static_cast<double>(row.high) / static_cast<double>(PRICE_SCALE_FACTOR);
        rowd["low"] = static_cast<double>(row.low) / static_cast<double>(PRICE_SCALE_FACTOR);
        rowd["close"] = static_cast<double>(row.close) / static_cast<double>(PRICE_SCALE_FACTOR);
        rowd["volume"] = row.volume;
        rowd["open_int"] = row.open_int;
        market[py::str(ticker)] = rowd;
    }

    try {
        py::object py_broker = py::cast(&broker, py::return_value_policy::reference);
        impl_->user_instance.attr("on_bar")(market, py_broker);
    } catch (py::error_already_set&) {
        log_python_error();
        throw std::runtime_error("Python error in Strategy.on_bar");
    }
}
