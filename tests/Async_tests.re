open Respect.Dsl.Async;
open Respect.Matcher;

module Ctx = TestContext;

let asyncResolve = actual => cb => {
  actual((x => cb(MatchSuccess(x)), x => cb(MatchFailure(x |> Obj.repr))));
};

let (>>=) = (x,f) => Async.bind(~f, x);
let (>>|) = (x,f) => Async.map(~f, x);

module Mongo = {
  include MongoDB;

  module AsyncHandler : CallbackHandler with type t('a) = Async.t('a) = {
    type t('a) = Async.t('a);
    let callbackConverter = (x:callback('a)) : Async.t('a) => x |> Async.from_js;
    };

  include Make(AsyncHandler);
};

let connection = Mongo.connect("mongodb://127.0.0.1:27019/") |> Async.once;
let collection = connection >>| Mongo.Db.collection("foo");

describe("Mongo", [
  describe("An object has already been inserted", [
    beforeEach((ctx,don) => 
      (collection
       >>= coll => coll |> Mongo.Collection.insertOne({"foo": "bar"})
       >>| Mongo.InsertResult.getInsertedId)
       |> Async.run(id => {
         ctx |> Ctx.add("id", id);
         don();
       })
    ),

    it("is retrievable", (ctx) => {
      let id: MongoDB.ObjectID.t = ctx |> Ctx.get("id");
      (collection
       >>= Mongo.Collection.findOne({"_id": id})
       >>| Js.Null.to_opt 
       >>| fun | None => "" | Some(x) => x##foo)
       |> shoulda(asyncResolve >=> equal("bar"))
     })
  ])
]) |> register
