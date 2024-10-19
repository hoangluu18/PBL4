from datetime import datetime
from pydantic import BaseModel, FilePath

class ResultDetection(BaseModel):
    dateCreate: datetime
    images: FilePath
    result: str
