import io
import os
import json

# Imports the Google Cloud client library
from google.cloud import vision
from google.cloud.vision import types


def getObjects(filename):
    # Instantiates a client
    client = vision.ImageAnnotatorClient.from_service_account_json(
        "./Singapore India Hackathon-10434400ab07.json")

    # The name of the image file to annotate
    file_name = os.path.abspath(f'./{filename}')

    # Loads the image into memory
    with io.open(file_name, 'rb') as image_file:
        content = image_file.read()

    image = types.Image(content=content)

    objects = client.object_localization(
        image=image).localized_object_annotations

    print('Number of objects found: {}'.format(len(objects)))

    arr = []

    a = {"objects": arr}

    for object_ in objects:
        a["objects"].append(object_.name)

    returnVal = json.dumps(a)
    return returnVal
