// ------------------------------------------------------------------------ //
// Copyright 2021 SPTK Working Group                                        //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ------------------------------------------------------------------------ //

#include <algorithm>  // std::transform
#include <cmath>      // std::log
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
// if the header file not in the current folder, we need to add the path
#include "SPTK/analysis/pitch_extraction.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum LongOptions {
  kT0 = 1000,
  kT1,
  kT2,
  kT3,
};

enum OutputFormats { kPitch = 0, kF0, kLogF0, kNumOutputFormats };

// this is something from the PitchExtraction class, 这里相当于定义了一个叫kDefaultAlgorithm的对象
// Create Object
const sptk::PitchExtraction::Algorithms kDefaultAlgorithm(
    sptk::PitchExtraction::Algorithms::kRapt);
const int kDefaultFrameShift(80);
const double kDefaultSamplingRate(16.0);
const double kDefaultLowerF0(60.0);
const double kDefaultUpperF0(240.0);
const double kDefaultVoicingThresholdForRapt(0.0);
const double kDefaultVoicingThresholdForSwipe(0.3);
const double kDefaultVoicingThresholdForReaper(0.9);
const double kDefaultVoicingThresholdForWorld(0.1);
const OutputFormats kDefaultOutputFormat(kPitch);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " pitch - pitch extraction" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       pitch [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -a a  : algorithm used for pitch      (   int)[" << std::setw(5) << std::right << kDefaultAlgorithm                 << "][    0 <= a <= 3     ]" << std::endl;  // NOLINT
  *stream << "               estimation" << std::endl;
  *stream << "                 0 (RAPT)" << std::endl;
  *stream << "                 1 (SWIPE')" << std::endl;
  *stream << "                 2 (REAPER)" << std::endl;
  *stream << "                 3 (WORLD)" << std::endl;
  *stream << "       -p p  : frame shift [point]           (   int)[" << std::setw(5) << std::right << kDefaultFrameShift                << "][    1 <= p <=       ]" << std::endl;  // NOLINT
  *stream << "       -s s  : sampling rate [kHz]           (double)[" << std::setw(5) << std::right << kDefaultSamplingRate              << "][  6.0 <  s <= 98.0  ]" << std::endl;  // NOLINT
  *stream << "       -L L  : minimum fundamental frequency (double)[" << std::setw(5) << std::right << kDefaultLowerF0                   << "][ 10.0 <  L <  H     ]" << std::endl;  // NOLINT
  *stream << "               to search for [Hz]" << std::endl;
  *stream << "       -H H  : maximum fundamental frequency (double)[" << std::setw(5) << std::right << kDefaultUpperF0                   << "][    L <  H <  500*s ]" << std::endl;  // NOLINT
  *stream << "               to search for [Hz]" << std::endl;
  // RAPT 是时域上经典的方法。
  // YIN and RAPT are well-known time-domain methods that estimate pitch directly from the signal waveform using autocorrelation.
  *stream << "       -t0 t : voicing threshold for RAPT    (double)[" << std::setw(5) << std::right << kDefaultVoicingThresholdForRapt   << "][ -0.6 <= t <= 0.7   ]" << std::endl;  // NOLINT
  // SWIPE is a harmonic comb pitch estimation method with cosine-shaped teeth that smoothly connects harmonic peaks with sub-harmonic valleys, and another feature is that 
  // it only uses the first few significant harmonics of the signal. 这是一种频域上的一种方法。
  *stream << "       -t1 t : voicing threshold for SWIPE'  (double)[" << std::setw(5) << std::right << kDefaultVoicingThresholdForSwipe  << "][  0.2 <= t <= 0.5   ]" << std::endl;  // NOLINT
  *stream << "       -t2 t : voicing threshold for REAPER  (double)[" << std::setw(5) << std::right << kDefaultVoicingThresholdForReaper << "][ -0.5 <= t <= 1.6   ]" << std::endl;  // NOLINT
  *stream << "       -t3 t : voicing threshold for WORLD   (double)[" << std::setw(5) << std::right << kDefaultVoicingThresholdForWorld  << "][ 0.02 <= t <= 0.2   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format                 (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat              << "][    0 <= o <= 2     ]" << std::endl;  // NOLINT
  *stream << "                 0 (1/F0)" << std::endl;
  *stream << "                 1 (F0)" << std::endl;
  *stream << "                 2 (log F0)" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       waveform                              (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       pitch                                 (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       if t is raised, the number of voiced frames increase in RAPT, REAPER, and WORLD" << std::endl;  // NOLINT
  *stream << "       if t is dropped, the number of voiced frames increase in SWIPE'" << std::endl;  // NOLINT
  *stream << "       the value of t should be in the recommended range but values outside the range can be given" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a pitch [ @e option ] [ @e infile ]
 *
 * - @b -a @e int
 *   - algorithm used for pitch extraction
 *     @arg @c 0 RAPT
 *     @arg @c 1 SWIPE'
 *     @arg @c 2 REAPER
 *     @arg @c 3 WORLD (DIO)
 * - @b -p @e int
 *   - frame shift [point] @f$(1 \le P)@f$
 * - @b -s @e double
 *   - sampling rate [kHz] @f$(6 < F_s < 98)@f$
 * - @b -L @e dobule
 *   - minimum F0 to search for [Hz] @f$(10 < F_l < F_h)@f$
 * - @b -H @e dobule
 *   - maximum F0 to search for [Hz] @f$(F_l < F_h < 500F_s)@f$
 * - @b -t0 @e dobule
 *   - voicing threshold for RAPT @f$(-0.6 \le T \le 0.7)@f$
 * - @b -t1 @e dobule
 *   - voicing threshold for SWIPE' @f$(0.2 \le T \le 0.5)@f$
 * - @b -t2 @e dobule
 *   - voicing threshold for REAPER @f$(-0.5 \le T \le 1.6)@f$
 * - @b -t3 @e dobule
 *   - voicing threshold for WORLD @f$(0.02 \le T \le 0.2)@f$
 * - @b -o @e int
 *   - output format
 *     @arg @c 0 pitch @f$(F_s / F_0)@f$
 *     @arg @c 1 F0
 *     @arg @c 2 log F0
 * - @b infile @e str
 *   - double-type waveform
 * - @b stdout
 *   - double-type pitch
 *
 * If @f$T@f$ is raised, the number of voiced frames increase except SWIPE'.
 *
 * The below is a simple example to extract pitch from @c data.d
 *
 * @code{.sh}
 *   pitch -s 16 -p 80 -L 80 -H 200 -o 1 < data.d > data.f0
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  sptk::PitchExtraction::Algorithms algorithm(kDefaultAlgorithm);
  int frame_shift(kDefaultFrameShift);
  double sampling_rate(kDefaultSamplingRate);
  double lower_f0(kDefaultLowerF0);
  double upper_f0(kDefaultUpperF0);
  std::vector<double> voicing_thresholds{
      kDefaultVoicingThresholdForRapt,
      kDefaultVoicingThresholdForSwipe,
      kDefaultVoicingThresholdForReaper,
      kDefaultVoicingThresholdForWorld,
  };
  OutputFormats output_format(kDefaultOutputFormat);




  // 这里有点像是一个类，区别是首先所以的元素都是public默认的，struct更多的是数据操作，定义一堆数组。
  const struct option long_options[] = {
      {"t0", required_argument, NULL, kT0},
      {"t1", required_argument, NULL, kT1},
      {"t2", required_argument, NULL, kT2},
      {"t3", required_argument, NULL, kT3},
      {0, 0, 0, 0},
  };

  for (;;) {
    const int option_char(
        getopt_long_only(argc, argv, "a:p:s:L:H:o:h", long_options, NULL));
    if (-1 == option_char) break;
    // switch(条件)语句 , switch实现分支语句最核心的是case,我们可以通过case语句来实现,以及default标签.
    switch (option_char) {
        // 这里就是指的是当这个option_char等于"a"的种种情况下的case,case后面一般是常量表达式就是一个值,用来匹配switch里面的条件.
      case 'a': {
        const int min(0);
        const int max(static_cast<int>(
                          sptk::PitchExtraction::Algorithms::kNumAlgorithms) -
                      1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -a option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        algorithm = static_cast<sptk::PitchExtraction::Algorithms>(tmp);

        // break主要用在switch或者循环语句里面,可以跳出当前的switch.执行switch下面的代码.没有的话会继续运行下面的条件代码,我们叫fall through.fall through一般很少考虑.
        break;
      }
      case 'p': {
        if (!sptk::ConvertStringToInteger(optarg, &frame_shift) ||
            frame_shift <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -p option must be a positive integer";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        const double min(6.0);
        const double max(98.0);
        if (!sptk::ConvertStringToDouble(optarg, &sampling_rate) ||
            sampling_rate <= min || max < sampling_rate) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a number "
                        << "in the interval (" << min << ", " << max << "]";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        break;
      }
      case 'L': {
        if (!sptk::ConvertStringToDouble(optarg, &lower_f0) ||
            lower_f0 <= 10.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -L option must be a number "
                        << "greater than 10";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        break;
      }
      case 'H': {
        if (!sptk::ConvertStringToDouble(optarg, &upper_f0) ||
            upper_f0 <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -H option must be a positive number";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        break;
      }
      case kT0: {
        double tmp;
        if (!sptk::ConvertStringToDouble(optarg, &tmp)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t0 option must be numeric";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        voicing_thresholds[sptk::PitchExtraction::Algorithms::kRapt] = tmp;
        break;
      }
      case kT1: {
        double tmp;
        if (!sptk::ConvertStringToDouble(optarg, &tmp)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t1 option must be numeric";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        voicing_thresholds[sptk::PitchExtraction::Algorithms::kSwipe] = tmp;
        break;
      }
      case kT2: {
        double tmp;
        if (!sptk::ConvertStringToDouble(optarg, &tmp)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t2 option must be numeric";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        voicing_thresholds[sptk::PitchExtraction::Algorithms::kReaper] = tmp;
        break;
      }
      case kT3: {
        double tmp;
        if (!sptk::ConvertStringToDouble(optarg, &tmp)) {
          std::ostringstream error_message;
          error_message << "The argument for the -t3 option must be numeric";
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        voicing_thresholds[sptk::PitchExtraction::Algorithms::kWorld] = tmp;
        break;
      }
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(kNumOutputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pitch", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'h': {
        PrintUsage(&std::cout);
        return 0;
      }
        // 这里表示无论输入什么值,那个值只要不是我们需要的都会报出这个错误,这是默认分支.因为如果之前满足条件,很早就break出去了.
      default: {
        PrintUsage(&std::cerr);
        return 1;
      }
    }
  }

  const double sampling_rate_in_hz(1000.0 * sampling_rate);
  if (0.5 * sampling_rate_in_hz <= upper_f0) {
    std::ostringstream error_message;
    error_message
        << "Upper fundamental frequency must be less than Nyquist frequency";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }

  if (upper_f0 <= lower_f0) {
    std::ostringstream error_message;
    error_message << "Lower fundamental frequency must be less than upper one";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);
  // call constructor with arguments
  sptk::PitchExtraction pitch_extraction(
      frame_shift, sampling_rate_in_hz, lower_f0, upper_f0,
      voicing_thresholds[algorithm], algorithm);
  if (!pitch_extraction.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize PitchExtraction";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }

  std::vector<double> waveform;
  {
    double tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      // 这里满足这个就继续执行,不满足就跳出
      waveform.push_back(tmp);
    }
  }
  if (waveform.empty()) return 0;

  std::vector<double> f0;
  if (!pitch_extraction.Run(waveform, &f0, NULL, NULL)) {
    std::ostringstream error_message;
    error_message << "Failed to extract pitch";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }

  switch (output_format) {
    case kPitch: {
      // 这里的begin指的是第一个元素，这个f0 vector的第一个元素。
      std::transform(f0.begin(), f0.end(), f0.begin(),
                     [sampling_rate_in_hz](double x) {
                       return (0.0 < x) ? sampling_rate_in_hz / x : 0.0;
                     });
      break;
    }
    case kF0: {
      // nothing to do
      break;
    }
    case kLogF0: {
      // 这里的end指的是最后一个元素.
      std::transform(f0.begin(), f0.end(), f0.begin(), [](double x) {
        return (0.0 < x) ? std::log(x) : sptk::kLogZero;
      });
      break;
    }
    default: {
      break;
    }
  }

  const int f0_length(static_cast<int>(f0.size()));
  if (!sptk::WriteStream(0, f0_length, f0, &std::cout, NULL)) {
    std::ostringstream error_message;
    error_message << "Failed to write pitch";
    sptk::PrintErrorMessage("pitch", error_message);
    return 1;
  }

  return 0;
}
