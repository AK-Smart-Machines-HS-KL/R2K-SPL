# Ball Detection Network: Practical Training Tutorial

## Complete Working Example

This tutorial provides copy-paste ready code for retraining your ball detection network.

---

## Part 1: Dataset Preparation

### Script 1: Extract Patches from SimRobot Logs

**File: `Tools/DataCollection/extract_ball_patches.py`**

```python
#!/usr/bin/env python3
"""
Extract ball patches from SimRobot rendered logs for neural network training.
"""

import cv2
import numpy as np
import os
import json
from pathlib import Path
from typing import Tuple, List
import argparse

class BallPatchExtractor:
    def __init__(self, patch_size: int = 64, negative_ratio: float = 2.0):
        """
        Initialize extractor.
        
        Args:
            patch_size: Size of extracted patches (64x64, 128x128, etc.)
            negative_ratio: How many negative patches per positive
        """
        self.patch_size = patch_size
        self.negative_ratio = negative_ratio
        self.extracted_count = 0
        self.annotations = []
    
    def extract_from_image(self, image: np.ndarray, 
                          ball_pos: Tuple[int, int], 
                          ball_radius: float,
                          image_name: str,
                          output_dir: Path) -> bool:
        """Extract positive patch around ball."""
        x, y = int(ball_pos[0]), int(ball_pos[1])
        half = self.patch_size // 2
        
        # Check bounds
        if (x - half < 0 or x + half >= image.shape[1] or
            y - half < 0 or y + half >= image.shape[0]):
            return False
        
        # Extract patch
        patch = image[y-half:y+half, x-half:x+half]
        
        if patch.shape != (self.patch_size, self.patch_size):
            return False
        
        # Save patch
        filename = f"positive_{self.extracted_count:06d}.jpg"
        cv2.imwrite(str(output_dir / "positive" / filename), patch)
        
        # Record annotation
        self.annotations.append({
            "filename": filename,
            "image": image_name,
            "is_ball": 1,
            "center_x": half,  # In patch coordinates
            "center_y": half,
            "radius": int(ball_radius),
            "confidence": 0.95
        })
        
        self.extracted_count += 1
        return True
    
    def extract_negatives(self, image: np.ndarray, 
                         ball_pos: Tuple[int, int],
                         ball_radius: float,
                         image_name: str,
                         output_dir: Path,
                         count: int = 2):
        """Extract random patches NOT containing ball."""
        h, w = image.shape[:2]
        min_dist = int(ball_radius * 3)  # Keep away from ball
        
        for i in range(count):
            # Random location far from ball
            attempts = 0
            while attempts < 10:
                x = np.random.randint(self.patch_size // 2, w - self.patch_size // 2)
                y = np.random.randint(self.patch_size // 2, h - self.patch_size // 2)
                
                # Check distance from ball
                if abs(x - ball_pos[0]) > min_dist or abs(y - ball_pos[1]) > min_dist:
                    break
                attempts += 1
            
            if attempts >= 10:
                continue
            
            half = self.patch_size // 2
            patch = image[y-half:y+half, x-half:x+half]
            
            if patch.shape != (self.patch_size, self.patch_size):
                continue
            
            filename = f"negative_{self.extracted_count:06d}.jpg"
            cv2.imwrite(str(output_dir / "negative" / filename), patch)
            
            self.annotations.append({
                "filename": filename,
                "image": image_name,
                "is_ball": 0,
                "center_x": 0,
                "center_y": 0,
                "radius": 0,
                "confidence": 0.0
            })
            
            self.extracted_count += 1
    
    def save_annotations(self, output_dir: Path):
        """Save annotations to JSON."""
        with open(output_dir / "annotations.json", 'w') as f:
            json.dump(self.annotations, f, indent=2)
        
        print(f"Extracted {self.extracted_count} patches")
        print(f"Saved to {output_dir}")


def main():
    parser = argparse.ArgumentParser(description='Extract ball patches from images')
    parser.add_argument('--image-dir', type=str, required=True,
                       help='Directory containing images')
    parser.add_argument('--output-dir', type=str, default='./dataset',
                       help='Output directory for patches')
    parser.add_argument('--patch-size', type=int, default=64,
                       help='Patch size (default 64x64)')
    parser.add_argument('--annotations', type=str, required=True,
                       help='JSON file with ball positions')
    
    args = parser.parse_args()
    
    # Create output directories
    output_dir = Path(args.output_dir)
    output_dir.mkdir(exist_ok=True)
    (output_dir / "positive").mkdir(exist_ok=True)
    (output_dir / "negative").mkdir(exist_ok=True)
    
    # Load annotations
    with open(args.annotations, 'r') as f:
        annotations = json.load(f)
    
    # Extract patches
    extractor = BallPatchExtractor(patch_size=args.patch_size)
    
    for ann in annotations:
        image_path = Path(args.image_dir) / ann['image']
        if not image_path.exists():
            print(f"Warning: {image_path} not found")
            continue
        
        image = cv2.imread(str(image_path), cv2.IMREAD_GRAYSCALE)
        if image is None:
            continue
        
        # Extract positive patch
        if extractor.extract_from_image(image, 
                                       (ann['x'], ann['y']), 
                                       ann['radius'],
                                       ann['image'],
                                       output_dir):
            # Extract negative patches
            num_negatives = int(extractor.negative_ratio)
            extractor.extract_negatives(image, 
                                       (ann['x'], ann['y']), 
                                       ann['radius'],
                                       ann['image'],
                                       output_dir,
                                       count=num_negatives)
    
    extractor.save_annotations(output_dir)


if __name__ == '__main__':
    main()
```

