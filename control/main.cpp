#include <iostream>
#include "../firmware/messages.h"
#include <boost/asio.hpp>

#include <chrono>
#include <thread>
#include <vector>
#include <iostream>
#include <array>


std::vector<Message> chunker(const std::array<RGB, 228>& buffer)
{
  std::vector<Message> res;
  Message x;
  x.type = COLOR;
  x.color.offset = 0;
  size_t index = 0;
  for (uint8_t i = 0; i < buffer.size(); i++)
  {
    x.color.offset = (index / 19) * 19;
    x.color.color[index % 19] = buffer[i];

    index++;
    if (index % 19 == 0)
    {
      res.push_back(x);
    }
  }

  res.back().color.settings = COLOR_SETTINGS_SHOW_AFTER;
  return res;
}

std::array<RGB, 228> empty()
{
  std::array<RGB, 228> res;
  RGB empty{0,0,0};
  res.fill(empty);
  return res;
}

void write(boost::asio::serial_port& serial, const std::array<RGB, 228>& canvas)
{
  auto msgs = chunker(canvas);
  for (const auto& msg : msgs)
  {
    boost::asio::write(serial, boost::asio::buffer(&msg, sizeof(msg)));
  }
}

int main(int argc, char* argv[])
{
  try {
    
    boost::asio::io_service io;
    boost::asio::serial_port serial{io, argv[1]};
    serial.set_option(boost::asio::serial_port_base::baud_rate(115200));
    
    while(1)
    {
      for (uint8_t i = 0 ; i < 228; i++)
      {
        auto canvas = empty();
        canvas[i].R = 255;
        write(serial, canvas);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }

    

  }
  catch(boost::system::system_error& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
    return 1;
  }
}
