open Respect.Dsl.Async;

module Mongo = {
  include MongoDB;

  module AsyncHandler : CallbackHandler with type t('a) = Js.Promise.t('a) with type tresult('a)=Js.Promise.t('a) = {
    type t('a) = Js.Promise.t('a);
    type tresult('a) = Js.Promise.t('a);
    let callbackConverter = (x) : Js.Promise.t('a) => x([%bs.raw {| undefined |}]);
    };

  include Make(AsyncHandler);
};

describe("Connection with promise", [
  it("Succeeds on a good port", (_,don) => {
    Mongo.connect("mongodb://127.0.0.1:27019") |> Js.Promise.then_((_) => {
      don();
      Js.Promise.resolve(());
    }) |> ignore;
  }),
  it("Fails on a bad port", (_,don) => {
    Mongo.connect("mongodb://127.0.0.1:9999") 
      |> Js.Promise.then_((_) => Js.Promise.resolve(don(~err="We should have failed",())) )
      |> Js.Promise.catch((_) => Js.Promise.resolve(don()))
      |> ignore;
  })
]) |> register
