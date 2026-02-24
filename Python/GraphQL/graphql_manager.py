from dataclasses import asdict
from attrs import asdict
from typing import Any
from gql import Client
from gql.transport.aiohttp import AIOHTTPTransport
from GraphQL.types import ExtensionInput, FetchSourceMangaInput, SourceConditionInput
from config import Config
import json
from GraphQL.exntesion import GET_EXTENSIONS, GET_SETTINGS, SET_SETTINGS, UPDATE_EXTENSION
from GraphQL.source import FETCH_SOURCE_MANGA, GET_SOURCES

class GraphQLManager:
    
    def __init__(self):
      transport = AIOHTTPTransport(url=f"{Config.URL}/{Config.ENDPOINT}")
      self.client = Client(transport=transport)

    def get_extensions(self, condition: ExtensionInput, pagination: dict[str, Any]):
      result = self.client.execute(GET_EXTENSIONS, variable_values={"input": asdict(condition), "first": pagination["first"], "offset": pagination["offset"]})
      json_result = json.dumps(result, indent=4)
      return json_result
    
    def install_extension(self, pkg_name: str):
      input: dict[str, Any] = {"id": pkg_name, "patch": {"install": True}}
      result = self.client.execute(UPDATE_EXTENSION, variable_values={"input": input})
      json_result = json.dumps(result, indent=4)
      return json_result
    
    def uninstall_extension(self, pkg_name: str):
      input: dict[str, Any] = {"id": pkg_name, "patch": {"uninstall": True}}
      result = self.client.execute(UPDATE_EXTENSION, variable_values={"input": input})
      json_result = json.dumps(result, indent=4)
      return json_result
    
    def get_sources(self, input_data: SourceConditionInput):
      result = self.client.execute(GET_SOURCES, variable_values={"input": asdict(input_data)})
      json_result = json.dumps(result, indent=4)
      return json_result

    def fetch_source_mangas(self, sourceMangaInput: FetchSourceMangaInput):
      result = self.client.execute(FETCH_SOURCE_MANGA, variable_values={"input": asdict(sourceMangaInput)})
      json_result = json.dumps(result, indent=4)
      return json_result

    def get_settings(self) -> dict[str, Any]:
      result = self.client.execute(GET_SETTINGS)
      return result["settings"]
       
    def set_settings(self, repo_url: str):
      input_data: dict[str, Any] = {"settings": {"extensionRepos": [repo_url]}}
      result = self.client.execute(SET_SETTINGS, variable_values={"input": input_data})
      json_result = json.dumps(result, indent=4)
      return json_result