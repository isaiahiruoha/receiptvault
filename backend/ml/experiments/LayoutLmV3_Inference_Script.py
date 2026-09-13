# LayoutLmV3 Inference Script

# Based on Receipt Understanding Notebook by IbrahimHabibeg 
# https://www.kaggle.com/code/ibrahimhabibeg/building-a-receipt-understanding-model
#



# ---------------------- libs ----------------------------------
import torch
from collections import Counter
from transformers import AutoModelForTokenClassification, AutoProcessor
from PIL import Image
import sys
import json
import pytesseract #OCR library for text recognition
from pdf2image import convert_from_path #To convert PDF to image

import os
import warnings

# Keep stdout clean: only the final JSON should be printed there. Silence the
# noisy library warnings/logging that would otherwise land on stderr.
os.environ.setdefault("TRANSFORMERS_VERBOSITY", "error")
os.environ.setdefault("TOKENIZERS_PARALLELISM", "false")
os.environ.setdefault("HF_HUB_DISABLE_PROGRESS_BARS", "1")

# Ensure the Homebrew binaries (tesseract for OCR, poppler for PDF conversion)
# are discoverable. A .app launched from Finder has a minimal PATH that excludes
# /opt/homebrew/bin, so without this the OCR/PDF steps fail only in the bundled app.
os.environ["PATH"] = "/opt/homebrew/bin:/usr/local/bin:" + os.environ.get("PATH", "")
warnings.simplefilter("ignore")
warnings.simplefilter("ignore", category=FutureWarning)

# Resolve the fine-tuned model directory relative to this script so it works on
# any machine (was previously a hardcoded Windows path).
DEFAULT_MODEL_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "model")


#Handles reading and processing recipt images using the tained model
class ReceiptReader:
  #Load pre-trained model 
  def __init__(self, path_to_model=DEFAULT_MODEL_PATH):
    self.model = AutoModelForTokenClassification.from_pretrained(path_to_model)
    self.model.eval()
    self.processor = AutoProcessor.from_pretrained(path_to_model, apply_ocr=True)      

  #Process used to extract prediction from a receipt image input.
  def __call__(self, image):
    encodings = self.__get_encodings(image)
    words = self.__get_words(encodings)
    bboxes = encodings.bbox[0]
    logits = self.model(**encodings).logits
    predictions = torch.argmax(logits, dim=2)
    labeled_tokens = [self.model.config.id2label[t.item()] for t in predictions[0]]
    response_dict = self.__merge_tokens(words, bboxes, labeled_tokens)
    response_dict["bboxes"] = [self.__unnormalize_bbox(bbox, image) for bbox in response_dict["bboxes"]]
    return response_dict
  
  #Tokenize and encode the image for the model
  def __get_encodings(self, image):
    return self.processor(image, return_tensors="pt")
  
  #Decode the tokenized inputs back to words
  def __get_words(self, encodings):
    words = [self.processor.tokenizer.decode(input_id) for input_id in encodings.input_ids[0]]
    return words
  
  #Merge tokens that belong to same entity based on bounding boxes
  def __merge_tokens(self, words, bboxes, labels):
    new_words = []
    new_bboxes = []
    new_labels = []
    i = 0
    while i < len(words):
        token, bbox, label = words[i], bboxes[i], labels[i]
        j = i + 1
        while j < len(words) and self.__is_same_bbox(bbox, bboxes[j]):
            token += words[j]
            j += 1
        counter = Counter([labels[k] for k in range(i, j)])
        sorted_labels = sorted(counter, key=counter.get, reverse=True)
        if sorted_labels[0] == "O" and len(sorted_labels) > 1:
          label = sorted_labels[1]
        else:
          label = sorted_labels[0]
        new_words.append(token)
        new_bboxes.append(bbox)
        new_labels.append(label)
        i = j
    return {
        "words": new_words,
        "bboxes": new_bboxes,
        "labels": new_labels
    }
  
  def __is_same_bbox(self, bbox1, bbox2):
    for i in range(4):
        if abs(bbox1[i] - bbox2[i]) > 3:
            return False
    return True
  
  def __unnormalize_bbox(self, bbox, image):
    width, height = image.size
    return [bbox[0] * width / 1000, bbox[1] * height / 1000, bbox[2] * width / 1000, bbox[3] * height / 1000]
  


#-------------------- ReceiptInformationExtractor --------------------------  

class ReceiptInformationExtractor:
  def __init__(self, path_to_model=DEFAULT_MODEL_PATH):
    self.receipt_reader = ReceiptReader(path_to_model)
  
  def __call__(self, image):
    receipt_data = self.receipt_reader(image)
    response_dict = {
        "company": "",
        "date": "",
        "address": "",
        "total": ""
    }
    
    # Get the company having the largest bbox
    max_bbox = 0
    for word, bbox, label in zip(receipt_data['words'], receipt_data["bboxes"], receipt_data["labels"]):
      if label == "B-COMPANY":
        bbox_size = (bbox[2] - bbox[0]) * (bbox[3] - bbox[1])
        if bbox_size > max_bbox:
          response_dict["company"] = word.strip()
          max_bbox = bbox_size
          
    # Get the address having the largest bbox
    max_bbox = 0
    for word, bbox, label in zip(receipt_data['words'], receipt_data["bboxes"], receipt_data["labels"]):
      if label == "B-ADDRESS":
        bbox_size = (bbox[2] - bbox[0]) * (bbox[3] - bbox[1])
        if bbox_size > max_bbox:
          response_dict["address"] = word.strip()
          max_bbox = bbox_size
  
    # Get the topmost date
    min_y = float("inf")
    for word, bbox, label in zip(receipt_data['words'], receipt_data["bboxes"], receipt_data["labels"]):
      if label == "B-DATE" and bbox[1] < min_y:
        response_dict["date"] = word.strip()
        min_y = bbox[1]

    # Get the bottommost total
    max_y = 0
    for word, bbox, label in zip(receipt_data['words'], receipt_data["bboxes"], receipt_data["labels"]):
      if label == "B-TOTAL" and bbox[3] > max_y:
        response_dict["total"] = word.strip()
        max_y = bbox[3]
        
    return response_dict

  

def extract_info(file_path, ):
    # Convert PDF to image if necessary
    if file_path.lower().endswith('.pdf'):
        images = convert_from_path(file_path)
        # For simplicity, take the first page
        image = images[0]
    else:
        image = Image.open(file_path).convert("RGB")
    #Use class to read    
    receipt_info_extractor = ReceiptInformationExtractor() 
    json_result = receipt_info_extractor(image)

    return json_result

#start----------------------------------------------------------------------

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(json.dumps({"error": "Invalid number of arguments"}))
        sys.exit(1)
    
    file_path = sys.argv[1]
    try:
        info = extract_info(file_path)
        print(json.dumps(info))
    except Exception as e:
        print(json.dumps({"error": str(e)}))
        sys.exit(1)
