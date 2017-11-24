module MongoError = {
  type t = Js.Exn.t;
  [@bs.get] external getCode : t => int = "code";
};

module ObjectID = {
  type t;
  [@bs.send.pipe : t] external toHexString : string = "";
  [@bs.new] [@bs.module "mongodb"] external from_string : string => t = "ObjectID";
  [@bs.new] [@bs.module "mongodb"] external make : t = "ObjectID";
};

type mongoCallback('a) = ((Js.Null.t(MongoError.t), 'a) => unit);
type mongoResult('a,'res) = mongoCallback('a) => 'res;

module type CallbackHandler = {
  type t('a);
  type tresult('a);
  let callbackConverter : mongoResult('a,tresult('a)) => t('a);
};

module Make = (Handler : CallbackHandler) => {
  type callback('a) = mongoCallback('a);
  type res('a) = Handler.tresult('a);
  module Cursor = {
    type t;
    [@bs.send.pipe : t]
      external skip : int => t = "";
    [@bs.send.pipe : t]
      external limit : int => t = "limit";
    [@bs.send]
      external toArray : (t, callback(Js.Array.t(Js.t('a)))) => res(Js.Array.t(Js.t('a))) = "";
    let toArray = cursor => toArray(cursor) |> Handler.callbackConverter;
    };

  module InsertResult = {
    type t;
    [@bs.get] external getInsertedCount : t => int = "insertedCount";
    /** Gets the id of the object inserted into mongo.
      It could be something else than an ObjectID, but that is only if we
      specified an _id value before inserting. And then we really didn't
      need to ask mongo what the value is. So if we need to ask, it is
      probably an ObjectId */
    [@bs.get] external getInsertedId : t => ObjectID.t = "insertedId";
  };

  module DeleteResult = {
    type t;
    [@bs.get] external getDeletedCount : t => int = "deletedCount";
  };

  module Collection = {
    type t;

    [@bs.send]
      external insertOne : (t, Js.t('a), callback(InsertResult.t)) => res(InsertResult.t) =
        "insertOne";
    [@bs.send]
      external findOne : (t, Js.t('a), callback(Js.null(Js.t('b)))) => res(Js.null(Js.t('b))) =
        "findOne";
    [@bs.send]
      external deleteOne : (t, Js.t('a), callback(DeleteResult.t)) => res(DeleteResult.t) = "";
    [@bs.send]
      external createIndex : (t, Js.t('a), Js.t('b), callback(string)) => res(string) = "";
    [@bs.send]
      external createIndexNoOpts : (t, Js.t('a), callback(string)) =>
        res(string) = "createIndex";
    [@bs.send.pipe : t]
      external find : Js.t('a) => Cursor.t = "";
    let insertOne = (doc, col) => insertOne(col, doc) |> Handler.callbackConverter;
    let findOne = (doc, col) => findOne(col, doc) |> Handler.callbackConverter;
    let deleteOne = (doc, col) => deleteOne(col, doc) |> Handler.callbackConverter;
    let createIndex = (index, options, col) =>
      createIndex(col, index, options) |> Handler.callbackConverter;
    let createIndexNoOpts = (index, col) =>
      createIndexNoOpts(col, index) |> Handler.callbackConverter;
  };

  module Db = {
    type t;
    [@bs.send] external collection : (t, string) => Collection.t = "collection";
    let collection = (name, conn) => collection(conn, name);
  };

  [@bs.module "mongodb"]
    external connect : (string, callback(Db.t)) => res(Db.t) =
      "connect";

  let connect = (url) => connect(url) |> Handler.callbackConverter;
};
