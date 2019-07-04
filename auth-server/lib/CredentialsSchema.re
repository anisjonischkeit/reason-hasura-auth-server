open Graphql_lwt.Schema;

let key = Sys.getenv("HASURA_GRAPHQL_JWT_SECRET");

type jwtData = {
  userId: string,
  exp: int,
};

let makePayload = (~jwtData) =>
  Printf.sprintf(
    {|{
      "https://hasura.io/jwt/claims": {
        "x-hasura-default-role": "user",
        "x-hasura-allowed-roles": [
          "user"
        ],
        "x-hasura-user-id": "%s"
      },
      "iss": "https://hasuraauth0demo.auth0.com/",
      "sub": "google-oauth2|107653521350844232123",
      "aud": "k3WmaWwAh7Xlzqr6Ly_WePvKZtL0bnrF",
      "iat": 1555432912,
      "exp": %i,
      "at_hash": "5RE4vPNxlxBk8WFZj5WksA",
      "nonce": "52RTMYJzgFXYmCKvxK-kRVKc5yMZyTXC"
    }|},
    jwtData.userId,
    jwtData.exp,
  );

let make = name =>
  obj(name, ~fields=_ =>
    [
      field(
        "jwtToken",
        ~doc=
          "The JWT access token. Send this using the using a header like so: `Authorization: Bearer yourJWTTokenHere",
        ~typ=non_null(string),
        ~args=Arg.[],
        ~resolve=(_, jwtData) => {
          let payload = makePayload(jwtData);
          print_endline(payload);
          JWT.encode(key, payload);
        },
      ),
      field(
        "exp",
        ~doc="token expiry timestamp",
        ~typ=non_null(int),
        ~args=Arg.[],
        ~resolve=(_, jwtData) =>
        jwtData.exp
      ),
    ]
  );