import requests
import tempfile
import os
from urllib.parse import urljoin
from config import Config
from utils.debug import debug_print

class ImageDownloader:
    num_files: int
    files: list[str]

    def __init__(self) -> None:
        self.num_files = 0
        self.files = []

    def download_image_from_url(self, url: str, filepath: str, filename: str) -> bool:
        try:
            response = requests.get(url)
            response.raise_for_status()

            tempdir = tempfile.gettempdir()
            path = os.path.join(tempdir, Config.EINK_TEMP_FOLDER, filepath, filename)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            debug_print(f"Downloading image from {url} to {path}...")

            with open(path, "wb") as f:
                f.write(response.content)

            self.add_file(path)
            return True
        except Exception as e:
            debug_print(f"Error downloading image: {e}")
            return False
        
    def donwload_images_from_urls(self, urls: list[str], filepath: str) -> list[str]:
        for i, url in enumerate(urls):
            filename = f"{i}.jpg"
            url = urljoin(f"{Config.URL.rstrip('/')}/", url)
            debug_print(f"Processing image {i+1}/{len(urls)}: {url}")
            if self.download_image_from_url(url, filepath, filename):
                debug_print(f"Successfully downloaded image from {url} to {filepath}/{filename}")
            else:
                debug_print(f"Failed to download image from {url}")
        return self.files

    def add_file(self, filepath: str) -> None:
        self.files.append(filepath)
        self.num_files += 1

    def clear_files(self) -> None:
        self.files.clear()
        self.num_files = 0

    def delete_all_files(self) -> None:
        for filepath in self.files:
            try:
                os.remove(filepath)
            except OSError as e:
                debug_print(f"Error deleting file {filepath}: {e}")
        self.clear_files()

    def delete_file(self, filepath: str) -> None:
        if filepath in self.files:
            try:
                os.remove(filepath)
                self.files.remove(filepath)
                self.num_files -= 1
            except OSError as e:
                debug_print(f"Error deleting file {filepath}: {e}")
        else:
            debug_print(f"File {filepath} not found in result files.")