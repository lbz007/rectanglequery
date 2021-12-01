/* @file  io_manager.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef SRC_UTIL_IO_MANAGER_H_
#define SRC_UTIL_IO_MANAGER_H_

#include <stdint.h>
#include <zlib.h>
#include <zip.h>
#include <lz4.h>
#include <lz4frame.h>
#include <zstd.h>

#include <string>
#include <vector>

#include "util/map_reduce.h"
#include "util/message.h"

namespace open_edi {
namespace util {

const uint32_t kDefaultSize = 4 * (1 <<20);  // 4 MiB
const uint8_t  kEOF = 0;
const uint8_t  kReadFail = -1;
const uint8_t  kReadSuccess = 0;
const uint8_t  kWriteFail = -1;
const uint8_t  kWriteSuccess = 0;
const uint32_t kMagicNumberSize = 4;
const uint32_t kLz4MagicNumber  = 0x184D2204;
const uint32_t kZstdMagicNumber = 0xFD2FB528;
const uint8_t  kZstdFrameHeaderSizeMax = 18;

class IOBuffer;
class CompressBlock;
class Lz4;
class Zstd;

enum CompressType {
    kCompressNull,
    kCompressLz4,
    kCompressGz,
    kCompressZip,
    kCompressZstd,
};

enum ModeType {
    kRead,
    kWrite,
};

enum CompressLevel {
    kCompressLevelInvalid,
    kCompressLevelMin = 1,  // default of LZ4, ZSTD
    kCompressLevel2,
    kCompressLevel3,
    kCompressLevel4,
    kCompressLevel5,
    kCompressLevel6,  // default of GZIP
    kCompressLevel7,
    kCompressLevel8,
    kCompressLevel9,
    kCompressLevel10,
    kCompressLevel11,
    kCompressLevel12,
    kCompressLevel13,
    kCompressLevel14,
    kCompressLevel15,
    kCompressLevelMax,
};

/// @brief Provide APIs to input or output data in paralled.
class IOManager {
  public:
    IOManager();
    ~IOManager();
    bool open(const char *file_name, const char *mode);
    bool open(const char *file_name, const char *mode, CompressLevel level);

    int read(void *buffer, uint32_t size);
    // The buffer will be reused when read(uint32_t size) is called next time.
    IOBuffer *read(uint32_t size = kDefaultSize);
    // The buffers should be freed by applications.
    CompressBlock *readCompressBlock();
    // comprees_block is allocated by caller
    bool readCompressBlock(CompressType compress_type,
                           std::vector<void*> &buffers,
                           std::vector<uint32_t> &sizes);

    int write(IOBuffer *buffer);
    int write(uint32_t size, void *buffer);
    int write(std::string);
    int write(const char *format, ...);
    bool writeCompressBlock(CompressType compress_type,
                            std::vector<void*> &buffers,
                            std::vector<uint32_t> &sizes);

    int seek(int64_t offset, int origin);
    void flush();
    int64_t tell();
    void close();

  private:
    CompressType compress_type_;
    CompressLevel compress_level_;

    FILE            *fp_;
    gzFile           gzfp_;
    Lz4             *lz4fp_;
    Zstd            *zstdfp_;
    struct zip      *zip_archive_;
    struct zip_file *zip_file_;
    IOBuffer        *read_io_buffer_;   // for read(uint32_t size)
    IOBuffer        *write_io_buffer_;  // for write with variable arguments
};

/// @brief Buffers to restore data with size.
class IOBuffer {
  public:
    IOBuffer() {
        buffer_ = nullptr;
        is_new_buffer_ = false;
    }
    explicit IOBuffer(int32_t size) {
        size_ = size;
        buffer_ = new char[size]();
        is_new_buffer_ = true;
    }
    ~IOBuffer() {
        if (is_new_buffer_) {
            delete[] buffer_;
        }
        buffer_ = nullptr;
        is_new_buffer_ = false;
    }
    void setBuffer(char *buffer) { buffer_ = buffer; }
    char *getBuffer() {return buffer_;}
    void setSize(int32_t size) { size_ = size; }
    int32_t getSize() {return size_;}

  private:
    bool is_new_buffer_;
    int32_t size_;  // the length of valid characters in buffer.
    char *buffer_;
};

/// @brief Blocks from compress and decompress.
class CompressBlock {
  public:
    CompressBlock(std::vector<void*> &buffers, std::vector<uint32_t> &sizes);
    ~CompressBlock() {
        for (auto io_buffer : io_buffers_) {
            delete io_buffer;
        }
    }
    void setTotalNumber(uint32_t total_number) { total_number_ = total_number; }
    uint32_t getTotalNumber() { return total_number_; }

    void setMaxBufferSize(uint32_t max_buffer_size) {
        max_buffer_size_ = max_buffer_size;
    }
    uint32_t getMaxBufferSize() { return max_buffer_size_; }

    void setIOBuffers(std::vector<IOBuffer*> &io_buffers) {
        io_buffers_ = io_buffers;
    }
    std::vector<IOBuffer*> &getIOBuffers() { return io_buffers_; }

  private:
    uint32_t total_number_;
    uint32_t max_buffer_size_;
    std::vector<IOBuffer*> io_buffers_;
};

/// @brief Compress or decompress blocks in parallel
class CompressManager {
  public:
    CompressManager(CompressType compress_type, IOManager *io_manager);
    bool compress(CompressBlock &compress_block);
    CompressBlock *decompress();
    bool decompress(CompressBlock &compress_block);

    class WriteInformation {
      public:
        int buffer_number_;
        std::vector<IOBuffer*> *buffers_;
        IOManager *io_manager_;
    };

  private:
    void freeIOBuffers(std::vector<IOBuffer*> &io_buffers);
    static void *write(void*);

    CompressType compress_type_;
    IOManager *io_manager_;
    std::vector<IOBuffer*> src_buffers_;
    std::vector<IOBuffer*> dst_buffers_;

    WriteInformation write_information_;
};

/// @brief Compress input and output.
class CompressInput : public MTAppInput {
  public:
    CompressInput() {
        src_buffers_ = nullptr;
        dst_buffers_ = nullptr;
    }
    CompressInput(std::vector<IOBuffer*> *src_buffers,
                  std::vector<IOBuffer*> *dst_buffers) {
        src_buffers_ = src_buffers;
        dst_buffers_ = dst_buffers;
    }
    void setSrcBuffers(std::vector<IOBuffer*> *src_buffers) {
        src_buffers_ = src_buffers;
    }
    std::vector<IOBuffer*> *getSrcBuffers() {
        return src_buffers_;
    }
    void setDstBuffers(std::vector<IOBuffer*> *dst_buffers) {
        dst_buffers_ = dst_buffers;
    }
    std::vector<IOBuffer*> *getDstBuffers() {
        return dst_buffers_;
    }

  private:
    std::vector<IOBuffer*> *src_buffers_;  // input
    std::vector<IOBuffer*> *dst_buffers_;  // output
};

/// @brief Compress task for map reducer
class CompressTask : public MTTask {
  public:
    CompressTask(IOBuffer *src_buffer, IOBuffer *dst_buffer) {
        ediAssert(src_buffer != nullptr);
        ediAssert(dst_buffer != nullptr);
        src_buffer_ = src_buffer;
        dst_buffer_ = dst_buffer;
    }
    void setSrcBuffer(IOBuffer *src_buffer) {
        src_buffer_ = src_buffer;
    }
    void setDstBuffer(IOBuffer *dst_buffer) {
        dst_buffer_ = dst_buffer;
    }
    IOBuffer *getSrcBuffer() {
        return src_buffer_;
    }
    IOBuffer *getDstBuffer() {
        return dst_buffer_;
    }
    void setSize(int32_t size) {
        dst_buffer_->setSize(size);
    }
    int32_t getCompressedSize() {
        return dst_buffer_->getSize();
    }

  private:
    IOBuffer *src_buffer_;
    IOBuffer *dst_buffer_;
};  // class CompressTask

/// @brief Compress blocks with map reducer.
class Compressor : public MTMRApp {
  public:
    ~Compressor();

    void setCompressType(CompressType compress_type) {
        compress_type_ = compress_type;
    }
    CompressType getCompressType() {
        return compress_type_;
    }
    void setInput(CompressInput *input) { input_ = input; }

    virtual void preRun();
    virtual void postRun();
  private:
    CompressType  compress_type_;
    virtual void* runMapper();
    virtual void* runWorker();
    virtual void* runReducer();

    CompressInput *input_;
};

/// @brief Decompress blocks with map reducer
class Decompressor : public MTMRApp {
  public:
    Decompressor() {}
    Decompressor(CompressType decompress_type,
                 void *src_buffer, uint32_t src_sizes);
    Decompressor(CompressType decompress_type,
                 std::vector<void *> *src_buffers,
                 std::vector<uint32_t *> *src_siezes);
    ~Decompressor() {}

    void getResult(void **dst_buffer, uint32_t dst_size);
    void getResult(std::vector<void *> &dst_buffer,
                   std::vector<uint32_t> &dst_sizes);

    void setDecompressType(CompressType decompress_type) {
        decompress_type_ = decompress_type;
    }
    CompressType getDecompressType() {
        return decompress_type_;
    }
    void setInput(CompressInput *input) { input_ = input; }
    virtual void preRun() {}
    virtual void postRun() {}

  private:
    CompressType  decompress_type_;
    virtual void* runMapper();
    virtual void* runWorker();
    virtual void* runReducer();

    CompressInput *input_;
};

/// @brief Read and write data from / to Lz4 file.
class Lz4 {
  public:
    Lz4();
    Lz4(const char *file_name, const char *mode, CompressLevel compress_level);
    ~Lz4();

    bool open(char *file_name, char * mode);
    int read(void *buffer, int size);
    int write(void *buffer, int size);
    int seek(int64_t offset, int origin);
    int flush();
    int64_t tell();
    void close();
    bool isOpen() {return (nullptr != fp_);}

  private:
    // functions
    int read_header();
    int write_header();
    uint32_t readLE32(const void* src);  // read little endian
    void writeLE32(void* dst, uint32_t value32);  // write little endian

    // data members
    FILE *fp_;
    ModeType mode_type_;
    LZ4F_decompressionContext_t decom_context_;
    LZ4F_compressionContext_t com_context_;
    LZ4F_preferences_t prefs_;
    int next_to_load_;
    int src_filled_size_;
    int src_done_size_;
    int dst_filled_size_;
    int dst_done_size_;  // for read
    int src_buffer_size_;
    int dst_buffer_size_;
    void *src_buffer_;
    void *dst_buffer_;
};  // class Lz4

/// @brief Read and write data from / to zstd file.
class Zstd {
  public:
    Zstd();
    Zstd(const char *file_name, const char *mode, CompressLevel compress_level);
    ~Zstd();

    bool open(char *file_name, char * mode);
    int read(void *buffer, int size);
    int write(void *buffer, int size);
    int seek(int64_t offset, int origin);
    int flush();
    int64_t tell();
    void close();
    bool isOpen() {return (nullptr != fp_);}

  private:
    // functions
    int read_header();
    uint32_t readLE32(const void* src);  // read little endian
    void writeLE32(void* dst, uint32_t value32);  // write little endian

    // data members
    FILE *fp_;
    ModeType mode_type_;
    ZSTD_DCtx* decom_context_;
    ZSTD_CCtx* com_context_;
    int next_to_load_;
    uint32_t src_filled_size_;
    uint32_t src_done_size_;  // already readed,written,compressed,decompressed
    uint32_t dst_filled_size_;
    uint32_t dst_done_size_;  // for read
    uint32_t src_buffer_size_;
    uint32_t dst_buffer_size_;
    void *src_buffer_;
    void *dst_buffer_;
};  // class Zstd

}  // namespace util
}  // namespace open_edi

#endif  // SRC_UTIL_IO_MANAGER_H_
