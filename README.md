# Reason Native Hasura Authentication Server

This is a sample that shows how a simple Reason Native authentication server for Hasura could look.

## Running the server

Terminal 1

```
cd auth-server
esy start
```

Terminal 2

```
cd hasura
docker-compose up -d # this starts hasura in the background, stop it with docker-compose down
scripts/run-migrations.sh
scripts/start-server.sh
```

Now go to http://localhost:9695/

NOTE: Make sure to start the auth server before you start hasura since hasura depends on it being up.
