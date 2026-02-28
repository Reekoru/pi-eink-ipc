from gql import gql

CHAPTER_FRAGMENT = """
    fragment CHAPTER_FRAGMENT on ChapterType {
        chapterNumber
        id
        isBookmarked
        isDownloaded
        isRead
        name
        pageCount
        lastPageRead
        lastReadAt    
        }
        """

GET_CHAPTERS = gql(
    CHAPTER_FRAGMENT + """
    query GET_CHAPTERS($condition: ChapterConditionInput!, $first: Int, $offset: Int) {
        chapters(condition: $condition, first: $first, offset: $offset) {
            totalCount
            nodes {
                ...CHAPTER_FRAGMENT
            }
        }
    }
    """
)

FETCH_CHAPTERS = gql(
    CHAPTER_FRAGMENT + """
        mutation GET_MANGA_CHAPTERS_FETCH($input: FetchChaptersInput!) {
        fetchChapters(input: $input) {
            chapters {
            ...CHAPTER_FRAGMENT
            }
        }
    }
    """
)