#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "broker.hpp"

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(netgrain, m) {
    py::enum_<Side>(m, "Side")
        .value("BUY", Side::BUY)
        .value("SELL", Side::SELL);

    py::enum_<OrderType>(m, "OrderType")
        .value("MARKET", OrderType::MARKET)
        .value("LIMIT", OrderType::LIMIT)
        .value("STOP", OrderType::STOP);

    // broker instances are created from c++ and passed into on_bar
    // there is no public python constructor (because there would be no way to pass the correct engine in)
    py::class_<Broker>(m, "Broker")
        .def("place_order", &Broker::place_order)
        .def("cancel_order", &Broker::cancel_order)
        .def("get_balance", &Broker::get_balance)
        .def("get_positions", &Broker::get_positions, py::return_value_policy::reference)
        .def("get_pending_orders", &Broker::get_pending_orders, py::return_value_policy::reference)
        .def("get_cancelled_orders", &Broker::get_cancelled_orders, py::return_value_policy::reference)
        .def("get_fill_log", &Broker::get_fill_log, py::return_value_policy::reference);
}
