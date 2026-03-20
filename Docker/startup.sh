#!/bin/bash

until curl -u "elastic:${ELASTIC_PASSWORD}" -sS http://elasticsearch:9200/_cat/health?h=status | grep -q "green\|yellow"; do
    echo "Waiting for elasticsearch..."
    sleep 3
done
    echo "Creating 'kibana_system_access' role"
    curl -u "elastic:${ELASTIC_PASSWORD}" -X POST "http://elasticsearch:9200/_security/role/kibana_system_access" -H "Content-Type: application/json" -d'
    {
        "cluster": ["all"],
        "indices": [
        {
            "names": [".kibana*", ".apm*"],
            "privileges": ["all"],
            "allow_restricted_indices": true
        }
        ]
    }'
    echo "Creating '${KIBANA_USER}' user"
    curl -u "elastic:${ELASTIC_PASSWORD}" -X POST "http://elasticsearch:9200/_security/user/${KIBANA_USER}" -H "Content-Type: application/json" -d'
    {
        "password": "'${KIBANA_PASSWORD}'",
        "roles": ["superuser", "kibana_system_access"],
        "full_name": "Kibana Admin"
    }'
    echo "updating password for 'kibana_system' user"
    curl -u elastic:${ELASTIC_PASSWORD} -X POST "http://elasticsearch:9200/_security/user/kibana_system/_password" -H "Content-Type: application/json" -d'
    {
        "password": "'${KIBANA_PASSWORD}'"
    }'
    echo "generating mappings for '${ELASTIC_INDEX}' index"
    curl -u "elastic:${ELASTIC_PASSWORD}" -X PUT "http://elasticsearch:9200/${ELASTIC_INDEX}" -H "Content-Type: application/json" -d'
    {
    }'

    bash /apikeygen.sh