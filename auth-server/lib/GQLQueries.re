module Client = Cohttp_lwt_unix.Client;
module Response = Cohttp_lwt_unix.Response;
module Code = Cohttp.Code;
module Body = Cohttp_lwt.Body;
let (>>=) = Lwt.(>>=);
let (>|=) = Lwt.(>|=);

let hasuraGraphQLUrl = Sys.getenv("HASURA_GRAPHQL_URL");

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
           |> add(
                _,
                "Authorization",
                "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJodHRwczovL2hhc3VyYS5pby9qd3QvY2xhaW1zIjp7IngtaGFzdXJhLWRlZmF1bHQtcm9sZSI6ImF1dGgtc2VydmVyIiwieC1oYXN1cmEtYWxsb3dlZC1yb2xlcyI6WyJhdXRoLXNlcnZlciJdfSwiaXNzIjoiaHR0cHM6Ly9oYXN1cmFhdXRoMGRlbW8uYXV0aDAuY29tLyIsInN1YiI6Imdvb2dsZS1vYXV0aDJ8MTA3NjUzNTIxMzUwODQ0MjMyMTIzIiwiYXVkIjoiazNXbWFXd0FoN1hsenFyNkx5X1dlUHZLWnRMMGJuckYiLCJpYXQiOjE1NTU0MzI5MTIsImV4cCI6MjA1NjQ2ODkxMiwiYXRfaGFzaCI6IjVSRTR2UE54bHhCazhXRlpqNVdrc0EiLCJub25jZSI6IjUyUlRNWUp6Z0ZYWW1DS3Z4Sy1rUlZLYzV5TVp5VFhDIn0.ce5BpBqXFHcjlc8_9LNvfY1OTkQkirRnz2i5Rkm2nx0",
              )
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