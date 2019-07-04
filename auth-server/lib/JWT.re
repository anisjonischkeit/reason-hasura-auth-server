let header = {|{"alg":"HS256","typ":"JWT"}|};

let sign = (key, data) => {
  let hash =
    Nocrypto.Hash.SHA256.hmac(
      ~key=Cstruct.of_string(key),
      Cstruct.of_string(data),
    );

  let b64Hash =
    hash
    |> Cstruct.to_string
    |> Base64.encode_string(~pad=false, ~alphabet=Base64.uri_safe_alphabet);
  data ++ "." ++ b64Hash;
};

let encode = (key, payload) => {
  let b64Header =
    Base64.encode_string(
      ~pad=false,
      ~alphabet=Base64.uri_safe_alphabet,
      header,
    );
  let b64Payload =
    Base64.encode_string(
      ~pad=false,
      ~alphabet=Base64.uri_safe_alphabet,
      payload,
    );
  let unsignedJWT = b64Header ++ "." ++ b64Payload;
  sign(key, unsignedJWT);
};

let verify = (key, jwt) =>
  switch (jwt |> Str.split(Str.regexp("\."))) {
  | [header, data, signature] =>
    jwt |> String.equal(sign(key, header ++ "." ++ data)) |> (r => Ok(r))
  | _ => Error("invalid jwt")
  };