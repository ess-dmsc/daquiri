#pragma once

#include <iostream>
#include <streambuf>
#include <string>

#include <QObject>

class LogEmitter : public QObject
{
 Q_OBJECT
 public:
  void write(std::string someline)
  {
    emit(writeLine(QString::fromStdString(someline)));
  }

 signals:
  void writeLine(QString someline);
};

class LogStreamBuffer : public std::basic_streambuf<char>
{
 public:
  LogStreamBuffer(std::ostream& stream, LogEmitter& emitter)
      : out_stream_(stream)
        , log_emitter_(emitter)
  {
    old_buffer_ = out_stream_.rdbuf();
    out_stream_.rdbuf(this);
  }

  ~LogStreamBuffer() override
  {
    // output anything that remains
    if (!buffer_string_.empty())
      log_emitter_.write(buffer_string_);
    out_stream_.rdbuf(old_buffer_);
  }

 protected:
  int_type overflow(int_type v) override
  {
    if (v == '\n')
    {
      log_emitter_.write(buffer_string_);
      buffer_string_.erase(buffer_string_.begin(), buffer_string_.end());
    }
    else
      buffer_string_ += v;
    return v;
  }

  std::streamsize xsputn(const char* p, std::streamsize n) override
  {
    buffer_string_.append(p, p + n);
    std::size_t pos;
    while ((pos = buffer_string_.find_last_of('\n')) != std::string::npos)
    {
      log_emitter_.write(buffer_string_.substr(0, pos));
      buffer_string_.erase(buffer_string_.begin(), buffer_string_.begin() + pos + 1);
    }
    return n;
  }

 private:
  std::ostream& out_stream_;
  std::streambuf* old_buffer_;
  std::string buffer_string_;
  LogEmitter& log_emitter_;
};
