open Respect.Dsl.Async;
open Respect.Matcher;

let asyncResolve = actual => cb => {
  actual((x => cb(MatchSuccess(x)), x => cb(MatchFailure(x |> Obj.repr))));
};

module Mongo = {
  include MongoDB;

  module AsyncHandler : CallbackHandler with type t('a) = Async.t('a) = {
    type t('a) = Async.t('a);
    let callbackConverter = (x:callback('a)) : Async.t('a) => x |> Async.from_js;
    };

  include Make(AsyncHandler);
};

let connection = Mongo.connect("mongodb://127.0.0.1:27019/") |> Async.once;
let collection = connection |> Async.map(~f=Mongo.MongoDb.collection("foo"));

describe("Mongo", [
  it("has a failing test", (_) => {
    collection
    |> Async.bind(~f=Mongo.Collection.insertOne({"foo": "bar"}))
    |> Async.map(~f=Mongo.InsertResult.getInsertedId)
    |> Async.bind(~f=id => collection |> Async.bind(~f=Mongo.Collection.findOne({"_id": id})))
    |> Async.map(~f=doc => 
      switch(Js.Null.to_opt(doc)) {
        | None => "" 
        | Some(x) => x##foo 
      })
    |> shoulda(asyncResolve >=> equal("bar"))
  })
]) |> register
