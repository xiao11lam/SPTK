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

#ifndef SPTK_ANALYSIS_PITCH_EXTRACTION_H_  // here it means if not defined, we give it a name, the name will be all in capital letter
// it means we only need one SPTK_ANALYSIS_PITCH_EXTRACTION_H_ not many times, this defines the name of the header file
// the logic is: if not defined, we define here
#define SPTK_ANALYSIS_PITCH_EXTRACTION_H_  // define and undefine here is to define a variable


#include <vector>  // std::vector

#include "SPTK/analysis/pitch_extraction_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Extract pitch (fundamental frequency) from waveform.
 *
 * The input is whole audio waveform and the output is the sequence of the
 * fundamental frequency. The implemented algorithms of the extraction are
 * RAPT, SWIPE, REAPER, and DIO.
 *
 * [1] D. Talkin, &quot;A robust algorithm for pitch tracking,&quot; Speech
 *     Coding and Synthesis, pp. 497-518, 1995.
 *
 * [2] A. Camacho, &quot;SWIPE: A sawtooth waveform inspired pitch estimator
 *     for speech and music,&quot; Doctoral dissertation, 2007.
 *
 * [3] D. Talkin, &quot;REAPER: Robust epoch and pitch estimator,&quot;
 *     https://github.com/google/REAPER, 2015.
 *
 * [4] M. Morise, H. Kawahara and H. Katayose, &quot;Fast and reliable F0
 *     estimation method based on the period extraction of vocal fold vibration
 *     of singing voice and speech, Proc. of AES 35th International Conference,
 *     2009.
 */
class PitchExtraction {
 public:  // visible outside/inside the class
  /**
   * Pitch extraction algorithm type.
   */
  enum Algorithms { kRapt = 0, kSwipe, kReaper, kWorld, kNumAlgorithms };

  /**
   * @param[in] frame_shift Frame shift in point.
   * @param[in] sampling_rate Sampling rate in Hz.
   * @param[in] lower_f0 Lower bound of F0 in Hz.
   * @param[in] upper_f0 Upper bound of F0 in Hz.
   * @param[in] voicing_threshold Threshold for determining voiced/unvoiced.
   * @param[in] algorithm Algorithm used for pitch extraction.
   */

  // 这里我们叫constructor，it is a special kind of function that is automatically called when an object is created
  // 它的名字和类的名字一模一样，而且不会返回return任何值， not even void, the purpose is to initialize the variable or allocation of the heap storage.
  // the constructor will run automatically.
  PitchExtraction(int frame_shift, double sampling_rate, double lower_f0,
                  double upper_f0, double voicing_threshold,
                  Algorithms algorithm);

  virtual ~PitchExtraction() {
    // deconstructor's name also the same.
    // 这个叫deconstructor主要就是来清除constructor不用之后的垃圾的，释放内存的。如果不放在这里的话，内存就可能会不断使用，导致内存泄露。
    delete pitch_extraction_;
  }

  /**
   * @return True if this object is valid.
   */

  /**
   *
   * 声明一般是加到头文件里面，
   *
   */

  bool IsValid() const {
    return (NULL != pitch_extraction_ && pitch_extraction_->IsValid());
  }

  /**
   * @param[in] waveform Waveform.
   * @param[out] f0 Extracted pitch in Hz.
   * @param[out] epochs Pitchmark (valid only for REAPER).
   * @param[out] polarity Polarity (valid only for REAPER).
   */
  bool Run(const std::vector<double>& waveform, std::vector<double>* f0,
           std::vector<double>* epochs,
           PitchExtractionInterface::Polarity* polarity) const {
    return (NULL != pitch_extraction_ &&
            pitch_extraction_->Get(waveform, f0, epochs, polarity));
  }

 private: // visible only inside the class
  PitchExtractionInterface* pitch_extraction_;

  DISALLOW_COPY_AND_ASSIGN(PitchExtraction);
};

}  // namespace sptk


// this is the end of the if definition statement!
#endif  // SPTK_ANALYSIS_PITCH_EXTRACTION_H_
