import json
import threading
from typing import Any
from GraphQL.graphql_manager import GraphQLManager
from utils.image import ImageDownloader
from config import Config
from ipc.unix_socket import UnixSocketServer
from ipc.ipc_types import IPCMessage, IPCStatus
from GraphQL.types import ChapterConditionInput, ExtensionInput, FetchSourceMangaInput, FetchSourceMangaType, Manga, MangasConditionInput, SourceConditionInput
from utils.debug import debug_print, setup_logging
from utils.message import InputMessage, create_message


def init_extension_repo(graphql_manager: GraphQLManager):
    settings = graphql_manager.get_settings()
    if not settings["extensionRepos"]:
        debug_print("No extension repos found. Setting default repo...")
        graphql_manager.set_settings(Config.DEFAULT_EXTENSION_REPO)
        settings = graphql_manager.get_settings()
        debug_print("Default repo set:", settings["extensionRepos"])
    
    debug_print("Current extension repos:", settings["extensionRepos"])

if __name__ == "__main__":
    setup_logging()
    graphql_manager = GraphQLManager()
    server = UnixSocketServer()
    
    try:
        settings = graphql_manager.get_settings()
        init_extension_repo(graphql_manager)
    except Exception as e:
        debug_print(f"Warning: Could not connect to GraphQL server: {e}")
        debug_print("Socket server will still start, waiting for GraphQL server...")
        settings = {}

    def handle_ping(_param: Any) -> bytes:
        debug_print("ping")
        dict = {"message": "pong"}
        return create_message(IPCStatus.OK.value, f"{dict}")

    def handle_get_settings(_param: Any) -> bytes:
        settings = graphql_manager.get_settings()
        return create_message(IPCStatus.OK.value, json.dumps(settings))

    def handle_get_extensions(input: InputMessage) -> bytes:
        try:
            pagination = input.pagination or {}
            params = input.params or {}
            condition = ExtensionInput(**params)
            debug_print(condition)
            fetch_result = graphql_manager.fetch_extensions()
            debug_print(f"Fetched extensions result: {json.dumps(fetch_result, indent=2)}")
            extensions = graphql_manager.get_extensions(condition, pagination)
            total_count = extensions["extensions"].get("totalCount", 0)
            extensions = extensions["extensions"]["nodes"]
            message: dict[str, Any] = {
                "count": len(extensions),
                "totalCount": total_count,
                "extensions": extensions,
            }
            return create_message(IPCStatus.OK.value, json.dumps(message))
        except Exception as e:
            debug_print(f"Error getting extensions: {e}")
            return create_message(IPCStatus.ERROR.value, f"Error getting extensions: {e}")
        
    def handle_install_extension(input: InputMessage) -> bytes:
        try:
            params = input.params or {}
            id = params["id"]
            debug_print(f"Installing extension: {id}")
            ret = graphql_manager.install_extension(id)
            return create_message(IPCStatus.OK.value, json.dumps(ret))
        except Exception as e:
            debug_print(f"Error installing extension: {e}")
            return create_message(IPCStatus.ERROR.value, f"Error installing extension: {e}")
        
    def handle_uninstall_extension(input: InputMessage) -> bytes:
        try:
            params = input.params or {}
            id = params["id"]
            debug_print(f"Uninstalling extension: {id}")
            ret = graphql_manager.uninstall_extension(id)
            return create_message(IPCStatus.OK.value, json.dumps(ret))
        except Exception as e:
            debug_print(f"Error uninstalling extension: {e}")
            return create_message(IPCStatus.ERROR.value, f"Error uninstalling extension: {e}")
        
    def handle_get_sources(input: InputMessage) -> bytes:
        try:
            params = input.params or {}
            condition = SourceConditionInput(**params)
            debug_print(f"Getting sources with condition: {condition}")
            sources = graphql_manager.get_sources(condition)
            total_count = sources["sources"].get("totalCount", 0)
            sources = sources["sources"]["nodes"]
            message: dict[str, Any] = {
                "count": len(sources),
                "totalCount": total_count,
                "sources": sources,
            }
            return create_message(IPCStatus.OK.value, json.dumps(message))
        except Exception as e:
            debug_print(f"Error getting sources: {e}")
            return create_message(IPCStatus.ERROR.value, f"Error getting sources: {e}")
        
    def handle_get_manga(input: InputMessage) -> bytes:
        try:
            pagination = input.pagination or {}
            params = input.params or {}
            source_id = params["id"]
            debug_print(f"Fetching manga from source id: {source_id}")

            if source_id is None:
                return create_message(IPCStatus.ERROR.value, "Missing source id in params")

            fetch_input = FetchSourceMangaInput(source=source_id, page=1, query=None, type=FetchSourceMangaType.LATEST.name)
            fetched_mangas_data = graphql_manager.fetch_source_mangas(fetch_input)
            debug_print(f"Fetched manga data from source: {json.dumps(fetched_mangas_data, indent=2)}")

            condition = MangasConditionInput(sourceId=str(source_id))
            mangas_data = graphql_manager.get_mangas(condition, pagination)
            total_manga_count = mangas_data["mangas"].get("totalCount", 0)

            debug_print(f"Raw manga data from source: {json.dumps(mangas_data, indent=2)}")
            mangas = [Manga(**manga) for manga in mangas_data["mangas"]["nodes"]]
            mangaCount = len(mangas)
            debug_print(f"Fetched {mangaCount} mangas from source: {mangas}")

            # 4. Download thumbnails
            manga_dict: list[dict[str, Any]] = [{"thumbnailUrl": manga.thumbnailUrl, "title": manga.title, "author": manga.author, "id": manga.id} for manga in mangas]
            thumbnails_url = [manga["thumbnailUrl"] for manga in manga_dict]

            image_downloader = ImageDownloader()
            thumbnail_img_path = image_downloader.donwload_images_from_urls(thumbnails_url, Config.MANGA_THUMBNAIL_PATH)

            for manga, thumbnail_path in zip(manga_dict, thumbnail_img_path):
                manga["thumbnailPath"] = thumbnail_path

            for manga in manga_dict:
                manga.pop("thumbnailUrl", None)

            debug_print(f"Manga data to be sent to extension: {json.dumps(manga_dict, indent=2)}")

            message: dict[str, Any] = {
                "count": mangaCount,
                "totalCount": total_manga_count,
                "mangas": manga_dict,
            }
 
            return create_message(IPCStatus.OK.value, json.dumps(message))

        except Exception as e:
            debug_print(f"Error fetching manga from source: {e}")
            return create_message(IPCStatus.ERROR.value, f"Error fetching manga from source: {e}")
        
    def handle_get_chapters(input: InputMessage) -> bytes:
        try:
            pagination = input.pagination or {}
            params = input.params or {}
            conditions = ChapterConditionInput(**params)
            chapters_data = graphql_manager.get_chapters(conditions, pagination)
            chapters = chapters_data["chapters"]["nodes"]
            num_chapters = len(chapters)
            total_chapter_count = chapters_data["chapters"].get("totalCount", 0)

            response: dict[str, Any] = {
                "count": num_chapters,
                "totalCount": total_chapter_count,
                "chapters": chapters,
            }
            return create_message(IPCStatus.OK.value, json.dumps(response))
        except Exception as e:
            debug_print(f"Error fetching chapters: {e}")
            return create_message(IPCStatus.ERROR.value, f"Error fetching chapters from source: {e}")
            

    server.add_response_callback(IPCMessage.IPC_MSG_PING, handle_ping)
    server.add_response_callback(IPCMessage.IPC_MSG_GET_SETTINGS, handle_get_settings)
    server.add_response_callback(IPCMessage.IPC_MSG_GET_EXTENSIONS, handle_get_extensions)
    server.add_response_callback(IPCMessage.IPC_MSG_INSTALL_EXTENSION, handle_install_extension)
    server.add_response_callback(IPCMessage.IPC_MSG_UNINSTALL_EXTENSION, handle_uninstall_extension)
    server.add_response_callback(IPCMessage.IPC_MSG_GET_SOURCES, handle_get_sources)
    server.add_response_callback(IPCMessage.IPC_MSG_GET_MANGAS, handle_get_manga)
    server.add_response_callback(IPCMessage.IPC_MSG_OPEN_MANGA, handle_get_chapters)
    server.start()
    debug_print("Unix socket server is running...")
    try:
        while True:
            threading.Event().wait(1)
    except KeyboardInterrupt:
        server.stop()