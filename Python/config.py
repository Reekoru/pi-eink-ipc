from dotenv import load_dotenv
import os

load_dotenv()

class Config:
    URL = os.getenv('GRAPHQL_URL', 'sqlite:///app.db')
    ENDPOINT = os.getenv('GRAPHQL_ENDPOINT', 'api/graphql')
    DEFAULT_EXTENSION_REPO = os.getenv('EXTENSION_REPO', 'https://raw.githubusercontent.com/EnriqueCordero/suwayomi-extension-repo/main/extensions.json')
    EINK_TEMP_FOLDER = os.getenv('EINK_TEMP_FOLDER', 'suwayomi_fetcher_eink')
    MANGA_THUMBNAIL_PATH = os.getenv('MANGA_THUMBNAIL_PATH', 'source/thumbnails')
    LOG_LEVEL = os.getenv('LOG_LEVEL', 'INFO').upper()