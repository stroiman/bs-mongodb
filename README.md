# MongoDB bindings for bucklescript

This library contains preliminary bindings to MongoDB. There is very little
documentation yet, and far from all features are implemented.

This library is need-driven, I am adding bindings as I need them. If you add
bindings to this, feel free to send me a pull request.

## Handling Async code

The mongo driver is inherently async, every result is either provided to a
callback, or delivered in a Promise. I personally use the callbacks with [my own
async handling library](https://github.com/PeteProgrammer/resync).

But I didn't want to push a specific async library upon any users of this
library, so in order to use the library, you construct the module through a
functor, that takes a `CallbackHandler` as argument.

This piece of code is from my project where I bind the mongo library to my async
library.

```
include MongoDB;

module AsyncHandler : CallbackHandler with type t('a) = Async.t('a) = {
  type t('a) = Async.t('a);
  let callbackConverter = (x:callback('a)) : Async.t('a) => x |> Async.from_js;
};

include Make(AsyncHandler);
```

If you prefer to use a `(Js.Result.t('a,MongoError.t) => unit) => unit`, you can
do so. I might get some examples up.

And I will try to see if I can get it to work with Promises, for those who
prefer this route (the underlying node library returns a promise, if no callback
is passed to the aync functions).