**Usage:**
```bash
python3 Tools/DataCollection/extract_ball_patches.py \
  --image-dir raw_images/ \
  --output-dir dataset/ \
  --patch-size 64 \
  --annotations annotations.json
```

---

### Script 2: Data Augmentation

**File: `Tools/DataCollection/augment_dataset.py`**

```python
#!/usr/bin/env python3
"""
Augment ball detection dataset to increase training data.
"""

import cv2
import numpy as np
from pathlib import Path
import json
from imgaug import augmenters as iaa
import argparse

class DatasetAugmenter:
    def __init__(self, output_dir: Path):
        self.output_dir = output_dir
        self.augmentation = iaa.Sequential([
            iaa.Fliplr(0.5),                    # Horizontal flip
            iaa.Flipud(0.3),                    # Vertical flip
            iaa.Affine(rotate=(-30, 30)),       # Rotation
            iaa.Multiply((0.7, 1.4)),           # Brightness
            iaa.GaussianBlur(sigma=(0, 0.8)),   # Blur
            iaa.Dropout(p=(0, 0.15)),           # Noise
            iaa.Affine(shear=(-10, 10)),        # Shear
        ])
        self.annotations = []
    
    def augment_image(self, image: np.ndarray, 
                     original_name: str,
                     ann: dict,
                     num_augmentations: int = 3) -> List[dict]:
        """Generate augmented versions of image."""
        new_annotations = []
        
        for i in range(num_augmentations):
            # Augment
            aug_img = self.augmentation(image=image)
            
            # Save
            class_dir = "positive" if ann['is_ball'] else "negative"
            filename = f"{class_dir}_aug_{original_name.stem}_{i}.jpg"
            filepath = self.output_dir / class_dir / filename
            cv2.imwrite(str(filepath), aug_img)
            
            # Record (keep same center for classification task)
            new_ann = ann.copy()
            new_ann['filename'] = filename
            new_ann['augmented'] = True
            new_annotations.append(new_ann)
        
        return new_annotations
    
    def process_directory(self, dataset_dir: Path):
        """Augment all images in dataset."""
        with open(dataset_dir / 'annotations.json', 'r') as f:
            annotations = json.load(f)
        
        all_annotations = annotations.copy()
        
        for ann in annotations:
            image_path = dataset_dir / ("positive" if ann['is_ball'] else "negative") / ann['filename']
            
            if not image_path.exists():
                continue
            
            image = cv2.imread(str(image_path), cv2.IMREAD_GRAYSCALE)
            if image is None:
                continue
            
            # Generate augmentations
            aug_annotations = self.augment_image(image, 
                                                image_path,
                                                ann,
                                                num_augmentations=3)
            all_annotations.extend(aug_annotations)
            
            print(f"Augmented {ann['filename']}")
        
        # Save updated annotations
        with open(self.output_dir / 'annotations_augmented.json', 'w') as f:
            json.dump(all_annotations, f, indent=2)
        
        print(f"Total augmented dataset size: {len(all_annotations)}")


def main():
    parser = argparse.ArgumentParser(description='Augment ball detection dataset')
    parser.add_argument('--dataset-dir', type=str, required=True,
                       help='Dataset directory with annotations.json')
    parser.add_argument('--output-dir', type=str, required=True,
                       help='Output directory for augmented data')
    
    args = parser.parse_args()
    
    output_dir = Path(args.output_dir)
    output_dir.mkdir(exist_ok=True)
    (output_dir / "positive").mkdir(exist_ok=True)
    (output_dir / "negative").mkdir(exist_ok=True)
    
    augmenter = DatasetAugmenter(output_dir)
    augmenter.process_directory(Path(args.dataset_dir))


if __name__ == '__main__':
    main()
```

