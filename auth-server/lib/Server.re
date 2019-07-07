open Lwt.Infix;

open Graphql_lwt;

let key = Sys.getenv("HASURA_GRAPHQL_JWT_SECRET");
let randomSeed = Random.State.make_self_init();
let genUuid = Uuidm.v4_gen(randomSeed); // TODO: might need to change this for extra randomness
let aMonthInSeconds = 30 * 24 * 60 * 60;

let mutations = {
  /* let data = {sub: "1234567890", name: "John Doe", iat: 1516239022}; */
  /* jwtPayload_to_json(data); */
  Schema.[
    io_field(
      "login",
      ~typ=non_null(LoginResult.loginResult),
      ~doc="Authenticates a user and returns a jwt access token",
      ~args=
        Arg.[
          arg("email", ~typ=non_null(string)),
          arg("password", ~typ=non_null(string)),
        ],
      ~resolve=(_ctx, (), email, password) =>
      GQLQueries.(GetUser.make(~email, ()) |> sendQuery)
      >|= (
        res => {
          switch (res) {
          | Ok(data) =>
            switch (data#users) {
            | [|user|] =>
              switch (user#id) {
              | `String(id) =>
                if (Bcrypt.verify(
                      password,
                      user#password_hash |> Bcrypt.hash_of_string,
                    )) {
                  let currentTime = Unix.time() |> int_of_float;
                  Ok(
                    CredentialsSchema.{
                      userId: id,
                      iat: currentTime,
                      exp: currentTime + aMonthInSeconds,
                    }
                    |> LoginResult.success,
                  );
                } else {
                  Ok(LoginResult.failure(InvalidCredentials));
                }
              | _ => Error("eeek")
              }
            | [||] => Ok(LoginResult.failure(InvalidCredentials))
            | _ => Error("something went wrong terribly wrong") // somehow more than one user with an email
            }
          | Error(e) => Error("something went wrong fetching the data")
          };
        }
      )
    ),
    io_field(
      "signup",
      ~typ=non_null(SignupResult.signupResult),
      ~doc="Signs a user up and returns a jwt access token",
      ~args=
        Arg.[
          arg("email", ~typ=non_null(string)),
          arg("firstName", ~typ=non_null(string)),
          arg("lastName", ~typ=non_null(string)),
          arg("password", ~typ=non_null(string)),
        ],
      ~resolve=(_ctx, (), email, firstName, lastName, password) => {
        let passwordHash = Bcrypt.hash(password) |> Bcrypt.string_of_hash;

        GQLQueries.(
          InsertUser.make(
            ~email,
            ~first_name=firstName,
            ~last_name=lastName,
            ~password_hash=passwordHash,
            (),
          )
          |> sendQuery
        )
        >|= (
          res => {
            switch (res) {
            | Ok(data) =>
              switch (data#insert_users) {
              | Some(insertRes) =>
                switch (insertRes#returning[0]#id) {
                | `String(id) =>
                  let currentTime = Unix.time() |> int_of_float;

                  Ok(
                    CredentialsSchema.{
                      userId: id,
                      iat: currentTime,
                      exp: currentTime + aMonthInSeconds,
                    }
                    |> SignupResult.success,
                  );
                }
              | None =>
                print_endline("didn't manage to insert user wtf");
                Ok(SignupResult.(failure(EmailAlreadyInUse)));
              }

            | Error(e) =>
              print_endline(e |> Yojson.Basic.to_string);
              Error("Something went wrong inserting a user");
            };
          }
        ); // somehow more than one user with an email
      },
    ),
  ];
};

type graphqlCtx = {authHeader: option(string)};

let schema =
  Schema.(
    schema(
      ~mutations,
      [
        field(
          "healthCheck",
          ~typ=non_null(string),
          ~args=Arg.[],
          ~resolve=(_, ()) =>
          "ok"
        ),
        field(
          "isJWTValid",
          ~typ=non_null(bool),
          ~args=Arg.[arg("jwt", ~typ=non_null(string))],
          ~resolve=(_, (), jwt) =>
          switch (JWT.verify(key, jwt)) {
          | Ok(v) => v
          | Error(_) => false
          }
        ),
      ],
    )
  );

module Graphql_cohttp_lwt =
  Graphql_cohttp.Make(
    Graphql_lwt.Schema,
    Cohttp_lwt_unix.IO,
    Cohttp_lwt.Body,
  );

let run = () => {
  let on_exn =
    fun
    | [@implicit_arity] Unix.Unix_error(error, func, arg) =>
      Logs.warn(m =>
        m(
          "Client connection error %s: %s(%S)",
          Unix.error_message(error),
          func,
          arg,
        )
      )
    | exn => Logs.err(m => m("Unhandled exception: %a", Fmt.exn, exn));
  let callback = Graphql_cohttp_lwt.make_callback(req => (), schema);
  let server = Cohttp_lwt_unix.Server.make_response_action(~callback, ());
  let mode = `TCP(`Port(1111));
  Cohttp_lwt_unix.Server.create(~on_exn, ~mode, server) |> Lwt_main.run;
};