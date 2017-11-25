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

type callback('a) = ((Js.Null.t(MongoError.t), 'a) => unit) => unit;

module type CallbackHandler = {
  type t('a);
  let callbackConverter : callback('a) => t('a);
};

module Make = (Handler : CallbackHandler) => {
  module Cursor = {
    type t;
    [@bs.send.pipe : t]
      external skip : int => t = "";
    [@bs.send.pipe : t]
      external limit : int => t = "limit";
    [@bs.send]
      external toArray : (t, (Js.null(MongoError.t), Js.Array.t(Js.t('a))) => unit) => unit = "";
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
      external insertOne : (t, Js.t('a), (Js.null(MongoError.t), InsertResult.t) => unit) => unit =
        "insertOne";
    [@bs.send]
      external findOne : (t, Js.t('a), (Js.null(MongoError.t), Js.null(Js.t('b))) => unit) => unit =
        "findOne";
    [@bs.send]
      external deleteOne : (t, Js.t('a), (Js.null(MongoError.t), DeleteResult.t) => unit) => unit = "";
    [@bs.send]
      external createIndex : (t, Js.t('a), Js.t('b), (Js.null(MongoError.t), string)
        => unit) => unit= "";
    [@bs.send]
      external createIndexNoOpts : (t, Js.t('a), (Js.null(MongoError.t), string)
        => unit) => unit= "createIndex";
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

  module MongoDb = Db;

  [@bs.module "mongodb"]
    external connect : (string, (Js.null(MongoError.t), Db.t) => unit) => unit =
      "connect";

  let connect = (url) => connect(url) |> Handler.callbackConverter;
};