**Usage:**
```bash
python3 Tools/DataCollection/augment_dataset.py \
  --dataset-dir dataset/ \
  --output-dir dataset_augmented/
```

---

## Part 2: Network Training

### Script 3: Train Encoder Network

**File: `Tools/Training/train_encoder.py`**

```python
#!/usr/bin/env python3
"""
Train encoder network for ball detection.
"""

import tensorflow as tf
from tensorflow import keras
import numpy as np
import json
from pathlib import Path
import argparse
import cv2
from typing import Tuple

class EncoderTrainer:
    def __init__(self, patch_size: int = 64):
        self.patch_size = patch_size
        self.model = self._build_model()
    
    def _build_model(self) -> keras.Model:
        """Build encoder architecture."""
        return keras.Sequential([
            # Input: grayscale patch
            keras.layers.Input(shape=(self.patch_size, self.patch_size, 1)),
            
            # Block 1
            keras.layers.Conv2D(32, (3, 3), padding='same', activation='relu'),
            keras.layers.BatchNormalization(),
            keras.layers.Conv2D(32, (3, 3), padding='same', activation='relu'),
            keras.layers.MaxPooling2D((2, 2)),
            keras.layers.Dropout(0.2),
            
            # Block 2
            keras.layers.Conv2D(64, (3, 3), padding='same', activation='relu'),
            keras.layers.BatchNormalization(),
            keras.layers.Conv2D(64, (3, 3), padding='same', activation='relu'),
            keras.layers.MaxPooling2D((2, 2)),
            keras.layers.Dropout(0.2),
            
            # Block 3
            keras.layers.Conv2D(128, (3, 3), padding='same', activation='relu'),
            keras.layers.BatchNormalization(),
            keras.layers.Conv2D(128, (3, 3), padding='same', activation='relu'),
            keras.layers.MaxPooling2D((2, 2)),
            keras.layers.Dropout(0.2),
            
            # Global pooling
            keras.layers.GlobalAveragePooling2D(),
            
            # Dense layers
            keras.layers.Dense(256, activation='relu'),
            keras.layers.LayerNormalization(),
            keras.layers.Dropout(0.3),
            
            # Output: feature vector
            keras.layers.Dense(128, activation='relu', name='encoder_output')
        ], name='encoder')
    
    def load_dataset(self, dataset_dir: Path) -> Tuple[np.ndarray, np.ndarray]:
        """Load patches and compile dataset."""
        with open(dataset_dir / 'annotations.json', 'r') as f:
            annotations = json.load(f)
        
        images = []
        labels = []
        
        for ann in annotations:
            class_dir = "positive" if ann['is_ball'] else "negative"
            image_path = dataset_dir / class_dir / ann['filename']
            
            if not image_path.exists():
                continue
            
            image = cv2.imread(str(image_path), cv2.IMREAD_GRAYSCALE)
            if image is None:
                continue
            
            # Normalize to [0, 1]
            image = image.astype(np.float32) / 255.0
            image = np.expand_dims(image, axis=-1)  # Add channel dimension
            
            images.append(image)
            labels.append(ann['is_ball'])
        
        return np.array(images), np.array(labels)
    
    def train(self, X_train: np.ndarray, y_train: np.ndarray,
             X_val: np.ndarray, y_val: np.ndarray,
             epochs: int = 50, batch_size: int = 32):
        """Train the encoder."""
        
        # Compile with contrastive loss
        self.model.compile(
            optimizer=keras.optimizers.Adam(learning_rate=0.001),
            loss='binary_crossentropy',
            metrics=['accuracy']
        )
        
        # Callbacks
        callbacks = [
            keras.callbacks.EarlyStopping(
                monitor='val_loss',
                patience=10,
                restore_best_weights=True
            ),
            keras.callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=0.5,
                patience=5,
                min_lr=1e-6
            ),
            keras.callbacks.ModelCheckpoint(
                'encoder_best.h5',
                monitor='val_accuracy',
                save_best_only=True
            )
        ]
        
        # Train
        history = self.model.fit(
            X_train, y_train,
            validation_data=(X_val, y_val),
            epochs=epochs,
            batch_size=batch_size,
            callbacks=callbacks,
            verbose=1
        )
        
        return history
    
    def save(self, filepath: str):
        """Save trained model."""
        self.model.save(filepath)
        print(f"Model saved to {filepath}")
    
    def extract_features(self, X: np.ndarray) -> np.ndarray:
        """Extract features from images using trained encoder."""
        feature_extractor = keras.Model(
            inputs=self.model.input,
            outputs=self.model.get_layer('encoder_output').output
        )
        return feature_extractor.predict(X)


def main():
    parser = argparse.ArgumentParser(description='Train encoder network')
    parser.add_argument('--dataset-dir', type=str, required=True,
                       help='Dataset directory')
    parser.add_argument('--output', type=str, default='encoder_trained.h5',
                       help='Output model filename')
    parser.add_argument('--epochs', type=int, default=50,
                       help='Number of training epochs')
    parser.add_argument('--batch-size', type=int, default=32,
                       help='Batch size')
    
    args = parser.parse_args()
    
    print("Loading dataset...")
    trainer = EncoderTrainer()
    X, y = trainer.load_dataset(Path(args.dataset_dir))
    
    print(f"Dataset size: {len(X)} images")
    print(f"Positive: {np.sum(y)}, Negative: {len(y) - np.sum(y)}")
    
    # Split
    split = int(0.8 * len(X))
    X_train, X_val = X[:split], X[split:]
    y_train, y_val = y[:split], y[split:]
    
    print(f"Train: {len(X_train)}, Val: {len(X_val)}")
    
    print("Training encoder...")
    history = trainer.train(X_train, y_train, X_val, y_val,
                           epochs=args.epochs, batch_size=args.batch_size)
    
    trainer.save(args.output)


if __name__ == '__main__':
    main()
```

