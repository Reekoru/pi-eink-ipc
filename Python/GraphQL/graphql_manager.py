from dataclasses import asdict
from attrs import asdict
from typing import Any
from gql import Client
from gql.transport.aiohttp import AIOHTTPTransport
from GraphQL.types import ChapterConditionInput, ExtensionInput, FetchSourceMangaInput, MangasConditionInput, SourceConditionInput
from GraphQL.manga import FETCH_SOURCE_MANGA, GET_MANGAS
from GraphQL.chapter import FETCH_CHAPTERS, GET_CHAPTERS
from config import Config
from GraphQL.exntesion import FETCH_EXTENSIONS, GET_EXTENSIONS, UPDATE_EXTENSION
from GraphQL.settings import GET_SETTINGS, SET_SETTINGS
from GraphQL.source import GET_SOURCES

class GraphQLManager:
    
    def __init__(self):
      transport = AIOHTTPTransport(url=f"{Config.URL}/{Config.ENDPOINT}")
      self.client = Client(transport=transport)

    def get_extensions(self, condition: ExtensionInput, pagination: dict[str, Any]):
      result = self.client.execute(GET_EXTENSIONS, variable_values={"input": asdict(condition), "first": pagination["first"], "offset": pagination["offset"]})
      return result

    def fetch_extensions(self):
      result = self.client.execute(FETCH_EXTENSIONS)
      return result
    
    def install_extension(self, pkg_name: str):
      input: dict[str, Any] = {"id": pkg_name, "patch": {"install": True}}
      result = self.client.execute(UPDATE_EXTENSION, variable_values={"input": input})
      return result
    
    def uninstall_extension(self, pkg_name: str):
      input: dict[str, Any] = {"id": pkg_name, "patch": {"uninstall": True}}
      result = self.client.execute(UPDATE_EXTENSION, variable_values={"input": input})
      return result
    
    def get_sources(self, input_data: SourceConditionInput):
      result = self.client.execute(GET_SOURCES, variable_values={"condition": asdict(input_data)})
      return result

    def fetch_source_mangas(self, sourceMangaInput: FetchSourceMangaInput):
      result = self.client.execute(FETCH_SOURCE_MANGA, variable_values={"input": asdict(sourceMangaInput)})
      return result
    
    def get_mangas(self, condition: MangasConditionInput, pagination: dict[str, Any]):
      result = self.client.execute(GET_MANGAS, variable_values={"condition": asdict(condition), "first": pagination["first"], "offset": pagination["offset"]})
      return result
    
    def get_chapters(self, condition: ChapterConditionInput, pagination: dict[str, Any]):
      result = self.client.execute(GET_CHAPTERS, variable_values={"condition": asdict(condition), "first": pagination["first"], "offset": pagination["offset"]})
      return result

    def fetch_chapters(self, manga_id: int):
      result = self.client.execute(FETCH_CHAPTERS, variable_values={"input": {"mangaId": manga_id}})
      return result

    def get_settings(self) -> dict[str, Any]:
      result = self.client.execute(GET_SETTINGS)
      return result["settings"]
       
    def set_settings(self, repo_url: str):
      input_data: dict[str, Any] = {"settings": {"extensionRepos": [repo_url]}}
      result = self.client.execute(SET_SETTINGS, variable_values={"input": input_data})
      return result