# ElasticSearch Analytics Plugin
Drop this plugin into your project to get up and running quickly with elastic search and kibana analytics!
This plugin contains the necessary docker files that will allow you to get a docker cluster running with one command.

## Emitting Analytics
Simply call `UESAnalytics::EmitAnalytic` either in C++ or blueprint. The following properties are available:
 * Name - The name associated with the analytic event
 * Entity (Optional) - The AActor associated with the analytic event
 * Entity Name (Optional) - The name of the entity associated with the analytic event. If blank, and Entity is provided, will use that actor’s name
 * Metadata (Optional) - Additional metadata associated with the event
 * Index Override (Optional/Advanced Usage) - Will instead emit to the specified elastic search index

## Deploying The Cluster
 * Docker
    * Install Docker Desktop: https://www.docker.com/products/docker-desktop/
    * Open a terminal at `Plugins/ESAnalytics/Source/ESAnalytics/Docker`
    * Run `docker compose up -d`
 * Configure Unreal with API key
    * After `docker compose` finishes, open `Plugins/ESAnalytics/Source/ESAnalytics/Docker/api_key.txt`
    * Open `Config/DefaultGame.ini` and copy line 2 of `api_key.txt` into the following:
```
[/Script/ESAnalytics.ESAnalyticsSettings]
Analytics.ApiKey=<paste here>
```

 * Expose Ports
    * 9200 for data ingress (elastic search)
    * 5601 for data egress (kibana)
 * Regenerate API Key (as needed)
    * Open a terminal at `Plugins/ESAnalytics/Source/ESAnalytics/Docker`
    * Run `docker compose run --rm apikeygen`
    * Repeat step "Configure Unreal with API key"

## Configurations
 * Analytics Stack: `Plugins/ESAnalytics/Source/ESAnalytics/Docker/.env`
    * ELASTIC_PASSWORD
    * KIBANA_USER
    * KIBANA_PASSWORD
    * ELASTIC_INDEX
 * UE: `Plugins/ESAnalytics/Source/ESAnalytics/Private/ESAnalyticsSettings.h`
    * Configs can/should be modified in editor `Project Settings`. They will be saved in `DefaultGame.ini`
        * bEnabled (CVAR `Analytics.Enabled`)
        * Endpoint (CVAR `Analytics.Endpoint`)
        * ApiKey (CVAR `Analytics.ApiKey`)
        * Index (CVAR `Analytics.Index`)
        * MaxPayloadSize (CVAR `Analytics.MaxPayloadSize`)
        * FlushInterval

## Gotchas
 * Perforce sucks at line endings! Make sure all shell scripts use LF line endings. Check the following files:
    * //ES/main/Plugins/ESAnalytics/Docker/startup.sh
    * //ES/main/Plugins/ESAnalytics/Docker/apikeygen.sh
 * Make sure you are not emitting the same analytic event multiple times when running multiplayer!
