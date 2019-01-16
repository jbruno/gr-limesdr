/* -*- c++ -*- */
/*
 * Copyright 2018 Lime Microsystems info@limemicro.com
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "source_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace limesdr {
source::sptr source::make(std::string serial,
                          int device_type,
                          int channel_mode,
                          int file_switch,
                          const char* filename,
                          double rf_freq,
                          double samp_rate,
                          size_t oversample,
                          int calibration_ch0,
                          double calibr_bandw_ch0,
                          int calibration_ch1,
                          double calibr_bandw_ch1,
                          int lna_path_mini,
                          int lna_path_ch0,
                          int lna_path_ch1,
                          int analog_filter_ch0,
                          double analog_bandw_ch0,
                          int analog_filter_ch1,
                          double analog_bandw_ch1,
                          int digital_filter_ch0,
                          double digital_bandw_ch0,
                          int digital_filter_ch1,
                          double digital_bandw_ch1,
                          int gain_dB_ch0,
                          int gain_dB_ch1,
                          float nco_freq_ch0,
                          float nco_freq_ch1) {
    return gnuradio::get_initial_sptr(new source_impl(serial,
                                                      device_type,
                                                      channel_mode,
                                                      file_switch,
                                                      filename,
                                                      rf_freq,
                                                      samp_rate,
                                                      oversample,
                                                      calibration_ch0,
                                                      calibr_bandw_ch0,
                                                      calibration_ch1,
                                                      calibr_bandw_ch1,
                                                      lna_path_mini,
                                                      lna_path_ch0,
                                                      lna_path_ch1,
                                                      analog_filter_ch0,
                                                      analog_bandw_ch0,
                                                      analog_filter_ch1,
                                                      analog_bandw_ch1,
                                                      digital_filter_ch0,
                                                      digital_bandw_ch0,
                                                      digital_filter_ch1,
                                                      digital_bandw_ch1,
                                                      gain_dB_ch0,
                                                      gain_dB_ch1,
                                                      nco_freq_ch0,
                                                      nco_freq_ch1));
}

source_impl::source_impl(std::string serial,
                         int device_type,
                         int channel_mode,
                         int file_switch,
                         const char* filename,
                         double rf_freq,
                         double samp_rate,
                         size_t oversample,
                         int calibration_ch0,
                         double calibr_bandw_ch0,
                         int calibration_ch1,
                         double calibr_bandw_ch1,
                         int lna_path_mini,
                         int lna_path_ch0,
                         int lna_path_ch1,
                         int analog_filter_ch0,
                         double analog_bandw_ch0,
                         int analog_filter_ch1,
                         double analog_bandw_ch1,
                         int digital_filter_ch0,
                         double digital_bandw_ch0,
                         int digital_filter_ch1,
                         double digital_bandw_ch1,
                         int gain_dB_ch0,
                         int gain_dB_ch1,
                         float nco_freq_ch0,
                         float nco_freq_ch1)
    : gr::block("source",
                gr::io_signature::make(
                    0, 0, 0), // Based on channel_mode SISO/MIMO use appropriate output signature
                args_to_io_signature(channel_mode, device_type)) {
    std::cout << "---------------------------------------------------------------" << std::endl;
    std::cout << "LimeSuite Source (RX) info" << std::endl;
    std::cout << std::endl;

    // 1. Store private variables upon implementation to protect from changing them later
    stored.serial = serial;
    stored.device_type = device_type;
    stored.channel_mode = channel_mode;
    stored.samp_rate = samp_rate;

    if (stored.device_type == LimeSDR_Mini || stored.device_type == LimeNET_Micro) {
        stored.channel = LMS_CH_0;
        stored.channel_mode = 1;
    } else if (stored.device_type == LimeSDR_USB) {
        if (stored.channel_mode < 3)                  // If SISO configure prefered channel
            stored.channel = stored.channel_mode - 1; // Channel is channel_mode - 1
        else if (stored.channel_mode == 3)            // If MIMO begin configuring channel 0
            stored.channel = LMS_CH_0;
    }

    if (stored.channel_mode < 1 && stored.channel > 3) {
        std::cout
            << "ERROR: source_impl::source_impl(): Channel must be A(1), B(2) or (A+B) MIMO(3)"
            << std::endl;
        exit(0);
    }

    // 2. Open device if not opened
    stored.device_number =
        device_handler::getInstance().open_device(stored.serial, stored.device_type);
    // 3. Check where to load settings from (file or block)
    if (file_switch == 1) {
        device_handler::getInstance().settings_from_file(stored.device_number, filename);
        device_handler::getInstance().check_blocks(stored.device_number,
                                                   source_block,
                                                   stored.device_type,
                                                   stored.channel_mode,
                                                   0,
                                                   0,
                                                   file_switch,
                                                   filename);
    } else {
        // 4. Check how many blocks were used and check values between blocks
        device_handler::getInstance().check_blocks(stored.device_number,
                                                   source_block,
                                                   stored.device_type,
                                                   stored.channel_mode,
                                                   samp_rate,
                                                   oversample,
                                                   file_switch,
                                                   NULL);

        // 5. Set SISO/MIMO mode
        device_handler::getInstance().set_chip_mode(stored.device_number,
                                                    stored.device_type,
                                                    stored.channel_mode,
                                                    stored.channel,
                                                    LMS_CH_RX);

        // 6. Set RF frequency
        device_handler::getInstance().set_rf_freq(
            stored.device_number, stored.device_type, LMS_CH_RX, stored.channel, rf_freq);


        // 7. Set sample rate
        // LimeSDR-Mini or LimeNET-Micro can only have the same rates
        if (stored.device_type == LimeSDR_Mini || stored.device_type == LimeNET_Micro)
            device_handler::getInstance().set_samp_rate(
                stored.device_number, samp_rate, oversample);
        // LimeSDR-USB can have separate rates for TX and RX
        else if (stored.device_type == LimeSDR_USB)
            device_handler::getInstance().set_samp_rate_dir(
                stored.device_number, LMS_CH_RX, samp_rate, oversample);

        // 8. Configure analog and digital filters
        device_handler::getInstance().set_analog_filter(
            stored.device_number, LMS_CH_RX, stored.channel, analog_filter_ch0, analog_bandw_ch0);
        device_handler::getInstance().set_digital_filter(
            stored.device_number, LMS_CH_RX, stored.channel, digital_filter_ch0, digital_bandw_ch0);


        // 9. Set LNA path
        if (stored.device_type == LimeSDR_Mini ||
            stored.device_type == LimeNET_Micro) // LimeSDR-Mini
            device_handler::getInstance().set_antenna(
                stored.device_number, stored.channel, LMS_CH_RX, lna_path_mini);
        else if (stored.device_type == LimeSDR_USB) // LimeSDR-USB
            device_handler::getInstance().set_antenna(
                stored.device_number, stored.channel, LMS_CH_RX, lna_path_ch0);

        // 10. Set GAIN
        device_handler::getInstance().set_gain(
            stored.device_number, LMS_CH_RX, stored.channel, gain_dB_ch0);

        // 11. Perform calibration
        device_handler::getInstance().calibrate(stored.device_number,
                                                stored.device_type,
                                                calibration_ch0,
                                                LMS_CH_RX,
                                                stored.channel,
                                                calibr_bandw_ch0,
                                                rf_freq,
                                                lna_path_ch0);

        // 12. Set NCO
        device_handler::getInstance().set_nco(
            stored.device_number, LMS_CH_RX, stored.channel, nco_freq_ch0, 0);

        // 13. Begin configuring device for channel 1 (if channel_mode is MIMO)
        if (stored.channel_mode == 3 && stored.device_type == LimeSDR_USB) {
            device_handler::getInstance().set_analog_filter(
                stored.device_number, LMS_CH_RX, LMS_CH_1, analog_filter_ch1, analog_bandw_ch1);
            device_handler::getInstance().set_digital_filter(
                stored.device_number, LMS_CH_RX, LMS_CH_1, digital_filter_ch1, digital_bandw_ch1);
            device_handler::getInstance().set_antenna(
                stored.device_number, LMS_CH_1, LMS_CH_RX, lna_path_ch1);
            device_handler::getInstance().set_gain(
                stored.device_number, LMS_CH_RX, LMS_CH_1, gain_dB_ch1);
            device_handler::getInstance().calibrate(stored.device_number,
                                                    stored.device_type,
                                                    calibration_ch1,
                                                    LMS_CH_RX,
                                                    LMS_CH_1,
                                                    calibr_bandw_ch1,
                                                    rf_freq,
                                                    lna_path_ch1);
            device_handler::getInstance().set_nco(
                stored.device_number, LMS_CH_RX, LMS_CH_1, nco_freq_ch1, 0);
        }
    }
    // device_handler::getInstance().debug_packets(stored.device_number);
    // 14. Initialize stream for channel 0 (if channel_mode is SISO)
    if (stored.channel_mode < 3) {
        this->init_stream(stored.device_number, stored.channel, stored.samp_rate);
    }

    // 15. Initialize both channels streams (if channel_mode is MIMO)
    else if (stored.channel_mode == 3 && stored.device_type == LimeSDR_USB) {
        this->init_stream(stored.device_number, LMS_CH_0, stored.samp_rate);
        this->init_stream(stored.device_number, LMS_CH_1, stored.samp_rate);
    }
    std::cout << "---------------------------------------------------------------" << std::endl;
}

source_impl::~source_impl() {
    // Stop and destroy stream for channel 0 (if channel_mode is SISO)
    if (stored.channel_mode < 3) {
        LMS_StopStream(&streamId[stored.channel]);
        LMS_DestroyStream(device_handler::getInstance().get_device(stored.device_number),
                          &streamId[stored.channel]);
    }
    // Stop and destroy stream for channels 0 & 1 (if channel_mode is MIMO)
    else if (stored.channel_mode == 3) {
        LMS_StopStream(&streamId[LMS_CH_0]);
        LMS_StopStream(&streamId[LMS_CH_1]);
        LMS_DestroyStream(device_handler::getInstance().get_device(stored.device_number),
                          &streamId[LMS_CH_0]);
        LMS_DestroyStream(device_handler::getInstance().get_device(stored.device_number),
                          &streamId[LMS_CH_1]);
    }
    device_handler::getInstance().close_device(stored.device_number, source_block);
}

bool source_impl::start(void) {
    std::unique_lock<std::recursive_mutex> lock(device_handler::getInstance().block_mutex);

    // Initialize and start stream for channel 0 (if channel_mode is SISO)
    if (stored.channel_mode < 3) // If SISO configure prefered channel
    {
        if (LMS_StartStream(&streamId[stored.channel]) != LMS_SUCCESS)
            device_handler::getInstance().error(stored.device_number);
    }

    // Initialize and start stream for channels 0 & 1 (if channel_mode is MIMO)
    else if (stored.channel_mode == 3 && stored.device_type == LimeSDR_USB) {
        if (LMS_StartStream(&streamId[LMS_CH_0]) != LMS_SUCCESS)
            device_handler::getInstance().error(stored.device_number);
        if (LMS_StartStream(&streamId[LMS_CH_1]) != LMS_SUCCESS)
            device_handler::getInstance().error(stored.device_number);
    }
    std::unique_lock<std::recursive_mutex> unlock(device_handler::getInstance().block_mutex);

    if (stream_analyzer) {
        t1 = std::chrono::high_resolution_clock::now();
        t2 = t1;
    }

    add_tag = true;

    return true;
}

bool source_impl::stop(void) {
    std::unique_lock<std::recursive_mutex> lock(device_handler::getInstance().block_mutex);
    // Stop stream for channel 0 (if channel_mode is SISO)
    if (stored.channel_mode < 3) {
        LMS_StopStream(&streamId[stored.channel]);
    }
    // Stop streams for channels 0 & 1 (if channel_mode is MIMO)
    else if (stored.channel_mode == 3) {
        LMS_StopStream(&streamId[LMS_CH_0]);
        LMS_StopStream(&streamId[LMS_CH_1]);
    }
    std::unique_lock<std::recursive_mutex> unlock(device_handler::getInstance().block_mutex);
    return true;
}

int source_impl::general_work(int noutput_items,
                              gr_vector_int& ninput_items,
                              gr_vector_const_void_star& input_items,
                              gr_vector_void_star& output_items) {
    // Receive stream for channel 0 (if channel_mode is SISO)
    if (stored.channel_mode < 3) {
        lms_stream_status_t status;
        lms_stream_meta_t rx_metadata;

        int ret0 = LMS_RecvStream(
            &streamId[stored.channel], output_items[0], noutput_items, &rx_metadata, 100);
        if (ret0 < 0) {
            return 0;
        }

        LMS_GetStreamStatus(&streamId[stored.channel], &status);

        if (add_tag || status.droppedPackets > 0) {
            pktLoss += status.droppedPackets;
            add_tag = false;
            this->add_time_tag(0, rx_metadata);
        }
        // Print stream stats to debug
        if (stream_analyzer == true) {
            this->print_stream_stats(status);
        }

        produce(0, ret0);
        return WORK_CALLED_PRODUCE;
    }
    // Receive stream for channels 0 & 1 (if channel_mode is MIMO)
    else if (stored.channel_mode == 3) {
        lms_stream_status_t status[2];

        lms_stream_meta_t rx_metadata[2];
        int ret0 = LMS_RecvStream(
            &streamId[LMS_CH_0], output_items[0], noutput_items, &rx_metadata[0], 100);
        int ret1 = LMS_RecvStream(
            &streamId[LMS_CH_1], output_items[1], noutput_items, &rx_metadata[1], 100);
        if (ret0 <= 0 || ret1 <= 0) {
            return 0;
        }

        LMS_GetStreamStatus(&streamId[LMS_CH_0], &status[0]);
        LMS_GetStreamStatus(&streamId[LMS_CH_1], &status[1]);

        if (add_tag || status[0].droppedPackets > 0 || status[1].droppedPackets > 0) {
            pktLoss += status[0].droppedPackets; // because every time GetStreamStatus is called,
                                                 // packet loss is reset
            add_tag = false;
            this->add_time_tag(LMS_CH_0, rx_metadata[0]);
            this->add_time_tag(LMS_CH_1, rx_metadata[1]);
        }

        // Print stream stats to debug
        if (stream_analyzer == true) {
            this->print_stream_stats(status[0]);
        }

        this->produce(0, ret0);
        this->produce(1, ret1);
        return WORK_CALLED_PRODUCE;
    }
    return 0;
}

// Setup stream
void source_impl::init_stream(int device_number, int channel, float samp_rate) {
    streamId[channel].channel = channel;
    streamId[channel].fifoSize = int(samp_rate) / 1e4;
    streamId[channel].throughputVsLatency = 0.5;
    streamId[channel].isTx = LMS_CH_RX;
    streamId[channel].dataFmt = lms_stream_t::LMS_FMT_F32;

    if (LMS_SetupStream(device_handler::getInstance().get_device(stored.device_number),
                        &streamId[channel]) != LMS_SUCCESS)
        device_handler::getInstance().error(stored.device_number);

    std::cout << "INFO: source_impl::init_stream(): source channel " << channel << " (device nr. "
              << device_number << ") stream setup done." << std::endl;
}

// Print stream status
void source_impl::print_stream_stats(lms_stream_status_t status) {
    t2 = std::chrono::high_resolution_clock::now();
    auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    if (timePeriod >= 1000) {
        std::cout << std::endl;
        std::cout << "RX";
        std::cout << "|rate: " << status.linkRate / 1e6 << " MB/s ";
        std::cout << "|dropped packets: " << pktLoss << " ";
        std::cout << "|FIFO: " << 100 * status.fifoFilledCount / status.fifoSize << "%"
                  << std::endl;
        pktLoss = 0;
        t1 = t2;
    }
}

// Add rx_time tag to stream
int source_impl::add_time_tag(int channel, lms_stream_meta_t meta) {

    uint64_t u_rate = (uint64_t)stored.samp_rate;
    double f_rate = stored.samp_rate - u_rate;
    uint64_t intpart = meta.timestamp / u_rate;
    double fracpart = (meta.timestamp - intpart * u_rate - intpart * f_rate) / stored.samp_rate;

    const pmt::pmt_t ID = pmt::string_to_symbol(stored.serial);
    const pmt::pmt_t t_val = pmt::make_tuple(pmt::from_uint64(intpart), pmt::from_double(fracpart));
    this->add_item_tag(channel, nitems_written(channel), TIME_TAG, t_val, ID);
}
// Return io_signature to manage module output count
// based on SISO (one output) and MIMO (two outputs) modes
inline gr::io_signature::sptr source_impl::args_to_io_signature(int channel_number,
                                                                int device_type) {
    if (channel_number < 3 || device_type == LimeSDR_Mini || device_type == LimeNET_Micro) {
        return gr::io_signature::make(1, 1, sizeof(gr_complex));
    } else if (channel_number == 3 && device_type == LimeSDR_USB) {
        return gr::io_signature::make(2, 2, sizeof(gr_complex));
    } else {
        std::cout << "ERROR: source_impl::args_to_io_signature(): channel_number must be 0 or 1."
                  << std::endl;
        exit(0);
    }
}
void source_impl::set_rf_freq(float rf_freq) {
    device_handler::getInstance().set_rf_freq(
        stored.device_number, stored.device_type, LMS_CH_RX, LMS_CH_0, rf_freq);
    add_tag = true;
}

void source_impl::set_nco(float nco_freq, int channel) {
    if ((stored.device_type == LimeSDR_Mini || stored.device_type == LimeNET_Micro) &&
        channel == 1) {
        // IGNORE CHANNEL 1 FOR LIMESDR-MINI
        std::cout << "source_impl::set_nco(): INFO: Setting bypassed. "
                  << device_string[stored.device_type - 1]
                  << " does not support channel 1 configuration." << std::endl;
    } else {
        device_handler::getInstance().set_nco(
            stored.device_number, LMS_CH_RX, channel, nco_freq, 0);
        add_tag = true;
    }
}

void source_impl::set_lna_path(int lna_path, int channel) {
    if ((stored.device_type == LimeSDR_Mini || stored.device_type == LimeNET_Micro) &&
        channel == 1) {
        // IGNORE CHANNEL 1 FOR LIMESDR-MINI
        std::cout << "source_impl::set_lna_path(): INFO: Setting bypassed. "
                  << device_string[stored.device_type - 1]
                  << " does not support channel 1 configuration." << std::endl;
    } else {
        device_handler::getInstance().set_antenna(
            stored.device_number, channel, LMS_CH_RX, lna_path);
    }
}

void source_impl::set_analog_filter(int analog_filter, float analog_bandw, int channel) {
    if ((stored.device_type == LimeSDR_Mini || stored.device_type == LimeNET_Micro) &&
        channel == 1) {
        // IGNORE CHANNEL 1 FOR LIMESDR-MINI
        std::cout << "source_impl::set_analog_filter(): INFO: Setting bypassed. "
                  << device_string[stored.device_type - 1]
                  << " does not support channel 1 configuration." << std::endl;
    } else {
        device_handler::getInstance().set_analog_filter(
            stored.device_number, LMS_CH_RX, channel, analog_filter, analog_bandw);
        add_tag = true;
    }
}

void source_impl::set_digital_filter(int digital_filter, float digital_bandw, int channel) {
    if ((stored.device_type == LimeSDR_Mini || stored.device_type == LimeNET_Micro) &&
        channel == 1) {
        // IGNORE CHANNEL 1 FOR LIMESDR-MINI
        std::cout << "source_impl::set_digital_filter(): INFO: Setting bypassed. "
                  << device_string[stored.device_type - 1]
                  << " does not support channel 1 configuration." << std::endl;
    } else {
        device_handler::getInstance().set_digital_filter(
            stored.device_number, LMS_CH_RX, channel, digital_filter, digital_bandw);
        add_tag = true;
    }
}

void source_impl::set_gain(int gain_dB, int channel) {
    if ((stored.device_type == LimeSDR_Mini || stored.device_type == LimeNET_Micro) &&
        channel == 1) {
        // IGNORE CHANNEL 1 FOR LIMESDR-MINI
        std::cout << "source_impl::set_gain(): INFO: Setting bypassed. "
                  << device_string[stored.device_type - 1]
                  << " does not support channel 1 configuration." << std::endl;
    } else {
        device_handler::getInstance().set_gain(stored.device_number, LMS_CH_RX, channel, gain_dB);
    }
}
} // namespace limesdr
} // namespace gr