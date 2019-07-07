module Client = Cohttp_lwt_unix.Client;
module Response = Cohttp_lwt_unix.Response;
module Code = Cohttp.Code;
module Body = Cohttp_lwt.Body;
let (>>=) = Lwt.(>>=);
let (>|=) = Lwt.(>|=);

let hasuraGraphQLUrl = Sys.getenv("HASURA_GRAPHQL_URL");
let aYearInSeconds = 365 * 24 * 60 * 60;
let key = Sys.getenv("HASURA_GRAPHQL_JWT_SECRET");

let payload = {
  let currentTime = Unix.time() |> int_of_float;

  Printf.sprintf(
    {|{
      "https://hasura.io/jwt/claims": {
        "x-hasura-default-role": "auth-server",
        "x-hasura-allowed-roles": [
          "auth-server"
        ]
      },
      "iss": "auth-server",
      "iat": %i,
      "exp": %i
    }|},
    currentTime,
    currentTime + aYearInSeconds,
  );
};

let sendQuery = q => {
  let queryBody =
    `Assoc([("query", `String(q#query)), ("variables", q#variables)]);

  Uri.of_string(hasuraGraphQLUrl)
  |> Client.post(
       ~body=queryBody |> Yojson.to_string |> Cohttp_lwt.Body.of_string,
       ~headers=
         Cohttp.Header.(
           init()
           |> add(_, "content-type", "application/json")
           |> add(_, "Authorization", "Bearer " ++ JWT.encode(key, payload))
         ),
     )
  >>= (
    ((res, body)) => {
      body
      |> Body.to_string
      >|= (
        body => {
          let json = Yojson.Basic.from_string(body);
          switch (
            Yojson.Basic.Util.(member("data", json), member("errors", json))
          ) {
          | (`Assoc(data), _) => Ok(q#parse(`Assoc(data)))
          | (`Null, `List(data)) => Error(`List(data))
          | _ => Error(`List([]))
          };
        }
      );
    }
  );
};

module InsertUser = [%graphql
  {|
   mutation($email:String!,$first_name:String!, $last_name: String!, $password_hash: String!) {
     insert_users(objects: {email: $email, first_name: $first_name, last_name: $last_name, password_hash: $password_hash}) {
       returning {
         id
       }
     }
   }
   |}
];

module GetUser = [%graphql
  {|
    query ($email: String!) {
      users(where: {email: {_eq: $email}}) {
        id
        password_hash
      }
    }

  |}
];