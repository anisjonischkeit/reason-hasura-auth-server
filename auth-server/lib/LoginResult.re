open Graphql_lwt.Schema;

type loginFailureTypes =
  | InvalidCredentials;

let (success, failure, loginResult) = {
  let loginSuccess = CredentialsSchema.make("LoginSuccess");

  let loginFailure =
    obj("LoginFailure", ~fields=_ =>
      [
        field(
          "reason",
          ~typ=
            non_null(
              enum(
                "FailureReason",
                ~values=[
                  enum_value("InvalidCredentials", ~value=InvalidCredentials),
                ],
              ),
            ),
          ~args=Arg.[],
          ~resolve=(_, failureType: loginFailureTypes) =>
          failureType
        ),
      ]
    );

  let loginResult: abstract_typ(unit, [ | `loginResult]) =
    union("LoginResult");

  let successConstructor = add_type(loginResult, loginSuccess);
  let loginFailureConstructor = add_type(loginResult, loginFailure);
  (successConstructor, loginFailureConstructor, loginResult);
};