#! /bin/bash   

source .env
hasura console --admin-secret ${HASURA_GRAPHQL_ADMIN_SECRET}