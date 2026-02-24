import json
import threading
from typing import Any
from GraphQL.graphql_manager import GraphQLManager
from config import Config
from ipc.unix_socket import UnixSocketServer
from ipc.ipc_types import IPCMessage, IPCStatus
from GraphQL.types import ExtensionInput, FetchSourceMangaInput, FetchSourceMangaResult, FetchSourceMangaType, SourceConditionInput
from utils.message import create_message


def init_extension_repo(graphql_manager: GraphQLManager):
    settings = graphql_manager.get_settings()
    if not settings["extensionRepos"]:
        print("No extension repos found. Setting default repo...")
        graphql_manager.set_settings(Config.DEFAULT_EXTENSION_REPO)
        settings = graphql_manager.get_settings()
        print("Default repo set:", settings["extensionRepos"])
    
    print("Current extension repos:", settings["extensionRepos"])

if __name__ == "__main__":
    graphql_manager = GraphQLManager()
    server = UnixSocketServer()
    
    try:
        settings = graphql_manager.get_settings()
        init_extension_repo(graphql_manager)
    except Exception as e:
        print(f"Warning: Could not connect to GraphQL server: {e}")
        print("Socket server will still start, waiting for GraphQL server...")
        settings = {}

    def handle_ping(_param: Any) -> bytes:
        print("ping")
        dict = {"message": "pong"}
        return create_message(IPCStatus.OK.value, f"{dict}")

    def handle_get_settings(_param: Any) -> bytes:
        settings = graphql_manager.get_settings()
        return create_message(IPCStatus.OK.value, f"{settings}")

    def handle_get_extensions(param: dict[str, Any]) -> bytes:
        try:
            pagination = param["pagination"]
            param.pop("pagination", None)
            input = ExtensionInput(**param)
            print(input)
            ret = graphql_manager.get_extensions(input, pagination)
            return create_message(IPCStatus.OK.value, ret)
        except Exception as e:
            print(f"Error getting extensions: {e}")
            return create_message(IPCStatus.ERROR.value, f"Error getting extensions: {e}")
        
    def handle_install_extension(param: dict[str, Any]) -> bytes:
        try:
            id = param["id"]
            print(f"Installing extension: {id}")
            ret = graphql_manager.install_extension(id)
            return create_message(IPCStatus.OK.value, ret)
        except Exception as e:
            print(f"Error installing extension: {e}")
            return create_message(IPCStatus.ERROR.value, f"Error installing extension: {e}")
        
    def handle_uninstall_extension(param: dict[str, Any]) -> bytes:
        try:
            id = param["id"]
            print(f"Uninstalling extension: {id}")
            ret = graphql_manager.uninstall_extension(id)
            return create_message(IPCStatus.OK.value, ret)
        except Exception as e:
            print(f"Error uninstalling extension: {e}")
            return create_message(IPCStatus.ERROR.value, f"Error uninstalling extension: {e}")
        
    def handle_fetch_source_manga(param: dict[str, Any]) -> bytes:
        try:

            # 1: Get Source ID
            input = SourceConditionInput(**param)
            print(f"Fetching manga from source with conditions: {input}")
            ret = graphql_manager.get_sources(input)
        
            # 2: Get manga from ID
            json_dict = json.loads(ret)
            sources = json_dict["sources"]
            total_count = sources["totalCount"]
            if total_count == 0 or len(sources) == 0:
                return create_message(IPCStatus.OK.value, f"No sources found with conditions: {input}")
            
            nodes = sources["nodes"]
            
            # 3: Get first valid id (Not 0)

            source_id: str | None = next((source["id"] for source in nodes if source["id"] != "0"), None)
            print(f"Found source ID: {source_id}")

            if source_id is None:
                return create_message(IPCStatus.OK.value, f"No valid sources found with conditions: {input}")

            input = FetchSourceMangaInput(source=source_id, page=1, query=None, type=FetchSourceMangaType.LATEST.name)
            ret = graphql_manager.fetch_source_mangas(input)

            fetchSourceMangas = json.loads(ret)

            fetch_source_manga_results = [FetchSourceMangaResult(**manga) for manga in fetchSourceMangas["fetchSourceManga"]]
            print(f"Fetched manga from source: {fetch_source_manga_results}")
 
            return create_message(IPCStatus.OK.value, ret)

        except Exception as e:
            print(f"Error fetching manga from source: {e}")
            return create_message(IPCStatus.ERROR.value, f"Error fetching manga from source: {e}")
            

    server.add_response_callback(IPCMessage.IPC_MSG_PING, handle_ping)
    server.add_response_callback(IPCMessage.IPC_MSG_GET_SETTINGS, handle_get_settings)
    server.add_response_callback(IPCMessage.IPC_MSG_GET_EXTENSIONS, handle_get_extensions)
    server.add_response_callback(IPCMessage.IPC_MSG_INSTALL_EXTENSION, handle_install_extension)
    server.add_response_callback(IPCMessage.IPC_MSG_UNINSTALL_EXTENSION, handle_uninstall_extension)
    server.add_response_callback(IPCMessage.IPC_MSG_OPEN_EXTENSION_REPO, handle_fetch_source_manga)
    server.start()
    print("Unix socket server is running...")
    try:
        while True:
            threading.Event().wait(1)
    except KeyboardInterrupt:
        server.stop()