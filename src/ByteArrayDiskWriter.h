/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_BYTE_ARRAY_DISK_WRITER_H_
#define _D_BYTE_ARRAY_DISK_WRITER_H_

#include "DiskWriter.h"

class ByteArrayDiskWriter : public DiskWriter {
private:
  char* buf;
  int maxBufLength;
  int bufLength;

  void init();
  void clear();
public:
  ByteArrayDiskWriter();
  virtual ~ByteArrayDiskWriter();

  virtual void initAndOpenFile(string filename);

  virtual void openFile(const string& filename);

  virtual void closeFile();

  virtual void openExistingFile(string filename);

  // position is ignored
  virtual void writeData(const char* data, int len, long long int position = 0);
  virtual int readData(char* data, int len, long long int position);
  // not implemented yet
  virtual string sha1Sum(long long int offset, long long int length) { return ""; }

  const char* getByteArray() const {
    return buf;
  }
  int getByteArrayLength() const {
    return bufLength;
  }
  void reset();
};

#endif // _D_BYTE_ARRAY_DISK_WRITER_H_
