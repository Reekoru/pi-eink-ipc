from gql import gql


GET_SETTINGS = gql(
    """
    query GetExtensionRepo {
        settings {
            extensionRepos
        }
    }
    """
)

SET_SETTINGS = gql(
    """
    mutation MyMutation($input: SetSettingsInput = {settings: {}}) {
        setSettings(input: $input) {
            settings {
                extensionRepos
            }
        }
    }
    """
)
