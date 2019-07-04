#! /bin/bash   

source .env
hasura migrate apply --endpoint http://localhost:8080 --admin-secret ${HASURA_GRAPHQL_ADMIN_SECRET}
