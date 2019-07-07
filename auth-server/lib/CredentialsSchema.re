open Graphql_lwt.Schema;

let key = Sys.getenv("HASURA_GRAPHQL_JWT_SECRET");

type jwtData = {
  userId: string,
  iat: int,
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
      "iss": "auth-server",
      "iat": %i,
      "exp": %i,
    }|},
    jwtData.userId,
    jwtData.iat,
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