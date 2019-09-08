// MIT License

// Copyright (c) 2018 Alex

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

// Code from https://github.com/Alexays/Waybar, to compute CPU usage.

#include "cpu.hpp"

#include <fmt/ostream.h>
#include <fstream>
#include <sstream>
#include <numeric>

std::vector<CPUInfo> parseCpuinfo( const std::string& stat_filename )
{
  std::ifstream stat( stat_filename );

  if( !stat.is_open() )
  {
    throw std::runtime_error(fmt::format("Could not open {}\n", stat_filename));
  }

  // Based on https://github.com/Alexays/Waybar/blob/master/src/modules/cpu.cpp
  std::string line;
  std::vector<CPUInfo> cpus;
  while( getline( stat, line ) )
  {
    if( line.substr(0,3).compare("cpu") != 0 )
      break;

    std::stringstream sline(line.substr(5));
    std::vector<size_t> times;

    for( size_t time = 0; sline>>time; times.emplace_back(time));

    CPUInfo cpu;
    if( times.size() >= 4 )
    {
      cpu.idle_time = times[3];
      cpu.total_time = std::accumulate(begin(times), end(times), 0);
    }
    cpus.emplace_back(std::move(cpu));
  }
  return cpus;
}

/// Compute the CPU usage.
std::uint16_t getCpuUsage( const CPUInfo& prev, const CPUInfo& curr )
{
  const double delta_idle = curr.idle_time - prev.idle_time;
  const double delta_total = curr.total_time - prev.total_time;

  return 100 * (1 - delta_idle / delta_total);
}


