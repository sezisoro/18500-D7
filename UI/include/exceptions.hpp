/**
 * @file    exceptions.hpp
 *
 * @brief   enum for managing exception types
 *
 * @date    2018/04/25
 * @author  LeRoy Gary <lgary@andrew.cmu.edu>
 */

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

struct Exceptions {
  static const char *VF_EMPTY;
  static const char *VF_OPEN_FAIL;
  static const char *GUI_UNKOWN_MODE;
  static const char *FRAME_EMPTY;
  static const char *PATH_ERROR;
  static const char *VF_WRITER_OPEN_FAIL;
  static const char *VF_CAPTURE_OPEN_FAIL;
  static const char *VF_UNKOWN_MODE;
};

#endif // EXCEPTIONS_HPP
