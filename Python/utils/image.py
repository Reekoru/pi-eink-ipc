import requests
import tempfile
import os

class DownloadImageResult:
    status: bool
    num_files: int
    files: list[str]

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
                print(f"Error deleting file {filepath}: {e}")
        self.clear_files()

    def delete_file(self, filepath: str) -> None:
        if filepath in self.files:
            try:
                os.remove(filepath)
                self.files.remove(filepath)
                self.num_files -= 1
            except OSError as e:
                print(f"Error deleting file {filepath}: {e}")
        else:
            print(f"File {filepath} not found in result files.")

"""
Downloads an image from the url. Filename is concatenated with the temp dir path.
"""
def download_image_from_url(url: str, filename: str) -> bool:
    try:
        response = requests.get(url)
        response.raise_for_status()

        tempdir = tempfile.gettempdir()
        filepath = os.path.join(tempdir, filename)
        print(f"Downloading image from {url} to {filepath}...")

        with open(filepath, "wb") as f:
            f.write(response.content)
        return True
    except Exception as e:
        print(f"Error downloading image: {e}")
        return False