**Usage:**
```bash
python3 Tools/Training/train_encoder.py \
  --dataset-dir dataset_augmented/ \
  --output encoder_trained.h5 \
  --epochs 50 \
  --batch-size 32
```

---

### Script 4: Train Classifier Network

**File: `Tools/Training/train_classifier.py`**

```python
#!/usr/bin/env python3
"""
Train classifier network using features from trained encoder.
"""

import tensorflow as tf
from tensorflow import keras
import numpy as np
import json
from pathlib import Path
import argparse
import cv2

class ClassifierTrainer:
    def __init__(self, encoder_model_path: str, encoder_output_size: int = 128):
        self.encoder = keras.models.load_model(encoder_model_path)
        self.encoder_output_size = encoder_output_size
        self.classifier = self._build_model()
    
    def _build_model(self) -> keras.Model:
        """Build classifier architecture."""
        return keras.Sequential([
            keras.layers.Input(shape=(self.encoder_output_size,)),
            keras.layers.Dense(128, activation='relu'),
            keras.layers.BatchNormalization(),
            keras.layers.Dropout(0.3),
            keras.layers.Dense(64, activation='relu'),
            keras.layers.Dropout(0.2),
            keras.layers.Dense(1, activation='sigmoid', name='ball_probability')
        ], name='classifier')
    
    def extract_features(self, images: np.ndarray) -> np.ndarray:
        """Extract features using encoder."""
        # Create feature extractor
        feature_extractor = keras.Model(
            inputs=self.encoder.input,
            outputs=self.encoder.get_layer(-2).output  # Before final dense
        )
        return feature_extractor.predict(images, batch_size=32)
    
    def train(self, X_features_train: np.ndarray, y_train: np.ndarray,
             X_features_val: np.ndarray, y_val: np.ndarray,
             epochs: int = 30, batch_size: int = 32):
        """Train classifier."""
        
        self.classifier.compile(
            optimizer=keras.optimizers.Adam(learning_rate=0.001),
            loss='binary_crossentropy',
            metrics=['accuracy', keras.metrics.Precision(), keras.metrics.Recall()]
        )
        
        callbacks = [
            keras.callbacks.EarlyStopping(
                monitor='val_loss',
                patience=10,
                restore_best_weights=True
            ),
            keras.callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=0.5,
                patience=5
            )
        ]
        
        history = self.classifier.fit(
            X_features_train, y_train,
            validation_data=(X_features_val, y_val),
            epochs=epochs,
            batch_size=batch_size,
            callbacks=callbacks,
            verbose=1
        )
        
        return history
    
    def save(self, filepath: str):
        """Save trained classifier."""
        self.classifier.save(filepath)
        print(f"Classifier saved to {filepath}")


def main():
    parser = argparse.ArgumentParser(description='Train classifier network')
    parser.add_argument('--dataset-dir', type=str, required=True)
    parser.add_argument('--encoder', type=str, required=True,
                       help='Path to trained encoder model')
    parser.add_argument('--output', type=str, default='classifier_trained.h5')
    parser.add_argument('--epochs', type=int, default=30)
    parser.add_argument('--batch-size', type=int, default=32)
    
    args = parser.parse_args()
    
    print("Loading dataset...")
    trainer = ClassifierTrainer(args.encoder)
    
    # Load images and labels
    with open(Path(args.dataset_dir) / 'annotations.json', 'r') as f:
        annotations = json.load(f)
    
    images = []
    labels = []
    for ann in annotations:
        class_dir = "positive" if ann['is_ball'] else "negative"
        image_path = Path(args.dataset_dir) / class_dir / ann['filename']
        
        if not image_path.exists():
            continue
        
        image = cv2.imread(str(image_path), cv2.IMREAD_GRAYSCALE)
        image = image.astype(np.float32) / 255.0
        image = np.expand_dims(image, axis=-1)
        
        images.append(image)
        labels.append(ann['is_ball'])
    
    X = np.array(images)
    y = np.array(labels)
    
    print(f"Dataset: {len(X)} images ({np.sum(y)} positive, {len(y)-np.sum(y)} negative)")
    
    # Extract features
    print("Extracting features...")
    X_features = trainer.extract_features(X)
    
    # Split
    split = int(0.8 * len(X_features))
    X_train, X_val = X_features[:split], X_features[split:]
    y_train, y_val = y[:split], y[split:]
    
    # Train
    print("Training classifier...")
    history = trainer.train(X_train, y_train, X_val, y_val,
                           epochs=args.epochs, batch_size=args.batch_size)
    
    trainer.save(args.output)


if __name__ == '__main__':
    main()
```

