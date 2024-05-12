
#define USE_SCOPE_ALLOCATE_BUFFER 1
#include "asherah_async_worker.h"
#include "cobhan_buffer_napi.h"
#include "hints.h"
#include "libasherah.h"
#include "logging_napi.h"
#include "napi_utils.h"
#ifdef USE_SCOPED_ALLOCATE_BUFFER
#include "scoped_allocate.h"
#endif
#include <atomic>
#include <napi.h>

#ifndef NAPI_CPP_EXCEPTIONS
#error Support for C++ exceptions is required
#endif

static volatile std::atomic<int32_t> setup_state{0};

class Asherah : public Napi::Addon<Asherah> {
public:
  Asherah(Napi::Env env, Napi::Object exports) : logger(env, "asherah-node") {
    DefineAddon(
        exports,
        {
            InstanceMethod("setup", &Asherah::SetupAsherahSync),
            InstanceMethod("setup_async", &Asherah::SetupAsherahAsync),
            InstanceMethod("encrypt", &Asherah::EncryptSync),
            InstanceMethod("encrypt_async", &Asherah::EncryptAsync),
            InstanceMethod("encrypt_string", &Asherah::EncryptSync),
            InstanceMethod("encrypt_string_async", &Asherah::EncryptAsync),
            InstanceMethod("decrypt", &Asherah::DecryptSync),
            InstanceMethod("decrypt_async", &Asherah::DecryptAsync),
            InstanceMethod("decrypt_string", &Asherah::DecryptStringSync),
            InstanceMethod("decrypt_string_async",
                           &Asherah::DecryptStringAsync),
            InstanceMethod("shutdown", &Asherah::ShutdownAsherahSync),
            InstanceMethod("shutdown_async", &Asherah::ShutdownAsherahAsync),
            InstanceMethod("set_max_stack_alloc_item_size",
                           &Asherah::SetMaxStackAllocItemSize),
            InstanceMethod("set_safety_padding_overhead",
                           &Asherah::SetSafetyPaddingOverhead),
            InstanceMethod("get_setup_status", &Asherah::GetSetupStatus),
            InstanceMethod("set_log_hook", &Asherah::SetLogHook),
        });
  }

private:
  size_t est_intermediate_key_overhead = 0;
  size_t maximum_stack_alloc_size = 2048;

  int32_t verbose_flag = 0;
  Napi::FunctionReference log_hook;
  LoggerNapi logger;

#pragma region Published Node Addon Methods

  void SetupAsherahSync(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    try {
      Napi::String config_string;
      size_t product_id_length;
      size_t service_name_length;

      BeginSetupAsherah(env, __func__, info, config_string, product_id_length,
                        service_name_length);

#ifdef USE_SCOPED_ALLOCATE_BUFFER
      char *config_cbuffer;
      size_t config_cbuffer_size =
          CobhanBufferNapi::StringToAllocationSize(env, config_string);
      SCOPED_ALLOCATE_BUFFER(logger, config_cbuffer, config_cbuffer_size,
                             maximum_stack_alloc_size, __func__);

      CobhanBufferNapi config(env, config_string, config_cbuffer,
                              config_cbuffer_size);
#else
      CobhanBufferNapi config(env, config_string);
#endif

      // extern GoInt32 SetupJson(void* configJson);
      GoInt32 result = SetupJson(config);
      EndSetupAsherah(env, result, product_id_length, service_name_length);
    } catch (Napi::Error &e) {
      e.ThrowAsJavaScriptException();
      return;
    } catch (const std::exception &e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return;
    }
  }

