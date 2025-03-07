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

#include <cfloat>     // DBL_MAX, FLT_MAX
#include <climits>    // INT_MIN, INT_MAX, SCHAR_MIN, SCHAR_MAX, etc.
#include <cstdint>    // int8_t, int16_t, int32_t, int64_t, etc.
#include <cstring>    // std::strncmp
#include <fstream>    // std::ifstream  输入流的头文件，支持输入流
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc. 
// 这里是加了系统io的头文件
#include <sstream>    // std::ostringstream
#include <stdexcept>  // std::invalid_argument
#include <string>     // std::stold, std::string  为了支持我们读取字符串


// 这里引用双引号的原因是系统会从当前的目录中寻找头文件，不像<>从系统环境变量中找，这里的头文件是自己编写的，所以用了""，而且要加后缀，原生的头文件<>不需要。
#include "Getopt/getoptwin.h"
#include "SPTK/utils/int24_t.h"
#include "SPTK/utils/sptk_utils.h"
#include "SPTK/utils/uint24_t.h"



// 这个是名字空间，防止命名重复问题，里面包含所有的东西包含在这个namespace里面的东西，如果没有名字空间的话，就可能会在不同的脚本中发生命名冲突，出现redefinition问题。
// 在C++中如果不定义一个东西处于那个命名空间，那么就会把他放在全局空间里面。unnamed namespace / anonymous namespace, 
// It's use is to make functions/objects/etc accessible only within that file. It's almost the same as static in C.