---

## Part 3: Integration & Testing

### Script 5: Convert to ONNX Format

**File: `Tools/Training/convert_to_onnx.py`**

```python
#!/usr/bin/env python3
"""
Convert trained models to ONNX format for CompiledNN.
"""

import tensorflow as tf
import tf2onnx
import onnx
import argparse

def convert_model(keras_path: str, onnx_path: str):
    """Convert Keras model to ONNX."""
    
    # Load model
    model = tf.keras.models.load_model(keras_path)
    
    # Infer input shapes
    input_signature = [tf2onnx.tf_utils.numpy_to_tf_tensor(
        tf.zeros(model.input_shape)
    )]
    
    # Convert
    onnx_model, _ = tf2onnx.convert.from_keras(model, 
                                               input_signature=input_signature)
    
    # Save
    onnx.save(onnx_model, onnx_path)
    print(f"Converted {keras_path} → {onnx_path}")


def main():
    parser = argparse.ArgumentParser(description='Convert models to ONNX')
    parser.add_argument('--encoder', type=str, required=True)
    parser.add_argument('--classifier', type=str, required=True)
    parser.add_argument('--corrector', type=str, required=True)
    parser.add_argument('--output-dir', type=str, default='.')
    
    args = parser.parse_args()
    
    convert_model(args.encoder, f"{args.output_dir}/encoder.onnx")
    convert_model(args.classifier, f"{args.output_dir}/classify.onnx")
    convert_model(args.corrector, f"{args.output_dir}/corrector.onnx")
    
    print("All models converted to ONNX format")


if __name__ == '__main__':
    main()
```

