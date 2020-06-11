// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: service.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_service_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_service_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3011000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3011004 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/service.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_service_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_service_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[2]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_service_2eproto;
namespace sdkproxy {
class HttpRequest;
class HttpRequestDefaultTypeInternal;
extern HttpRequestDefaultTypeInternal _HttpRequest_default_instance_;
class HttpResponse;
class HttpResponseDefaultTypeInternal;
extern HttpResponseDefaultTypeInternal _HttpResponse_default_instance_;
}  // namespace sdkproxy
PROTOBUF_NAMESPACE_OPEN
template<> ::sdkproxy::HttpRequest* Arena::CreateMaybeMessage<::sdkproxy::HttpRequest>(Arena*);
template<> ::sdkproxy::HttpResponse* Arena::CreateMaybeMessage<::sdkproxy::HttpResponse>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace sdkproxy {

// ===================================================================

class HttpRequest :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:sdkproxy.HttpRequest) */ {
 public:
  HttpRequest();
  virtual ~HttpRequest();

  HttpRequest(const HttpRequest& from);
  HttpRequest(HttpRequest&& from) noexcept
    : HttpRequest() {
    *this = ::std::move(from);
  }

  inline HttpRequest& operator=(const HttpRequest& from) {
    CopyFrom(from);
    return *this;
  }
  inline HttpRequest& operator=(HttpRequest&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const HttpRequest& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const HttpRequest* internal_default_instance() {
    return reinterpret_cast<const HttpRequest*>(
               &_HttpRequest_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(HttpRequest& a, HttpRequest& b) {
    a.Swap(&b);
  }
  inline void Swap(HttpRequest* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline HttpRequest* New() const final {
    return CreateMaybeMessage<HttpRequest>(nullptr);
  }

  HttpRequest* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<HttpRequest>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const HttpRequest& from);
  void MergeFrom(const HttpRequest& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(HttpRequest* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "sdkproxy.HttpRequest";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_service_2eproto);
    return ::descriptor_table_service_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:sdkproxy.HttpRequest)
 private:
  class _Internal;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_service_2eproto;
};
// -------------------------------------------------------------------

class HttpResponse :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:sdkproxy.HttpResponse) */ {
 public:
  HttpResponse();
  virtual ~HttpResponse();

  HttpResponse(const HttpResponse& from);
  HttpResponse(HttpResponse&& from) noexcept
    : HttpResponse() {
    *this = ::std::move(from);
  }

  inline HttpResponse& operator=(const HttpResponse& from) {
    CopyFrom(from);
    return *this;
  }
  inline HttpResponse& operator=(HttpResponse&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const HttpResponse& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const HttpResponse* internal_default_instance() {
    return reinterpret_cast<const HttpResponse*>(
               &_HttpResponse_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(HttpResponse& a, HttpResponse& b) {
    a.Swap(&b);
  }
  inline void Swap(HttpResponse* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline HttpResponse* New() const final {
    return CreateMaybeMessage<HttpResponse>(nullptr);
  }

  HttpResponse* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<HttpResponse>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const HttpResponse& from);
  void MergeFrom(const HttpResponse& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(HttpResponse* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "sdkproxy.HttpResponse";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_service_2eproto);
    return ::descriptor_table_service_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:sdkproxy.HttpResponse)
 private:
  class _Internal;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_service_2eproto;
};
// ===================================================================

class DeviceQueryService_Stub;

class DeviceQueryService : public ::PROTOBUF_NAMESPACE_ID::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline DeviceQueryService() {};
 public:
  virtual ~DeviceQueryService();

  typedef DeviceQueryService_Stub Stub;

  static const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* descriptor();

  virtual void Discovery(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void Query(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method,
                  ::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                  const ::PROTOBUF_NAMESPACE_ID::Message* request,
                  ::PROTOBUF_NAMESPACE_ID::Message* response,
                  ::google::protobuf::Closure* done);
  const ::PROTOBUF_NAMESPACE_ID::Message& GetRequestPrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;
  const ::PROTOBUF_NAMESPACE_ID::Message& GetResponsePrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(DeviceQueryService);
};

class DeviceQueryService_Stub : public DeviceQueryService {
 public:
  DeviceQueryService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
  DeviceQueryService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel,
                   ::PROTOBUF_NAMESPACE_ID::Service::ChannelOwnership ownership);
  ~DeviceQueryService_Stub();

  inline ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel() { return channel_; }

  // implements DeviceQueryService ------------------------------------------

  void Discovery(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  void Query(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(DeviceQueryService_Stub);
};


// -------------------------------------------------------------------

class RealStreamService_Stub;

class RealStreamService : public ::PROTOBUF_NAMESPACE_ID::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline RealStreamService() {};
 public:
  virtual ~RealStreamService();

  typedef RealStreamService_Stub Stub;

  static const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* descriptor();

  virtual void Start(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method,
                  ::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                  const ::PROTOBUF_NAMESPACE_ID::Message* request,
                  ::PROTOBUF_NAMESPACE_ID::Message* response,
                  ::google::protobuf::Closure* done);
  const ::PROTOBUF_NAMESPACE_ID::Message& GetRequestPrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;
  const ::PROTOBUF_NAMESPACE_ID::Message& GetResponsePrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(RealStreamService);
};

class RealStreamService_Stub : public RealStreamService {
 public:
  RealStreamService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
  RealStreamService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel,
                   ::PROTOBUF_NAMESPACE_ID::Service::ChannelOwnership ownership);
  ~RealStreamService_Stub();

  inline ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel() { return channel_; }

  // implements RealStreamService ------------------------------------------

  void Start(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(RealStreamService_Stub);
};


// -------------------------------------------------------------------

class VodService_Stub;

class VodService : public ::PROTOBUF_NAMESPACE_ID::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline VodService() {};
 public:
  virtual ~VodService();

  typedef VodService_Stub Stub;

  static const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* descriptor();

  virtual void Query(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void DownloadByTime(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method,
                  ::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                  const ::PROTOBUF_NAMESPACE_ID::Message* request,
                  ::PROTOBUF_NAMESPACE_ID::Message* response,
                  ::google::protobuf::Closure* done);
  const ::PROTOBUF_NAMESPACE_ID::Message& GetRequestPrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;
  const ::PROTOBUF_NAMESPACE_ID::Message& GetResponsePrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(VodService);
};

class VodService_Stub : public VodService {
 public:
  VodService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
  VodService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel,
                   ::PROTOBUF_NAMESPACE_ID::Service::ChannelOwnership ownership);
  ~VodService_Stub();

  inline ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel() { return channel_; }

  // implements VodService ------------------------------------------

  void Query(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  void DownloadByTime(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(VodService_Stub);
};


// -------------------------------------------------------------------

class EventAnalyzeService_Stub;

class EventAnalyzeService : public ::PROTOBUF_NAMESPACE_ID::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline EventAnalyzeService() {};
 public:
  virtual ~EventAnalyzeService();

  typedef EventAnalyzeService_Stub Stub;

  static const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* descriptor();

  virtual void Reset(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void Query(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void Start(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void Stop(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method,
                  ::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                  const ::PROTOBUF_NAMESPACE_ID::Message* request,
                  ::PROTOBUF_NAMESPACE_ID::Message* response,
                  ::google::protobuf::Closure* done);
  const ::PROTOBUF_NAMESPACE_ID::Message& GetRequestPrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;
  const ::PROTOBUF_NAMESPACE_ID::Message& GetResponsePrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(EventAnalyzeService);
};

class EventAnalyzeService_Stub : public EventAnalyzeService {
 public:
  EventAnalyzeService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
  EventAnalyzeService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel,
                   ::PROTOBUF_NAMESPACE_ID::Service::ChannelOwnership ownership);
  ~EventAnalyzeService_Stub();

  inline ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel() { return channel_; }

  // implements EventAnalyzeService ------------------------------------------

  void Reset(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  void Query(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  void Start(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  void Stop(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(EventAnalyzeService_Stub);
};


// -------------------------------------------------------------------

class HealthService_Stub;

class HealthService : public ::PROTOBUF_NAMESPACE_ID::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline HealthService() {};
 public:
  virtual ~HealthService();

  typedef HealthService_Stub Stub;

  static const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* descriptor();

  virtual void Health(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method,
                  ::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                  const ::PROTOBUF_NAMESPACE_ID::Message* request,
                  ::PROTOBUF_NAMESPACE_ID::Message* response,
                  ::google::protobuf::Closure* done);
  const ::PROTOBUF_NAMESPACE_ID::Message& GetRequestPrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;
  const ::PROTOBUF_NAMESPACE_ID::Message& GetResponsePrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(HealthService);
};

class HealthService_Stub : public HealthService {
 public:
  HealthService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
  HealthService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel,
                   ::PROTOBUF_NAMESPACE_ID::Service::ChannelOwnership ownership);
  ~HealthService_Stub();

  inline ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel() { return channel_; }

  // implements HealthService ------------------------------------------

  void Health(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(HealthService_Stub);
};


// -------------------------------------------------------------------

class ConfigService_Stub;

class ConfigService : public ::PROTOBUF_NAMESPACE_ID::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline ConfigService() {};
 public:
  virtual ~ConfigService();

  typedef ConfigService_Stub Stub;

  static const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* descriptor();

  virtual void GetFtp(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void SetFtp(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method,
                  ::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                  const ::PROTOBUF_NAMESPACE_ID::Message* request,
                  ::PROTOBUF_NAMESPACE_ID::Message* response,
                  ::google::protobuf::Closure* done);
  const ::PROTOBUF_NAMESPACE_ID::Message& GetRequestPrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;
  const ::PROTOBUF_NAMESPACE_ID::Message& GetResponsePrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ConfigService);
};

class ConfigService_Stub : public ConfigService {
 public:
  ConfigService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
  ConfigService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel,
                   ::PROTOBUF_NAMESPACE_ID::Service::ChannelOwnership ownership);
  ~ConfigService_Stub();

  inline ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel() { return channel_; }

  // implements ConfigService ------------------------------------------

  void GetFtp(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
  void SetFtp(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ConfigService_Stub);
};


// -------------------------------------------------------------------

class VisitorsFlowRateService_Stub;

class VisitorsFlowRateService : public ::PROTOBUF_NAMESPACE_ID::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline VisitorsFlowRateService() {};
 public:
  virtual ~VisitorsFlowRateService();

  typedef VisitorsFlowRateService_Stub Stub;

  static const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* descriptor();

  virtual void QueryHistory(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method,
                  ::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                  const ::PROTOBUF_NAMESPACE_ID::Message* request,
                  ::PROTOBUF_NAMESPACE_ID::Message* response,
                  ::google::protobuf::Closure* done);
  const ::PROTOBUF_NAMESPACE_ID::Message& GetRequestPrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;
  const ::PROTOBUF_NAMESPACE_ID::Message& GetResponsePrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(VisitorsFlowRateService);
};

class VisitorsFlowRateService_Stub : public VisitorsFlowRateService {
 public:
  VisitorsFlowRateService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
  VisitorsFlowRateService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel,
                   ::PROTOBUF_NAMESPACE_ID::Service::ChannelOwnership ownership);
  ~VisitorsFlowRateService_Stub();

  inline ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel() { return channel_; }

  // implements VisitorsFlowRateService ------------------------------------------

  void QueryHistory(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::sdkproxy::HttpRequest* request,
                       ::sdkproxy::HttpResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(VisitorsFlowRateService_Stub);
};


// -------------------------------------------------------------------

class EntranceGuardService_Stub;

class EntranceGuardService : public ::PROTOBUF_NAMESPACE_ID::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline EntranceGuardService() {};
 public:
  virtual ~EntranceGuardService();

  typedef EntranceGuardService_Stub Stub;

  static const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* descriptor();


  // implements Service ----------------------------------------------

  const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method,
                  ::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                  const ::PROTOBUF_NAMESPACE_ID::Message* request,
                  ::PROTOBUF_NAMESPACE_ID::Message* response,
                  ::google::protobuf::Closure* done);
  const ::PROTOBUF_NAMESPACE_ID::Message& GetRequestPrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;
  const ::PROTOBUF_NAMESPACE_ID::Message& GetResponsePrototype(
    const ::PROTOBUF_NAMESPACE_ID::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(EntranceGuardService);
};

class EntranceGuardService_Stub : public EntranceGuardService {
 public:
  EntranceGuardService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
  EntranceGuardService_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel,
                   ::PROTOBUF_NAMESPACE_ID::Service::ChannelOwnership ownership);
  ~EntranceGuardService_Stub();

  inline ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel() { return channel_; }

  // implements EntranceGuardService ------------------------------------------

 private:
  ::PROTOBUF_NAMESPACE_ID::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(EntranceGuardService_Stub);
};


// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// HttpRequest

// -------------------------------------------------------------------

// HttpResponse

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace sdkproxy

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_service_2eproto