namespace {

/*
 *
 *
 * 枚举(enum) ：一种取值受限的特殊类型
– 分为无作用域枚举与有作用域枚举（ C++11 起）两种
– 枚举项缺省使用 0 初始化，依次递增，可以使用常量表达式来修改缺省值
– 可以为枚举指定底层类型，表明了枚举项的尺寸
– 无作用域枚举项可隐式转换为整数值；也可用 static_cast 在枚举项与整数值间转换
– 注意区分枚举的定义与声明

 */

// in enum here like give each element a ID value, 这里面给NumericType里的每一个元素相当于一个值，这样的话可以将类别类型的数据变成编号类型
enum NumericType {
  kUnknown = 0,
  kSignedInteger,
  kUnsignedInteger,
  kFloatingPoint,
};

enum WarningType { kIgnore = 0, kWarn, kExit, kNumWarningTypes };

const int kBufferSize(128);

// char字符串类型，能取256种数据
const char* kDefaultDataTypes("da");
const bool kDefaultRoundingFlag(false);
const WarningType kDefaultWarningType(kExit);
const int kDefaultNumColumn(1);
  
//  一般输出很耗内存，所以我们可以通过缓存的方式一次性全部打印出来。ifstream是输入文件流，是一种输入流， std::ifstream x; 。

void PrintUsage(std::ostream* stream) {
  // stream 其实是一个形式参数，当我们要调用这个参数的时候需要喂入实参数。
  // 这里* stream代表了stream是个指针，这里std::ostream代表了指针的长度。
  
  // clang-format off
  
//   << 表示输出， >> 表示输入， 这里都是格式化I/O主要是非格式化会用到一些像是常用的输入函数像是get、read、getline，gcount等等函数就是一些unformated input。输出像是put、write
//   std::cout << x << std::end; 这里std::out输入终端里面， c是character的意思，字符的意思。
//   std::cin >> x; printf 是C语言的IO
//   我们构造了一个内存流指针stream，相当于我们把这些东西都放到了*stream这个流里面了。如果对象很大的话，我们就需要。
// 这里*stream代表了解除引用，取到该指针所指向的地址就是那块内存地址里面的值（give me the value of the pointer stream)。这里的stream 相当于一个内存地址就是0x.....，
  *stream << std::endl;
  *stream << " x2x - data type transformation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       x2x [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       +type : input and output data types                [" << std::setw(5) << std::right << kDefaultDataTypes  << "]" << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("c", stream); sptk::PrintDataType("C", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("s", stream); sptk::PrintDataType("S", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("h", stream); sptk::PrintDataType("H", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("i", stream); sptk::PrintDataType("I", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("l", stream); sptk::PrintDataType("L", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("f", stream); sptk::PrintDataType("d", stream); *stream << std::endl;  // NOLINT
  *stream << "                 "; sptk::PrintDataType("e", stream); sptk::PrintDataType("a", stream); *stream << std::endl;  // NOLINT
  *stream << "       -r    : rounding                           (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultRoundingFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -e e  : warning type of out-of-range value (   int)[" << std::setw(5) << std::right << kDefaultWarningType << "][ 0 <= e <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (no warning)" << std::endl;
  *stream << "                 1 (output the index to stderr)" << std::endl;
  *stream << "                 2 (output the index to stderr" << std::endl;
  *stream << "                    and exit immediately)" << std::endl;
  *stream << "       -c c  : number of columns                  (   int)[" << std::setw(5) << std::right << kDefaultNumColumn   << "][ 1 <= c <=   ]" << std::endl;  // NOLINT
  *stream << "       -f f  : print format                       (string)[" << std::setw(5) << std::right << "N/A"               << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                                      [stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       transformed data sequence" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       values of f and c are valid only if output data type is ascii" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class DataTransformInterface {
 public:
  // public 继承：描述 是一个 的关系
  virtual ~DataTransformInterface() {
  }

  virtual bool Run(std::istream* input_stream) const = 0;
};
// 这里的;表示这个语句的结束.分号的作用是标志一个语句的结束.


template <typename T1, typename T2>


//使用 template 关键字引入模板： template<typename T> void fun(T) {...}
//– 函数模板的声明与定义
//– typename 关键字可以替换为 class ，含义相同
//– 函数模板中包含了两对参数：函数形参 / 实参；模板形参 / 实参


class DataTransform : public DataTransformInterface {
 public:

  // 这里先找到print_format的地址获取到了print_format的地址, & – 取地址操作符, * – 解引用操作符, & is refer to the address of the operator
  DataTransform(const std::string& print_format, int num_column,
                NumericType input_numeric_type, WarningType warning_type,
                bool rounding, bool is_ascii_input, bool is_ascii_output,
                T2 minimum_value = 0, T2 maximum_value = 0)
      : print_format_(print_format),
        num_column_(num_column),
        input_numeric_type_(input_numeric_type),
        warning_type_(warning_type),
        rounding_(rounding),
        is_ascii_input_(is_ascii_input),
        is_ascii_output_(is_ascii_output),
        minimum_value_(minimum_value),
        maximum_value_(maximum_value) {
  }

  //这里没有分号,表示这是复合语句.

  ~DataTransform() {
  }

  virtual bool Run(std::istream* input_stream) const {
    char buffer[kBufferSize];
    // char表示基本的字符信息类型, 这里是一串字符串数组长度是kBufferSize：就是128个元素。我们要声明这个叫“char”的变量长度是kBufferSize。这是约定俗成的。
    int index(0);
    // 这里;代表空语句就是什么也不做的意思,通常和循环语句联系在一起.
    // 无限循环
    for (;; ++index) {
      // this is just a while loop, the break represents when the loop ends.
      // Read.
      T1 input_data;
      if (is_ascii_input_) {
        std::string word;
        *input_stream >> word;
        if (word.empty()) break;
        try {
          // C++ 中的处理方法：通过关键字 try/catch/throw 引入异常处理机制
//          异常触发时的系统行为 栈展开 ——
//          – 抛出异常后续的代码不会被执行
//          – 局部对象会按照构造相反的顺序自动销毁
//          – 系统尝试匹配相应的 catch 代码段
          input_data = std::stold(word);
          /*
           * ●
              try / catch 语句块
              – 一个 try 语句块后面可以跟一到多个 catch 语句块
              – 每个 catch 语句块用于匹配一种类型的异常对象
              – catch 语句块的匹配按照从上到下进行
              – 使用 catch(...) 匹配任意异常
              – 在 catch 中调用 throw 继续抛出相同的异常
              ● 在一个异常未处理完成时抛出新的异常会导致程序崩溃
              – 不要在析构函数或 operator delete 函数重载版本中抛出异常
              – 通常来说， catch 所接收的异常类型为引用类型
           */
        } catch (std::invalid_argument&) {
          return false;
        }
      } else {
        if (!sptk::ReadStream(&input_data, input_stream)) {
          break;
        }
      }

      // Convert.
      T2 output_data(input_data);

      bool is_clipped(false);
      {
        // Clipping.
        if (minimum_value_ < maximum_value_) {
          // 一般都是常量在左边。
          if (kSignedInteger == input_numeric_type_) {
            if (static_cast<int64_t>(input_data) <
                static_cast<int64_t>(minimum_value_)) {
              output_data = minimum_value_;
              is_clipped = true;
            } else if (static_cast<int64_t>(maximum_value_) <
                       static_cast<int64_t>(input_data)) {
              output_data = maximum_value_;
              is_clipped = true;
            }
          } else if (kUnsignedInteger == input_numeric_type_) {
            if (static_cast<uint64_t>(input_data) <
                static_cast<uint64_t>(minimum_value_)) {
              output_data = minimum_value_;
              is_clipped = true;
            } else if (static_cast<uint64_t>(maximum_value_) <
                       static_cast<uint64_t>(input_data)) {
              output_data = maximum_value_;
              is_clipped = true;
            }
          } else if (kFloatingPoint == input_numeric_type_) {
            // 这里static_cast<long double其实是个布尔值
            if (static_cast<long double>(input_data) <
                static_cast<long double>(minimum_value_)) {
              output_data = minimum_value_;
              is_clipped = true;
            } else if (static_cast<long double>(maximum_value_) <
                       static_cast<long double>(input_data)) {
              output_data = maximum_value_;
              is_clipped = true;
            }
          }
        }
        // if 语句这里
        // Rounding.
        if (rounding_ && !is_clipped) {
          if (0.0 < input_data) {
            output_data = static_cast<T2>(input_data + 0.5);
          } else {
            output_data = static_cast<T2>(input_data - 0.5);
          }
        }
      }

      if (is_clipped && kIgnore != warning_type_) {
        std::ostringstream error_message;
        error_message << index << "th data is over the range of output type";
        sptk::PrintErrorMessage("x2x", error_message);
        if (kExit == warning_type_) return false;
      }

      // Write output.
      if (is_ascii_output_) {
        // 存储所需要的尺寸 (sizeof ，标准并没有严格限制), sizeof 不会返回动态分配的内存大小
        if (!sptk::SnPrintf(output_data, print_format_, sizeof(buffer),
                            buffer)) {
          return false;
        }
        std::cout << buffer;
        // 我们通常把常量放在左边，如果为了比较逻辑，变量是右值，放在右边。避免变量问题。
        if (0 == (index + 1) % num_column_) {
          std::cout << std::endl;
        } else {
          std::cout << "\t";
        }
      } else {
        if (!sptk::WriteStream(output_data, &std::cout)) {
          return false;
        }
      }
    }

    if (is_ascii_output_ && 0 != index % num_column_) {
      std::cout << std::endl;
    }

    return true;
  }

 private:
  //  private 继承：描述 根据基类实现出 的关系
  const std::string print_format_; // 这里定义了一个字符串叫print_format_
  // const在这里限定表示是个常量，表示这个整数是不能被修改的，就是只读的意思,
  const int num_column_;
  const NumericType input_numeric_type_;
  const WarningType warning_type_;
  const bool rounding_;
  const bool is_ascii_input_;
  const bool is_ascii_output_;
  const T2 minimum_value_;
  const T2 maximum_value_;

  DataTransform<T1, T2>(const DataTransform<T1, T2>&);
  void operator=(const DataTransform<T1, T2>&);
};

class DataTransformWrapper {
 public:
  // 这里通过&我们相当于传入了一个空指针，这样我们又不用担心空指针的问题，这里叫引用，但是指针和引用本身就是类似的。input_data_type还是个地址。这里加上const的原因是我们不希望后面
  // 给这个值错误赋值，导致错误，不然的话编译器就会报错。
  // this is called as call by reference instead of call by value, we pass the reference to input_data_type, 这样传值的话，这样传值的话不会新建一个内存空间
  // 并且可以把值都传过去，相当于复制了原来的值但是没有消耗新的存储空间。
  DataTransformWrapper(const std::string& input_data_type,
                       const std::string& output_data_type,
                       const std::string& given_print_format, int num_column,
                       WarningType warning_type, bool given_rounding_flag)
      // the NULL here is the C language way to define a null pointer here is another way to say integer number 0
      : data_transform_(NULL) {
    std::string print_format(given_print_format);
    // 所以说这里就是input_data_type的值是const不会报错。这种常量引用但是推荐的，能够快速拷贝。防止修改。一般是结构体复制成本比较高才这么搞像是const int& parm是没必要的
    // 完全可以是int param是因为int才4个字节，但是地址是8个所以这个时候没必要。
    if (print_format.empty() && "a" == output_data_type) {
      //
      if ("c" == input_data_type || "s" == input_data_type ||
          "h" == input_data_type || "i" == input_data_type) {
        print_format = "%d";
      } else if ("l" == input_data_type) {
        print_format = "%lld";
      } else if ("C" == input_data_type || "S" == input_data_type ||
                 "H" == input_data_type || "I" == input_data_type) {
        print_format = "%u";
      } else if ("L" == input_data_type) {
        print_format = "%llu";
      } else if ("f" == input_data_type || "d" == input_data_type) {
        print_format = "%g";
      } else if ("e" == input_data_type || "a" == input_data_type) {
        print_format = "%Lg";
      }
    }

    NumericType input_numeric_type(kUnknown);
    if ("c" == input_data_type || "s" == input_data_type ||
        "h" == input_data_type || "i" == input_data_type ||
        "l" == input_data_type) {
      input_numeric_type = kSignedInteger;
    } else if ("C" == input_data_type || "S" == input_data_type ||
               "H" == input_data_type || "I" == input_data_type ||
               "L" == input_data_type) {
      input_numeric_type = kUnsignedInteger;
    } else if ("f" == input_data_type || "d" == input_data_type ||
               "e" == input_data_type || "a" == input_data_type) {
      input_numeric_type = kFloatingPoint;
    }

    bool rounding(false);
    if (("f" == input_data_type || "d" == input_data_type ||
         "e" == input_data_type || "a" == input_data_type) &&
        ("c" == output_data_type || "C" == output_data_type ||
         "s" == output_data_type || "S" == output_data_type ||
         "h" == output_data_type || "H" == output_data_type ||
         "i" == output_data_type || "I" == output_data_type ||
         "l" == output_data_type || "L" == output_data_type)) {
      rounding = given_rounding_flag;
    }

    const bool is_ascii_input("a" == input_data_type);
    const bool is_ascii_output("a" == output_data_type);
    // -> 等价于 (*)
    // c -> *, -> 的左操作数指针，返回左值
    if ("c" == input_data_type && "c" == output_data_type) {
      // 在 C++ 中通常使用 new 与 delete 来构造、销毁对象
      data_transform_ = new DataTransform<int8_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("c" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(SCHAR_MAX));
    } else if ("c" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("c" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("c" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int8_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // s -> *
    if ("s" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("s" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("s" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("s" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("s" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(SHRT_MAX));
    } else if ("s" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("s" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("s" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("s" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int16_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // h -> *
    if ("h" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("h" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("h" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("h" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("h" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("h" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::INT24_MAX));
    } else if ("h" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, sptk::INT24_MAX);
    } else if ("h" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, sptk::INT24_MAX);
    } else if ("h" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("h" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<sptk::int24_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    }

    // i -> *
    if ("i" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("i" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("i" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("i" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("i" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("i" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("i" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("i" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("i" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("i" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int32_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // l -> *
    if ("l" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("l" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("l" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("l" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("l" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("l" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("l" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("l" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("l" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, LLONG_MAX);
    } else if ("l" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("l" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<int64_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // C -> *
    if ("C" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("C" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("C" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    } else if ("C" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("C" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint8_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // S -> *
    if ("S" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("S" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("S" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0), sptk::int24_t(0));
    } else if ("S" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("S" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    } else if ("S" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("S" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint16_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // H -> *
    if ("H" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("H" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("H" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("H" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("H" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("H" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    } else if ("H" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("H" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<sptk::uint24_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(0));
    }

    // I -> *
    if ("I" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("I" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("I" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("I" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("I" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("I" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("I" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("I" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("I" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint32_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // L -> *
    if ("L" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SCHAR_MAX);
    } else if ("L" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, SHRT_MAX);
    } else if ("L" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(0),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("L" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, INT_MAX);
    } else if ("L" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, LLONG_MAX);
    } else if ("L" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("L" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("L" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("L" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("L" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("L" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<uint64_t, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // f -> *
    if ("f" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<float, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("f" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<float, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("f" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<float, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("f" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<float, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("f" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<float, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("f" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<float, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("f" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<float, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("f" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<float, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("f" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<float, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("f" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<float, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("f" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<float, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("f" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<float, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("f" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<float, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("f" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<float, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // d -> *
    if ("d" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<double, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("d" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<double, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("d" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<double, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("d" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<double, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("d" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<double, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("d" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<double, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("d" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<double, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("d" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<double, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("d" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<double, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("d" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<double, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("d" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<double, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, -FLT_MAX, FLT_MAX);
    } else if ("d" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<double, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("d" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<double, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("d" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<double, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // e -> *
    if ("e" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<long double, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("e" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<long double, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("e" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("e" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<long double, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("e" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<long double, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("e" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("e" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("e" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("e" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("e" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("e" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<long double, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, -FLT_MAX, FLT_MAX);
    } else if ("e" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<long double, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, -DBL_MAX, DBL_MAX);
    } else if ("e" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("e" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }

    // a -> *
    if ("a" == input_data_type && "c" == output_data_type) {
      data_transform_ = new DataTransform<long double, int8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SCHAR_MIN, SCHAR_MAX);
    } else if ("a" == input_data_type && "s" == output_data_type) {
      data_transform_ = new DataTransform<long double, int16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, SHRT_MIN, SHRT_MAX);
    } else if ("a" == input_data_type && "h" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::int24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::int24_t(sptk::INT24_MIN),
          sptk::int24_t(sptk::INT24_MAX));
    } else if ("a" == input_data_type && "i" == output_data_type) {
      data_transform_ = new DataTransform<long double, int32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, INT_MIN, INT_MAX);
    } else if ("a" == input_data_type && "l" == output_data_type) {
      data_transform_ = new DataTransform<long double, int64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, LLONG_MIN, LLONG_MAX);
    } else if ("a" == input_data_type && "C" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint8_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UCHAR_MAX);
    } else if ("a" == input_data_type && "S" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint16_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, USHRT_MAX);
    } else if ("a" == input_data_type && "H" == output_data_type) {
      data_transform_ = new DataTransform<long double, sptk::uint24_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, sptk::uint24_t(0),
          sptk::uint24_t(sptk::UINT24_MAX));
    } else if ("a" == input_data_type && "I" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint32_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, UINT_MAX);
    } else if ("a" == input_data_type && "L" == output_data_type) {
      data_transform_ = new DataTransform<long double, uint64_t>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, 0, ULLONG_MAX);
    } else if ("a" == input_data_type && "f" == output_data_type) {
      data_transform_ = new DataTransform<long double, float>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, -FLT_MAX, FLT_MAX);
    } else if ("a" == input_data_type && "d" == output_data_type) {
      data_transform_ = new DataTransform<long double, double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output, -DBL_MAX, DBL_MAX);
    } else if ("a" == input_data_type && "e" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    } else if ("a" == input_data_type && "a" == output_data_type) {
      data_transform_ = new DataTransform<long double, long double>(
          print_format, num_column, input_numeric_type, warning_type, rounding,
          is_ascii_input, is_ascii_output);
    }
  }

  ~DataTransformWrapper() {
    // delete 来构造、销毁对象
    delete data_transform_;
  }

  bool IsValid() const {
    return NULL != data_transform_;
  }

  bool Run(std::istream* input_stream) const {
    // 成员访问运算符（ -> ）模拟指针行为
    return IsValid() && data_transform_->Run(input_stream);
  }

 private:
  DataTransformInterface* data_transform_;

  DISALLOW_COPY_AND_ASSIGN(DataTransformWrapper);
};

}  // namespace

/**
 * @a x2x [ @e option ] [ @e infile ]
 *
 * - @b +type @e char
 *   - data type
 *     \arg @c c char (1byte)
 *     \arg @c C unsigned char (1byte)
 *     \arg @c s short (2byte)
 *     \arg @c S unsigned short (2byte)
 *     \arg @c h int (3byte)
 *     \arg @c H unsigned int (3byte)
 *     \arg @c i int (4byte)
 *     \arg @c I unsigned int (4byte)
 *     \arg @c l long (8byte)
 *     \arg @c L unsigned long (8byte)
 *     \arg @c f float (4byte)
 *     \arg @c d double (8byte)
 *     \arg @c e long double (16byte)
 *     \arg @c a ascii
 * - @b -r
 *   - rounding
 * - @b -e @e int
 *   - warning type for out-of-range value
 *     \arg @c 0 no warning
 *     \arg @c 1 output index
 *     \arg @c 2 output index and exit immediately
 * - @b -c @e int
 *   - number of columns
 * - @b -f @e str
 *   - print format
 * - @b infile @e str
 *   - double-type data sequence
 * - @b stdout
 *   - double-type transformed data sequence
 *
 * @code{.sh}
 *   ramp -l 4 | x2x +da
 *   # 0
 *   # 1
 *   # 2
 *   # 3
 *   ramp -l 4 | x2x +da -c 2
 *   # 0       1
 *   # 2       3
 *   ramp -l 4 | sopr -a 0.5 | x2x +dc -r | x2x +ca -c 2
 *   # 1       2
 *   # 3       4
 *   ramp -l 4 | x2x +da -c 2 -f %.1f
 *   # 0.0     1.0
 *   # 2.0     3.0
 *   echo -1 1000 | x2x +aC -e 0 | x2x +Ca
 *   # 0
 *   # 255
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */


// main函数是最后的执行函数，一个程序一定要包含main函数，这是C++和底层达成的协议，这是操作系统将要调用的函数。main函数返回的类型一定是int。C++规定有两种形式写main
// 1. int main()
// 2. int main(int argc, char* argv[]) 这里面包含两个形参，一个是int，一个是argv[]其实是一个char*类型的一种数组，形参。这是约定俗成的。可以改变那个argc，argv[]的名称
// 但是要保证类型不变就行！
//只有两种形式！！！

// argc is the number of arguments being passed into your program from the command line and argv is the array of arguments, argc 就是你输入多少个
// 值，都是数字，1个就是1，2个就是2
// an array of characters which like a string of the param passed into the command line



int main(int argc, char* argv[]) {
  // argc – [in] Number of arguments.
  // argv – [in] Argument vector.
  bool rounding_flag(kDefaultRoundingFlag);
  WarningType warning_type(kDefaultWarningType);
  int num_column(kDefaultNumColumn);
  std::string print_format("");
  std::string data_types(kDefaultDataTypes);

  for (;;) {
    /* 如果条件永不为假，则循环将变为无限循环，for循环再传统意义上可以用于实验无限循环，由于构成循环的三个表达式中何人一个都不是必须的，用户可以将某些条件表达式留空来构成一个无限循环

    * 第一个句子是初始化用的，如果没有初始化的必要，就视为空语句，加上分号。

        第二个句子作为判断条件，如果没有判断条件，也视为空语句，后加一个分号。这种情况，会无限循环，相当于while(1)。如果for的执行部分，就是{}之间有break语句，可以退出。

        第三个句子是执行部分执行完毕再执行的语句；无则视为空语句；此时不用再加分号。
     */

    /*
     * #include <iostream>

        using namespace std;

        int main{

          for ( ;  ;)

          {printf('this loop will run for ever.\n')

          }

          return 0;
        这个就叫C++的无限循环
        }
     */
    // int getopt_long(int argc, char * const argv[], const char *optstring,const struct option *longopts, int *longindex);
    // getopt_long支持长选项的命令行解析
    // Argc: 通过命令行传给main函数参数的个数
    // argv: 通过命令行传给main函数参数组成的字符串指针数组
    // optstring: 选项字符串。用来告诉getopt可以解析哪些选项，哪些选项需要参数以及函数返回值等（字符后面紧跟一个冒号则需要参数，而两个则表示参数可选）
    // longopts: 长选项参数的名称、属性、以及解析后的返回值等结构信息
    // longindex: 用来记录解析到的当前长选项的索引，也就是longopts的下标。

          const int option_char(getopt_long(argc, argv, "re:c:f:h", NULL, NULL));
    // getopt函数将依次检查命令行是否指定了 -c， -f， -h（这需要多次调用getopt函数，直到其返回-1），当检查到上面某一个参数被指定时，函数会返回被指定的参数名称（即该字母)
    // 带有冒号，: 表示参数d是可以指定值的，如-f user
    // 单个字符，表示选项, -r 后面什么也没有。
    if (-1 == option_char) break;

    switch (option_char) {
      case 'r': {
        // -r bool
        rounding_flag = true;
        break;
      }
      case 'e': {
        // -e int
        const int min(0);
        const int max(static_cast<int>(kNumWarningTypes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("x2x", error_message);
          return 1;
        }
        warning_type = static_cast<WarningType>(tmp);
        break;
      }
      case 'c': {
        // -c int
        if (!sptk::ConvertStringToInteger(optarg, &num_column) ||
            // num_column事实上是个值
            num_column <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -c option must be a positive integer";
          sptk::PrintErrorMessage("x2x", error_message);
          return 1;
        }
        break;
      }
      case 'f': {
        // -f str
        print_format = optarg;
        if ("%" != print_format.substr(0, 1)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -f option must be begin with %";
          sptk::PrintErrorMessage("x2x", error_message);
          return 1;
        }
        break;
      }
      case 'h': {
        PrintUsage(&std::cout);
        return 0;
      }
      default: {
        PrintUsage(&std::cerr);
        return 1;
      }
    }
  }


  // 这里定义了一个const指针，这里表示不能通过修改input_file(NULL)的值来修改其指向的参数大小。
  const char* input_file(NULL);
  // optind表示的是下一个将被处理到的参数在argv中的下标值。
  for (int i(argc - optind); 1 <= i; --i) {
    const char* arg(argv[argc - i]);
    // strncmp(str1, str2, 1);
    // str1, str2	-pointers to the null-terminated byte strings to compare
    // count 1	-maximum number of characters to compare
    // Return value: Negative value if str1 is less than str2.
    // 0 if  str1 is equal to str2.
    // Positive value if str1 is greater than str2.

    if (0 == std::strncmp(arg, "+", 1)) {
      const std::string str(arg);
      // data_types 就是+。。那个东西
      // "std::string::npos" means "until the end of the string", 这里相当于(1, -1)
      // static const size_t npos = -1;
      // The substring is the portion of the object that starts at character position 1 and spans len characters (or until the end of the string, whichever comes first).
      // 这里相当于提取除了“+。。”，然后去掉了+号。
      data_types = str.substr(1, std::string::npos);
      // +号后面要有两个值，如果只有一个值或者多于两个值就会报错。
      if (2 != data_types.size()) {
        std::ostringstream error_message;
        error_message << "The +type option must be two characters";
        sptk::PrintErrorMessage("x2x", error_message);
        return 1;
      }
    } else if (NULL == input_file) {
      input_file = arg;
    } else {
      std::ostringstream error_message;
      error_message << "Too many input files";
      sptk::PrintErrorMessage("x2x", error_message);
      return 1;
    }
  }
  // ifstream 是用于输入的,不同于ofstrem用于输出,fstream是同时接受输入和输出.
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("x2x", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const std::string input_data_type(data_types.substr(0, 1));
  // 这里定义了了一个叫input_data_type的string,里面的值是data_types.substr(0, 1)
  const std::string output_data_type(data_types.substr(1, 1));
  DataTransformWrapper data_transform(input_data_type, output_data_type,
                                      print_format, num_column, warning_type,
                                      rounding_flag);

  if (!data_transform.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected argument for the +type option";
    sptk::PrintErrorMessage("x2x", error_message);
    return 1;
  }

  if (!data_transform.Run(&input_stream)) {
    std::ostringstream error_message;
    error_message << "Failed to transform";
    sptk::PrintErrorMessage("x2x", error_message);
    return 1;
  }

  // 0 在这里表示正常返回。0会返回给操作系统。main函数可以没有return 0；这个语句，系统默认会帮你加上。
  return 0;
}
