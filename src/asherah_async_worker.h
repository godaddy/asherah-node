#ifndef ASHERAH_ASYNC_WORKER_H
#define ASHERAH_ASYNC_WORKER_H

#include <napi.h>
#include <stdexcept>

class Asherah;

template <typename ResultType>
class AsherahAsyncWorker : public Napi::AsyncWorker {
public:
  Napi::Promise Promise() { return deferred.Promise(); }

  AsherahAsyncWorker(Napi::Env env, Asherah *instance)
      : Napi::AsyncWorker(env), asherah(instance), deferred(env) {}

protected:
  Asherah *asherah;
  ResultType result;

  virtual ResultType ExecuteTask() = 0;
  virtual Napi::Value OnOKTask(Napi::Env &env) = 0;
  virtual Napi::Value OnErrorTask(Napi::Env &, Napi::Error const &error) {
    return error.Value();
  }

private:
  Napi::Promise::Deferred deferred;

  void Execute() final {
    try {
      result = ExecuteTask();
    } catch (const std::exception &ex) {
      SetError(ex.what());
    }
  }

  void OnOK() final {
    Napi::Env env = Env();
    Napi::HandleScope scope(env);
    try {
      auto value = OnOKTask(env);
      deferred.Resolve(value);
    } catch (const std::exception &e) {
      deferred.Reject(Napi::Error::New(Env(), e.what()).Value());
    }
  }

  void OnError(Napi::Error const &error) final {
    Napi::Env env = Env();
    Napi::HandleScope scope(Env());
    try {
      deferred.Reject(OnErrorTask(env, error));
    } catch (const std::exception &e) {
      deferred.Reject(Napi::Error::New(Env(), e.what()).Value());
    }
  }
};

#endif // ASHERAH_ASYNC_WORKER_H
