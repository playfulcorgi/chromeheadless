[@bs.deriving abstract]
type response = {
  mutable statusCode: int
};

[@bs.send] external callVariantResponse1: response => unit = "end";
[@bs.send] external callVariantResponse2: (response, string) => unit = "end";
[@bs.send] external callVariantResponse3: (response, Buffer.t, string) => unit = "end";
[@bs.set] external setStatusCode: (response, int) => unit = "statusCode";
