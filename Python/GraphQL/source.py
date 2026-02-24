from gql import gql
from GraphQL.manga import MANGA_FRAGMENT

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

FETCH_SOURCE_MANGA = gql(
    MANGA_FRAGMENT + """
    mutation FETCH_SOURCE_MANGA($input: FetchSourceMangaInput!) {
        fetchSourceManga(input: $input) {
            hasNextPage
            mangas {
                ...MANGA_FRAGMENT
            }
        }
    }
    """
)