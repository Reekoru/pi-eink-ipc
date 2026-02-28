import logging
from config import Config


def setup_logging() -> None:
    level = getattr(logging, Config.LOG_LEVEL, logging.INFO)
    logging.basicConfig(
        level=level,
        format="%(asctime)s %(levelname)s %(name)s: %(message)s",
        force=True,
    )


def get_logger(name: str) -> logging.Logger:
    return logging.getLogger(name)


def debug_print(*args, **kwargs) -> None:
    logger = get_logger("suwayomi")
    sep = kwargs.get("sep", " ")
    message = sep.join(str(arg) for arg in args)
    logger.debug(message)
