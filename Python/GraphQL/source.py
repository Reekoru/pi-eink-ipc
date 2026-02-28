from gql import gql

SOURCE_FRAGMENT ="""
    fragment SOURCE_FRAGMENT on SourceType {
        displayName
        iconUrl
        id
        isConfigurable
        isNsfw
        lang
        name
        supportsLatest
    }
    """

GET_SOURCES = gql(
    SOURCE_FRAGMENT + """
    query GET_SOURCES($condition: SourceConditionInput) {
        sources(condition: $condition) {
            totalCount
            nodes {
                ...SOURCE_FRAGMENT
            }
        }
    }
    """
)