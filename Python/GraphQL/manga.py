from gql import gql


MANGA_FRAGMENT = """
    fragment MANGA_FRAGMENT on MangaType {
        age
        artist
        author
        bookmarkCount
        description
        hasDuplicateChapters
        genre
        highestNumberedChapter {
            chapterNumber
            name
            id
        }
        id
        status
        thumbnailUrl
        status
        title
        inLibrary
        url
    }
"""

GET_MANGAS = gql(
    MANGA_FRAGMENT + """
    query GET_MANGAS($condition: MangaConditionInput!, $first: Int, $offset: Int) {
        mangas(condition: $condition, first: $first, offset: $offset) {
            nodes {
                ...MANGA_FRAGMENT
            }
        }
    }
    """
)

FETCH_SOURCE_MANGA = gql(
    MANGA_FRAGMENT + """
    mutation FETCH_SOURCE_MANGA($input: FetchSourceMangaInput!) {
        fetchSourceManga(input: $input) {
            mangas {
                ...MANGA_FRAGMENT
            }
        }
    }
    """
)