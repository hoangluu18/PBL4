from datetime import datetime
from pydantic import BaseModel, FilePath

class ImageViewModel(BaseModel):
    dateReceived: datetime
    images: FilePath


