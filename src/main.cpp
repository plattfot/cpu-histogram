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

#include <fmt/ostream.h>
#include <docopt/docopt.h>
#include <json/json.h>

#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <array>
#include <cmath>
#include <algorithm>

#ifndef CPU_HIST_VERSION
#  define CPU_HIST_VERSION "0.0.0"
#endif

static const char USAGE[] =
R"(cpu-hist - custom module for waybar to show CPU usage as a histogram.

Usage:
  cpu-hist [options]
  cpu-hist --loop [--sleep=S] [options]

Options:
  -h, --help         Display this help and exit.
  --high-load LOAD   Set the threshold for the high-load class [default: 75]
  --bins N           Number of bins for the histogram [default: 5]
  --loop             If set, loop indefinitely.
  --sleep S          When looping, set time between iterations. [default: 10]
  --version          Show version
)";

class Histogram
{
public:
  Histogram( const size_t num_bins, const size_t num_cpus ):
    m_cpus( num_cpus ),
    m_bins(num_bins, 0)
  {}

  const std::vector<size_t>& bins() const { return m_bins; }

  /// Return the histogram as a string
  std::string str() const;

  /// Update bins with values in range [fist, last].
  template<typename Iterator>
  void update(Iterator first, const Iterator last);

  /// Set all bins to 0
  void reset();
private:
  size_t m_cpus;
  std::vector<size_t> m_bins;
};

std::vector<std::uint16_t> updateCpuUsage(std::vector<std::uint16_t>&& cpu_usage,
                                          const std::vector<CPUInfo>& prev_info,
                                          const std::vector<CPUInfo>& curr_info);

std::string usageToBar( const size_t value, const size_t total );
Json::Value updateJson( Json::Value&& output,
                        const Histogram& histogram,
                        const std::uint16_t high_load,
                        const std::vector<std::uint16_t>& cpu_usage);

int main( int argc, char** argv )
{
  const auto args = docopt::docopt(
    USAGE,
    {argv+1, argv + argc },
    true, /*Show help*/
    fmt::format("cpu-hist version {}", CPU_HIST_VERSION ));

  const std::string stat_filename = "/proc/stat";
  size_t num_bins = 5;
  if(auto it = args.find("--bins"); it != end(args))
  {
    assert(it->second.isString());
    num_bins = std::stoul( it->second.asString() );
  }

  std::uint16_t high_load = 75;
  if(auto it = args.find("--high-load"); it != end(args))
  {
    assert(it->second.isString());
    high_load = static_cast<std::uint16_t>(
      std::min(100ul, std::stoul(it->second.asString())));
  }

  auto prev_info = parseCpuinfo( stat_filename );

  {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
  }

  auto curr_info = parseCpuinfo( stat_filename );

  std::vector<std::uint16_t> cpu_usage;
  cpu_usage = updateCpuUsage( std::move(cpu_usage), prev_info, curr_info);

  const size_t num_cpus = cpu_usage.size()-1;
  Histogram histogram(num_bins, num_cpus);
  Json::Value output;
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "";

  histogram.update(std::next(begin(cpu_usage)), end(cpu_usage));
  output = updateJson(std::move(output), histogram, high_load, cpu_usage);
  fmt::print("{}\n", Json::writeString(builder, output));

  if( auto it = args.find("--loop"); it != end(args) )
  {
    assert(it->second.isBool());
    const bool loop = it->second.asBool();

    using namespace std::chrono_literals;
    auto seconds = 10s;
    if(auto it = args.find("--sleep"); it != end(args))
    {
      assert(it->second.isString());
      seconds = static_cast<decltype(seconds)>(
        std::stoul(it->second.asString()));
    }

    while(loop)
    {
      std::this_thread::sleep_for(seconds);
      histogram.reset();

      prev_info = curr_info;
      curr_info = parseCpuinfo( stat_filename );
      cpu_usage = updateCpuUsage( std::move(cpu_usage), prev_info, curr_info);

      histogram.update(std::next(begin(cpu_usage)), end(cpu_usage));

      output = updateJson(std::move(output), histogram, high_load, cpu_usage);
      fmt::print("{}\n", Json::writeString(builder, output));
    }
  }
  return 0;
}

auto Histogram::str() const -> std::string
{
  std::stringstream text;
  for( auto bin : m_bins )
    text<<usageToBar(bin, m_cpus);
  return text.str();
}

auto Histogram::reset() -> void
{
  std::fill(begin(m_bins), end(m_bins), 0);
}

template<typename Iterator>
auto Histogram::update(Iterator first, const Iterator last) -> void
{
  for( ; first != last; ++first)
  {
    ++m_bins[*first/(100 / (m_bins.size()-1))];
  }
}

std::vector<std::uint16_t> updateCpuUsage(std::vector<std::uint16_t>&& cpu_usage,
                                          const std::vector<CPUInfo>& prev_info,
                                          const std::vector<CPUInfo>& curr_info)
{
  cpu_usage.clear();
  cpu_usage.reserve(curr_info.size());
  for( size_t i = 0, I = curr_info.size(); i < I; ++i)
  {
    cpu_usage.emplace_back(getCpuUsage(prev_info[i], curr_info[i]));
  }

  return std::move(cpu_usage);
}

std::string usageToBar( const size_t value, const size_t total )
{
  const std::array<std::string,8>
    bar{"▁" ,"▂" ,"▃" ,"▄" ,"▅" ,"▆" ,"▇" ,"█"};

  const double percentage = value/ static_cast<double>(total) * (bar.size()-1);
  const size_t index = static_cast<size_t>(std::round(percentage));

  return bar[index];
}

Json::Value updateJson( Json::Value&& output,
                        const Histogram& histogram,
                        const std::uint16_t high_load,
                        const std::vector<std::uint16_t>& cpu_usage)
{
  output["text"] = histogram.str();
  output["percentage"] = cpu_usage[0];

  std::stringstream tooltip;
  fmt::print(tooltip, "Cpu: {}%\n", cpu_usage[0]);
  for(size_t i = 1, I = cpu_usage.size(); i < I; ++i )
  {
    fmt::print(tooltip, "Cpu{}: {}%\n", i-1, cpu_usage[i]);
  }
  output["tooltip"] = tooltip.str();

  if(cpu_usage[0] >= high_load )
  {
    output["class"] = "high-load";
  }

  return std::move(output);
}
