from dotenv import load_dotenv
import os

load_dotenv()

class Config:
    URL = os.getenv('GRAPHQL_URL', 'sqlite:///app.db')
    ENDPOINT = os.getenv('GRAPHQL_ENDPOINT', 'api/graphql')
    DEFAULT_EXTENSION_REPO = os.getenv('EXTENSION_REPO', 'https://raw.githubusercontent.com/EnriqueCordero/suwayomi-extension-repo/main/extensions.json')