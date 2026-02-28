


from enum import Enum
from typing import Any, Optional
from attr import dataclass
    
class FetchSourceMangaType(Enum):
    SEARCH = 0
    POPULAR = 1
    LATEST = 2

@dataclass
class ExtensionInput:
    apkName: Optional[str] = None
    hasUpdate: Optional[bool] = None
    iconUrl: Optional[str] = None
    isInstalled: Optional[bool] = None
    isNsfw: Optional[bool] = None
    isObsolete: Optional[bool] = None 
    lang: Optional[str] = None
    name: Optional[str] = None
    pkgName: Optional[str] = None
    repo: Optional[str] = None
    versionCode: Optional[int] = None
    versionName: Optional[str] = None

@dataclass
class SourceConditionInput:
    id: Optional[str] = None
    isNsfw: Optional[bool] = None
    name: Optional[str] = None
    lang: Optional[str] = None

@dataclass
class FetchSourceMangaInput:
    source: str
    page: Optional[int] = None
    query: Optional[str] = None
    type: Optional[str] = None

@dataclass
class MangasConditionInput:
    sourceId: Optional[str] = None
    title: Optional[str] = None
    author: Optional[str] = None

@dataclass
class ChapterConditionInput:
    mangaId: int
    chapterNumber: Optional[float] = None

@dataclass
class Manga:
    age: str
    artist: str
    author: str
    bookmarkCount: int
    description: str
    hasDuplicateChapters: bool
    genre: list[str]
    highestNumberedChapter: dict[str, Any]
    id: int
    thumbnailUrl: str
    status: str
    title: str
    inLibrary: bool
    url: str
@dataclass
class FetchSourceMangaResult:
    manga: list[Manga]