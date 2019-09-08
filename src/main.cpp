// MIT License

// Copyright (c) 2019 Fredrik Salomonsson

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "cpu.hpp"

#include <fmt/format.h>
#include <docopt/docopt.h>

#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <array>
#include <cmath>

static const char USAGE[] =
R"(cpu-hist.

Usage:
  cpu-hist

Options:
  -h, --help    Display this help and exit.
)";

std::string usageToBar( const size_t bucket, const size_t num_cpus )
{
  const std::array<std::string,8>
    bar{"▁" ,"▂" ,"▃" ,"▄" ,"▅" ,"▆" ,"▇" ,"█"};

  const double percentage = bucket/ static_cast<double>(num_cpus) * (bar.size()-1);
  const size_t index = static_cast<size_t>(std::round(percentage));

  return bar[index];
}

int main( int argc, char** argv )
{
  const auto args = docopt::docopt( USAGE,
                                    {argv+1, argv + argc },
                                    true, /*Show help*/
                                    "cpu-hist version x.x.x");

  const std::string stat_filename = "/proc/stat";
  auto prev_info = parseCpuinfo( stat_filename );

  {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
  }

  auto curr_info = parseCpuinfo( stat_filename );

  std::vector<std::uint16_t> cpu_usage;
  cpu_usage.reserve(curr_info.size());
  for( size_t i = 0, I = curr_info.size(); i < I; ++i)
  {
    cpu_usage.emplace_back(getCpuUsage(prev_info[i], curr_info[i]));
  }

  size_t num_buckets = 5;
  std::vector<size_t> buckets(num_buckets, 0);
  for( size_t i = 1, I = cpu_usage.size(); i < I; ++i)
  {
    ++buckets[cpu_usage[i]/(100 / (num_buckets-1))];
  }

  const size_t num_cpus = cpu_usage.size()-1;
  std::stringstream text;
  for( auto bucket : buckets )
    text<<usageToBar(bucket, num_cpus);

  fmt::print(R"({{"text": "{}"}})", text.str());
  fmt::print("\n");

  return 0;
}
