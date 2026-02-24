from gql import gql

EXTENSION_FRAGMENT = """
    fragment EXTENSION_FRAGMENT on ExtensionType {
        apkName
        hasUpdate
        iconUrl
        isInstalled
        isNsfw
        isObsolete
        lang
        name
        pkgName
        repo
        versionCode
        versionName
    }
"""


GET_EXTENSIONS = gql(
    EXTENSION_FRAGMENT + """
    query GET_EXTENSIONS($first: Int, $input: ExtensionConditionInput, $offset: Int) {
        extensions(first: $first, condition: $input, offset: $offset) {
            totalCount
            nodes {
                ...EXTENSION_FRAGMENT
            }
        }
    }
"""
)

UPDATE_EXTENSION = gql(
    EXTENSION_FRAGMENT + """
    mutation UPDATE_EXTENSION($input: UpdateExtensionInput!) {
        updateExtension(input: $input) {
            extension {
                ...EXTENSION_FRAGMENT
            }
        }
    }
"""
)

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