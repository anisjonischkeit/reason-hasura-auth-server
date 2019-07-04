open Graphql_lwt.Schema;

type signupFailureTypes =
  | WeakPassword
  | EmailAlreadyInUse;

let (success, failure, signupResult) = {
  let signupSuccess = CredentialsSchema.make("SignupSuccess");

  let signupFailure =
    obj("SignupFailure", ~fields=_ =>
      [
        field(
          "reason",
          ~typ=
            non_null(
              enum(
                "FailureReason",
                ~values=[
                  enum_value("WeakPassword", ~value=WeakPassword),
                  enum_value("EmailAlreadyInUse", ~value=EmailAlreadyInUse),
                ],
              ),
            ),
          ~args=Arg.[],
          ~resolve=(_, failureType: signupFailureTypes) =>
          failureType
        ),
      ]
    );

  let signupResult: abstract_typ(unit, [ | `signupResult]) =
    union("SignupResult");

  let successConstructor = add_type(signupResult, signupSuccess);
  let signupFailureConstructor = add_type(signupResult, signupFailure);
  (successConstructor, signupFailureConstructor, signupResult);
};