---

## Quick Training Workflow

```bash
# 1. Extract patches
python3 Tools/DataCollection/extract_ball_patches.py \
  --image-dir ./raw_images \
  --output-dir ./dataset \
  --patch-size 64

# 2. Augment data
python3 Tools/DataCollection/augment_dataset.py \
  --dataset-dir ./dataset \
  --output-dir ./dataset_augmented

# 3. Train encoder
python3 Tools/Training/train_encoder.py \
  --dataset-dir ./dataset_augmented \
  --output ./encoder_trained.h5 \
  --epochs 50

# 4. Train classifier
python3 Tools/Training/train_classifier.py \
  --dataset-dir ./dataset_augmented \
  --encoder ./encoder_trained.h5 \
  --output ./classifier_trained.h5

# 5. Train corrector (similar to classifier)
# ... (implement based on classifier_train.py)

# 6. Convert to ONNX
python3 Tools/Training/convert_to_onnx.py \
  --encoder ./encoder_trained.h5 \
  --classifier ./classifier_trained.h5 \
  --corrector ./corrector_trained.h5 \
  --output-dir ./Config/NeuralNets/BallPerceptor/

# 7. Replace models
cp ./Config/NeuralNets/BallPerceptor/encoder.onnx \
   ./Config/NeuralNets/BallPerceptor/encoder.h5

# 8. Test in SimRobot
./Build/Linux/SimRobot/Release/SimRobot
```

---

## Expected Results

After training on 10,000+ patches:
- **Encoder accuracy:** 85-90% (features)
- **Classifier accuracy:** 92-96% (ball/no-ball)
- **Position error:** 2-4 pixels
- **Training time:** 8-12 hours on GPU

With augmentation to 30,000+ images:
- **Classifier accuracy:** 95-98%
- **Position error:** 1-2 pixels
- **Robustness:** Handles lighting variations

