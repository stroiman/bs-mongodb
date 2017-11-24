open Respect.Dsl.Async;

module Mongo = {
  include MongoDB;

  type resultCallback('a) = Js.Result.t('a,MongoDB.MongoError.t) => unit;
  module AsyncHandler : CallbackHandler with type t('a) = (resultCallback('a) => unit) with type tresult('a)=unit = {
    type t('a) = resultCallback('a) => unit;
    type tresult('a)=unit;
    let callbackConverter = (x:callback('a,unit)) : t('a) => cb => {
      x((err, result) => switch(Js.Null.to_opt(err)) {
        | None => cb(Ok(result))
        | Some(x) => cb(Error(x))
      })
    };
  };

  include Make(AsyncHandler);
};

module Ctx = TestContext;

describe("Mongo using result callbacks", [
  describe("creating a valid connection", [
    it("succeeds", (_,don) => {
      Mongo.connect("mongodb://127.0.0.1:27019", res => {
        switch(res) {
          | Js.Result.Ok(_) => don()
          | Js.Result.Error(_) => don(~err="No error was expected",())
          }
      })
    })
  ]),

  describe("creating an invalid connection", [
    it("succeeds", (_,don) => {
      Mongo.connect("mongodb://127.0.0.1:9999", res => {
        switch(res) {
          | Js.Result.Error(_) => don()
          | Js.Result.Ok(_) => don(~err="This was expected to fail",())
          }
      })
    })
  ])
]) |> register
