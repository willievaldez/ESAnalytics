until curl -u "elastic:${ELASTIC_PASSWORD}" -sS http://elasticsearch:9200/_cat/health?h=status | grep -q "green\|yellow"; do
    echo "Waiting for elasticsearch..."
    sleep 3
done
    api_key_filename="/api_key.txt"

    # Check if previous api key exists and delete if necessary
    if [[ -s "$api_key_filename" ]]; then
        {
            read -r id
        } < "$api_key_filename"

        echo "Deleting previous API Key"
        curl -u "elastic:${ELASTIC_PASSWORD}" -X DELETE "http://elasticsearch:9200/_security/api_key" -H "Content-Type: application/json" -d'
        {
            "ids" : [ "'${id}'" ]
        }'
    fi

    echo "Generating API Key"
    response=$(curl -u "elastic:${ELASTIC_PASSWORD}" -X POST "http://elasticsearch:9200/_security/api_key" -H "Content-Type: application/json" -d'
    {
        "name": "hawthorne-client",
        "role_descriptors": { 
            "bulk_ingest_role": {
                "cluster": [],
                "indices": [
                    {
                        "names": ["'${ELASTIC_INDEX}'*"],
                        "privileges": ["write"]
                    }
                ]
            }
        }
    }')

    id=$(echo "$response" | sed -n 's/.*"id":"\([^"]*\)".*/\1/p')
    encoded=$(echo "$response" | sed -n 's/.*"encoded":"\([^"]*\)".*/\1/p')

    echo -e "${id}\n${encoded}" > "$api_key_filename"