  Napi::Value SetupAsherahAsync(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    try {
      Napi::String config_string;
      size_t product_id_length;
      size_t service_name_length;

      BeginSetupAsherah(env, __func__, info, config_string, product_id_length,
                        service_name_length);

      CobhanBufferNapi config(env, config_string);

      auto worker = new SetupAsherahWorker(env, this, config, product_id_length,
                                           service_name_length);
      worker->Queue();
      return worker->Promise();
    } catch (Napi::Error &e) {
      e.ThrowAsJavaScriptException();
      return env.Undefined();
    } catch (const std::exception &e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }

  void ShutdownAsherahSync(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    try {
      BeginShutdownAsherah(env, __func__, info);
      Shutdown();
      EndShutdownAsherah();
    } catch (Napi::Error &e) {
      e.ThrowAsJavaScriptException();
      return;
    } catch (const std::exception &e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return;
    }
  }

  Napi::Value ShutdownAsherahAsync(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    try {
      BeginShutdownAsherah(env, __func__, info);
      auto worker = new ShutdownAsherahWorker(env, this);
      worker->Queue();
      return worker->Promise();
    } catch (Napi::Error &e) {
      e.ThrowAsJavaScriptException();
      return env.Undefined();
    } catch (const std::exception &e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }

  // This is the exported sync Encrypt function
  Napi::Value EncryptSync(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::String output_string;
    try {
      Napi::String partition_id_string;
      Napi::Value input_value;

      BeginEncryptToJson(env, __func__, info, partition_id_string, input_value);

#ifdef USE_SCOPED_ALLOCATE_BUFFER
      char *partition_id_cbuffer;
      size_t partition_id_cbuffer_size =
          CobhanBufferNapi::StringToAllocationSize(env, partition_id_string);
      SCOPED_ALLOCATE_BUFFER(logger, partition_id_cbuffer,
                             partition_id_cbuffer_size,
                             maximum_stack_alloc_size, __func__);

      char *input_cbuffer;
      size_t input_cbuffer_size =
          CobhanBufferNapi::ValueToAllocationSize(env, input_value);
      SCOPED_ALLOCATE_BUFFER(logger, input_cbuffer, input_cbuffer_size,
                             maximum_stack_alloc_size, __func__);

      CobhanBufferNapi partition_id(env, partition_id_string,
                                    partition_id_cbuffer,
                                    partition_id_cbuffer_size);
      CobhanBufferNapi input(env, input_value, input_cbuffer,
                             input_cbuffer_size);
#else
      CobhanBufferNapi partition_id(env, partition_id_string);
      CobhanBufferNapi input(env, input_value);
#endif

      size_t partition_id_data_len_bytes = partition_id.get_data_len_bytes();
      size_t input_data_len_bytes = input.get_data_len_bytes();
      size_t asherah_output_size_bytes = EstimateAsherahOutputSize(
          input_data_len_bytes, partition_id_data_len_bytes);

#ifdef USE_SCOPED_ALLOCATE_BUFFER
      char *output_cobhan_buffer;
      size_t output_size_bytes =
          CobhanBuffer::DataSizeToAllocationSize(asherah_output_size_bytes);
      SCOPED_ALLOCATE_BUFFER(logger, output_cobhan_buffer, output_size_bytes,
                             maximum_stack_alloc_size, __func__);
      CobhanBufferNapi output(output_cobhan_buffer, output_size_bytes);
#else
      CobhanBufferNapi output(asherah_output_size_bytes);
#endif

      GoInt32 result = EncryptToJson(partition_id, input, output);

      EndEncryptToJson(env, output, result, output_string);
    } catch (Napi::Error &e) {
      e.ThrowAsJavaScriptException();
      return env.Undefined();
    } catch (const std::exception &e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }

    return output_string;
  }

  // This is the exported async Encrypt function
  Napi::Value EncryptAsync(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    try {
      Napi::String partition_id_string;
      Napi::Value input_value;
      BeginEncryptToJson(env, __func__, info, partition_id_string, input_value);

      CobhanBufferNapi partition_id(env, partition_id_string);
      CobhanBufferNapi input(env, input_value);

      size_t partition_id_data_len_bytes = partition_id.get_data_len_bytes();
      size_t input_data_len_bytes = input.get_data_len_bytes();
      size_t asherah_output_size_bytes = EstimateAsherahOutputSize(
          input_data_len_bytes, partition_id_data_len_bytes);

      CobhanBufferNapi output(asherah_output_size_bytes);

      auto worker =
          new EncryptAsherahWorker(env, this, partition_id, input, output);
      worker->Queue();
      return worker->Promise();
    } catch (Napi::Error &e) {
      e.ThrowAsJavaScriptException();
      return env.Undefined();
    } catch (const std::exception &e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }

  Napi::Value DecryptSync(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Object output_value;
    try {
      Napi::String partition_id_string;
      Napi::Value input_value;

      BeginDecryptFromJson(env, __func__, info, partition_id_string,
                           input_value);

#ifdef USE_SCOPED_ALLOCATE_BUFFER
      char *partition_id_cbuffer;
      size_t partition_id_cbuffer_size =
          CobhanBufferNapi::StringToAllocationSize(env, partition_id_string);
      SCOPED_ALLOCATE_BUFFER(logger, partition_id_cbuffer,
                             partition_id_cbuffer_size,
                             maximum_stack_alloc_size, __func__);

      char *input_cbuffer;
      size_t input_cbuffer_size =
          CobhanBufferNapi::ValueToAllocationSize(env, input_value);
      SCOPED_ALLOCATE_BUFFER(logger, input_cbuffer, input_cbuffer_size,
                             maximum_stack_alloc_size, __func__);

      CobhanBufferNapi partition_id(env, partition_id_string,
                                    partition_id_cbuffer,
                                    partition_id_cbuffer_size);
      CobhanBufferNapi input(env, input_value, input_cbuffer,
                             input_cbuffer_size);

      char *output_cobhan_buffer;
      size_t output_size_bytes =
          CobhanBuffer::DataSizeToAllocationSize(input.get_data_len_bytes());
      SCOPED_ALLOCATE_BUFFER(logger, output_cobhan_buffer, output_size_bytes,
                             maximum_stack_alloc_size, __func__);
      CobhanBufferNapi output(output_cobhan_buffer, output_size_bytes);
#else
      CobhanBufferNapi partition_id(env, partition_id_string);
      CobhanBufferNapi input(env, input_value);
      CobhanBufferNapi output(input.get_data_len_bytes());
#endif

      // extern GoInt32 DecryptFromJson(void* partitionIdPtr, void* jsonPtr,
      // void* dataPtr);
      GoInt32 result = DecryptFromJson(partition_id, input, output);

      CheckResult(env, result);

      output_value = output.ToBuffer(env); // NOLINT(*-slicing)
    } catch (Napi::Error &e) {
      e.ThrowAsJavaScriptException();
      return env.Undefined();
    } catch (const std::exception &e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }

    return output_value;
  }

  Napi::Value DecryptAsync(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    try {
      Napi::String partition_id_string;
      Napi::Value input_value;
      BeginDecryptFromJson(env, __func__, info, partition_id_string,
                           input_value);

      CobhanBufferNapi partition_id(env, partition_id_string);
      CobhanBufferNapi input(env, input_value);

      CobhanBufferNapi output(input.get_data_len_bytes());
      auto worker = new DecryptFromJsonWorker<Napi::Buffer<unsigned char>>(
          env, this, partition_id, input, output);
      worker->Queue();
      return worker->Promise();
    } catch (Napi::Error &e) {
      e.ThrowAsJavaScriptException();
      return env.Undefined();
    } catch (const std::exception &e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }

  Napi::Value DecryptStringSync(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::String output_string;
    try {
      NapiUtils::RequireParameterCount(info, 2);

      Napi::String partition_id_string;
      Napi::Value input_value;
      BeginDecryptFromJson(env, __func__, info, partition_id_string,
                           input_value);

#ifdef USE_SCOPED_ALLOCATE_BUFFER
      char *partition_id_cbuffer;
      size_t partition_id_cbuffer_size =
          CobhanBufferNapi::StringToAllocationSize(env, partition_id_string);
      SCOPED_ALLOCATE_BUFFER(logger, partition_id_cbuffer,
                             partition_id_cbuffer_size,
                             maximum_stack_alloc_size, __func__);

      char *input_cbuffer;
      size_t input_cbuffer_size =
          CobhanBufferNapi::ValueToAllocationSize(env, input_value);
      SCOPED_ALLOCATE_BUFFER(logger, input_cbuffer, input_cbuffer_size,
                             maximum_stack_alloc_size, __func__);

      CobhanBufferNapi partition_id(env, partition_id_string,
                                    partition_id_cbuffer,
                                    partition_id_cbuffer_size);
      CobhanBufferNapi input(env, input_value, input_cbuffer,
                             input_cbuffer_size);

      CobhanBufferNapi output(input.get_data_len_bytes());
#else
      CobhanBufferNapi partition_id(env, partition_id_string);
      CobhanBufferNapi input(env, input_value);
      CobhanBufferNapi output(input.get_data_len_bytes());
#endif

      GoInt32 result = DecryptFromJson(partition_id, input, output);

      EndDecryptFromJson(env, output, result, output_string);
    } catch (Napi::Error &e) {
      e.ThrowAsJavaScriptException();
      return env.Undefined();
    } catch (const std::exception &e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }
    return output_string;
  }

  Napi::Value DecryptStringAsync(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    try {
      NapiUtils::RequireParameterCount(info, 2);

      Napi::String partition_id_string;
      Napi::Value input_value;
      BeginDecryptFromJson(env, __func__, info, partition_id_string,
                           input_value);

      CobhanBufferNapi partition_id(env, partition_id_string);
      CobhanBufferNapi input(env, input_value);

      CobhanBufferNapi output(input.get_data_len_bytes());

      auto worker = new DecryptFromJsonWorker<Napi::String>(
          env, this, partition_id, input, output);
      worker->Queue();

      return worker->Promise();
    } catch (Napi::Error &e) {
      e.ThrowAsJavaScriptException();
      return env.Undefined();
    } catch (const std::exception &e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }

  void SetMaxStackAllocItemSize(const Napi::CallbackInfo &info) {
    NapiUtils::RequireParameterCount(info, 1);

    Napi::Number item_size = info[0].ToNumber();
    auto new_size = (size_t)item_size.Int32Value();

    maximum_stack_alloc_size = new_size;
  }

  void SetSafetyPaddingOverhead(const Napi::CallbackInfo &info) {
    NapiUtils::RequireParameterCount(info, 1);

    // Napi::Number safety_padding_number = info[0].ToNumber();
    // auto new_safety_padding_bytes = (size_t)
    // safety_padding_number.Int32Value(); Safety padding size is now fixed -
    // ignore the input set_safety_padding_bytes(new_safety_padding_bytes);
  }

  Napi::Value
  GetSetupStatus(const Napi::CallbackInfo
                     &info) { // NOLINT(*-convert-member-functions-to-static)
    int32_t setup_status = setup_state.load(std::memory_order_acquire);
    return Napi::Boolean::New(info.Env(), setup_status != 0);
  }

  void SetLogHook(const Napi::CallbackInfo &info) {
    NapiUtils::RequireParameterCount(info, 1);

    if (unlikely(!info[0].IsFunction())) {
      logger.log_error_and_throw(__func__, "Wrong argument type");
    }

    logger.set_log_hook(info[0].As<Napi::Function>());
  }

  void BeginSetupAsherah(const Napi::Env &env, const char *func_name,
                         const Napi::CallbackInfo &info,
                         Napi::String &config_string, size_t &product_id_length,
                         size_t &service_name_length) {
    RequireAsherahNotSetup(func_name);

    NapiUtils::RequireParameterCount(info, 1);

    Napi::Object config_json;
    NapiUtils::AsJsonObjectAndString(env, info[0], config_string, config_json);

    Napi::String product_id;
    NapiUtils::GetStringProperty(config_json, "ProductID", product_id);
    product_id_length = NapiUtils::GetUtf8StringLength(env, product_id);

    Napi::String service_name;
    NapiUtils::GetStringProperty(config_json, "ServiceName", service_name);
    service_name_length = NapiUtils::GetUtf8StringLength(env, service_name);

    bool verbose;
    NapiUtils::GetBooleanProperty(config_json, "Verbose", verbose, false);
    verbose_flag = verbose;
  }

  void EndSetupAsherah(const Napi::Env &env, GoInt32 result,
                       size_t product_id_length, size_t service_name_length) {
    CheckResult(env, result);

    est_intermediate_key_overhead = product_id_length + service_name_length;

    auto old_setup_state = setup_state.exchange(1, std::memory_order_acq_rel);
    if (unlikely(old_setup_state != 0)) {
      logger.log_error_and_throw(__func__, "lost race to mark setup_state!");
    }
  }

  void BeginEncryptToJson(const Napi::Env &env, const char *func_name,
                          const Napi::CallbackInfo &info,
                          Napi::String &partition_id, Napi::Value &input) {
    RequireAsherahSetup(func_name);

    NapiUtils::RequireParameterCount(info, 2);

    partition_id = NapiUtils::RequireParameterString(env, func_name, info[0]);
    input = NapiUtils::RequireParameterStringOrBuffer(env, func_name, info[1]);
  }

  void EndEncryptToJson(Napi::Env env, CobhanBufferNapi &output, GoInt32 result,
                        Napi::String &output_string) {
    CheckResult(env, result);

    output_string = output.ToString(env);
  }

  void EndEncryptToJson(Napi::Env env, CobhanBufferNapi &output, GoInt32 result,
                        Napi::Buffer<unsigned char> &output_buffer) {
    CheckResult(env, result);

    output_buffer = output.ToBuffer(env);
  }

  void BeginDecryptFromJson(const Napi::Env &env, const char *func_name,
                            const Napi::CallbackInfo &info,
                            Napi::String &partition_id, Napi::Value &input) {
    RequireAsherahSetup(func_name);

    NapiUtils::RequireParameterCount(info, 2);

    partition_id = NapiUtils::RequireParameterString(env, func_name, info[0]);
    input = NapiUtils::RequireParameterStringOrBuffer(env, func_name, info[1]);
  }

  void EndDecryptFromJson(Napi::Env &env, CobhanBufferNapi &output,
                          GoInt32 result,
                          Napi::Buffer<unsigned char> &output_buffer) {
    CheckResult(env, result);

    output_buffer = output.ToBuffer(env);
  }

  void EndDecryptFromJson(Napi::Env &env, CobhanBufferNapi &output,
                          GoInt32 result, Napi::String &output_string) {
    CheckResult(env, result);

    output_string = output.ToString(env);
  }

  void BeginShutdownAsherah(const Napi::Env &, const char *func_name,
                            const Napi::CallbackInfo &info) {
    RequireAsherahSetup(func_name);
    NapiUtils::RequireParameterCount(info, 0);
  }

  void EndShutdownAsherah() {
    auto old_setup_state = setup_state.exchange(0, std::memory_order_acq_rel);
    if (unlikely(old_setup_state == 0)) {
      logger.log_error_and_throw(__func__, "lost race to mark setup_state!");
    }
  }

#pragma endregion Begin / End Methods

#pragma region AsyncWorkers

  class SetupAsherahWorker : public AsherahAsyncWorker<GoInt32> {
  public:
    SetupAsherahWorker(Napi::Env env, Asherah *instance,
                       CobhanBufferNapi &config, size_t product_id_length,
                       size_t service_name_length)
        : AsherahAsyncWorker<GoInt32>(env, instance), config(std::move(config)),
          product_id_length(product_id_length),
          service_name_length(service_name_length) {}

    GoInt32 ExecuteTask() override { return SetupJson(config); }

    Napi::Value OnOKTask(Napi::Env &env) override {
      asherah->EndSetupAsherah(env, result, product_id_length,
                               service_name_length);
      return env.Undefined();
    }

  private:
    CobhanBufferNapi config;
    size_t product_id_length;
    size_t service_name_length;
  };

  class EncryptAsherahWorker : public AsherahAsyncWorker<GoInt32> {
  public:
    EncryptAsherahWorker(const Napi::Env &env, Asherah *instance,
                         CobhanBufferNapi &partition_id,
                         CobhanBufferNapi &input, CobhanBufferNapi &output)
        : AsherahAsyncWorker(env, instance),
          partition_id(std::move(partition_id)), input(std::move(input)),
          output(std::move(output)) {}

    // extern GoInt32 EncryptToJson(void* partitionIdPtr, void* dataPtr,
    // void* jsonPtr);
    GoInt32 ExecuteTask() override {
      return EncryptToJson(partition_id, input, output);
    }

    Napi::Value OnOKTask(Napi::Env &env) override {
      Napi::String output_string;
      asherah->EndEncryptToJson(env, output, result, output_string);
      return output_string;
    }

  private:
    CobhanBufferNapi partition_id;
    CobhanBufferNapi input;
    CobhanBufferNapi output;
  };

  template <typename T>
  class DecryptFromJsonWorker : public AsherahAsyncWorker<GoInt32> {
  public:
    DecryptFromJsonWorker(const Napi::Env &env, Asherah *instance,
                          CobhanBufferNapi &partition_id,
                          CobhanBufferNapi &input, CobhanBufferNapi &output)
        : AsherahAsyncWorker(env, instance),
          partition_id(std::move(partition_id)), input(std::move(input)),
          output(std::move(output)) {}

    GoInt32 ExecuteTask() override {
      return DecryptFromJson(partition_id, input, output);
    }

    Napi::Value OnOKTask(Napi::Env &env) override {
      T output_result;
      asherah->EndDecryptFromJson(env, output, result, output_result);
      return output_result;
    }

  protected:
    CobhanBufferNapi partition_id;
    CobhanBufferNapi input;
    CobhanBufferNapi output;
  };

  class ShutdownAsherahWorker : public AsherahAsyncWorker<GoInt32> {
  public:
    using AsherahAsyncWorker::AsherahAsyncWorker;

    // extern void Shutdown();
    GoInt32 ExecuteTask() override {
      Shutdown();
      return 0;
    }

    Napi::Value OnOKTask(Napi::Env &env) override {
      asherah->EndShutdownAsherah();
      return env.Undefined();
    }
  };

#pragma endregion AsyncWorkers

#pragma region Helpers

  void CheckResult(const Napi::Env &env, GoInt32 result) {
    if (unlikely(result < 0)) {
      NapiUtils::ThrowException(env, AsherahCobhanErrorToString(result));
    }
  }

  void RequireAsherahSetup(const char *func_name) {
    if (unlikely(setup_state.load(std::memory_order_acquire) == 0)) {
      logger.log_error_and_throw(func_name, "setup() not called");
    }
  }

  void RequireAsherahNotSetup(const char *func_name) {
    if (unlikely(setup_state.load(std::memory_order_acquire) != 0)) {
      logger.log_error_and_throw(func_name, "setup() already called");
    }
  }

  __attribute__((always_inline)) inline size_t
  EstimateAsherahOutputSize(size_t data_byte_len, size_t partition_byte_len) {
    const size_t est_encryption_overhead = 48;
    const size_t est_envelope_overhead = 185;
    const double base64_overhead = 1.34;

    // Add one rather than using std::ceil to round up
    size_t est_data_byte_len =
        size_t(double(data_byte_len + est_encryption_overhead) *
               base64_overhead) +
        1;

    return est_envelope_overhead + est_intermediate_key_overhead +
           partition_byte_len + est_data_byte_len;
  }

  __attribute__((always_inline)) static inline const char *
  AsherahCobhanErrorToString(int32_t error) {
    switch (error) {
    case 0:
      return "Success";
    case -1:
      return "Cobhan error: NULL pointer";
    case -2:
      return "Cobhan error: Buffer too large";
    case -3:
      return "Cobhan error: Buffer too small";
    case -4:
      return "Cobhan error: Copy failed";
    case -5:
      return "Cobhan error: JSON decode failed";
    case -6:
      return "Cobhan error: JSON encode failed";
    case -7:
      return "Cobhan error: Invalid UTF-8";
    case -8:
      return "Cobhan error: Read temp file failed";
    case -9:
      return "Cobhan error: Write temp file failed";
    case -100:
      return "Asherah error: Not initialized";
    case -101:
      return "Asherah error: Already initialized";
    case -102:
      return "Asherah error: Failed to get session";
    case -103:
      return "Asherah error: Encrypt operation failed";
    case -104:
      return "Asherah error: Decrypt operation failed";
    case -105:
      return "Asherah error: Invalid configuration";
    default:
      return "Unknown error";
    }
  }

#pragma endregion Helpers
};

NODE_API_NAMED_ADDON(asherah, Asherah)
