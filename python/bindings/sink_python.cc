/*
 * Copyright 2021 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(sink.h)                                                    */
/* BINDTOOL_HEADER_FILE_HASH(2c64a374c662a1949f8ef0584060255a)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <limesdr/sink.h>
// pydoc.h is automatically generated in the build directory
#include <sink_pydoc.h>

void bind_sink(py::module& m)
{

    using sink    = ::gr::limesdr::sink;


    py::class_<sink, gr::block, gr::basic_block,
        std::shared_ptr<sink>>(m, "sink", D(sink))

        .def(py::init(&sink::make),
           py::arg("serial"),
           py::arg("channel_mode"),
           py::arg("filename"),
           py::arg("length_tag_name"),
           D(sink,make)
        )
        




        
        .def("set_center_freq",&sink::set_center_freq,       
            py::arg("freq"),
            py::arg("chan") = 0,
            D(sink,set_center_freq)
        )


        
        .def("set_antenna",&sink::set_antenna,       
            py::arg("antenna"),
            py::arg("channel") = 0,
            D(sink,set_antenna)
        )


        
        .def("set_nco",&sink::set_nco,       
            py::arg("nco_freq"),
            py::arg("channel"),
            D(sink,set_nco)
        )


        
        .def("set_bandwidth",&sink::set_bandwidth,       
            py::arg("analog_bandw"),
            py::arg("channel") = 0,
            D(sink,set_bandwidth)
        )


        
        .def("set_digital_filter",&sink::set_digital_filter,       
            py::arg("digital_bandw"),
            py::arg("channel"),
            D(sink,set_digital_filter)
        )


        
        .def("set_gain",&sink::set_gain,       
            py::arg("gain_dB"),
            py::arg("channel") = 0,
            D(sink,set_gain)
        )


        
        .def("set_sample_rate",&sink::set_sample_rate,       
            py::arg("rate"),
            D(sink,set_sample_rate)
        )


        
        .def("set_oversampling",&sink::set_oversampling,       
            py::arg("oversample"),
            D(sink,set_oversampling)
        )


        
        .def("calibrate",&sink::calibrate,       
            py::arg("bandw"),
            py::arg("channel") = 0,
            D(sink,calibrate)
        )


        
        .def("set_buffer_size",&sink::set_buffer_size,       
            py::arg("size"),
            D(sink,set_buffer_size)
        )


        
        .def("set_tcxo_dac",&sink::set_tcxo_dac,       
            py::arg("dacVal") = 125,
            D(sink,set_tcxo_dac)
        )

        ;




}








